/* MediQueue web server: C backend + Min-Heap, BST, Linked List. */
#include "patient.h"
#include "heap.h"
#include "bst.h"
#include "history.h"
#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef SOCKET socket_t;
#define CLOSESOCK closesocket
#define SOCKET_INVALID INVALID_SOCKET
#define SOCKET_ERROR_CODE SOCKET_ERROR
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
typedef int socket_t;
#define CLOSESOCK close
#define SOCKET_INVALID (-1)
#define SOCKET_ERROR_CODE (-1)
#endif

#define PORT 8091
#define REQ_CAP 32768
#define DATA_FILE "data/patients.csv"
static MinHeap heap;
static BSTNode *index_root = NULL;
static HistoryList history;
static Patient *all_patients = NULL;
static unsigned long sequence_no = 0;

static void send_all(socket_t s, const char *data) { size_t n = strlen(data), sent = 0; while (sent < n) { int k = send(s, data + sent, (int)(n - sent), 0); if (k <= 0) break; sent += (size_t)k; } }
static void response(socket_t s, const char *status, const char *type, const char *body) {
    char head[512]; snprintf(head, sizeof head, "HTTP/1.1 %s\r\nContent-Type: %s; charset=utf-8\r\nAccess-Control-Allow-Origin: *\r\nCache-Control: no-store\r\nConnection: close\r\nContent-Length: %zu\r\n\r\n", status, type, strlen(body));
    send_all(s, head); send_all(s, body);
}
static void json_string(FILE *out, const char *s) {
    fputc('"', out); for (; *s; ++s) { unsigned char c = (unsigned char)*s; if (c == '"' || c == '\\') { fputc('\\', out); fputc(c, out); } else if (c == '\n') fputs("\\n", out); else if (c == '\r') fputs("\\r", out); else if (c == '\t') fputs("\\t", out); else if (c >= 32) fputc(c, out); } fputc('"', out);
}
static void patient_json(FILE *out, const Patient *p) {
    fputs("{\"id\":", out); json_string(out,p->id); fputs(",\"name\":",out); json_string(out,p->name);
    fprintf(out, ",\"age\":%d,\"priority\":%d,\"arrivalOrder\":%lu,\"registeredAt\":",p->age,p->priority,p->arrival_order);
    json_string(out,p->registered_at); fprintf(out, ",\"status\":\"%s\"}",p->status==STATUS_WAITING?"Menunggu":"Dipanggil");
}
static int before_patient(const Patient *a,const Patient *b) { if(a->priority!=b->priority)return a->priority<b->priority; return a->arrival_order<b->arrival_order; }
static void sort_patients(Patient **a,int n) { for(int i=1;i<n;i++){Patient *v=a[i];int j=i-1;while(j>=0&&before_patient(v,a[j])){a[j+1]=a[j];j--;}a[j+1]=v;} }
static int collect_waiting(Patient **a) { int n=0; for(Patient *p=all_patients;p;p=p->next_all) if(p->status==STATUS_WAITING && n<HEAP_CAPACITY)a[n++]=p; sort_patients(a,n); return n; }
static void api_patients(socket_t s) {
    Patient *arr[HEAP_CAPACITY]; int n=collect_waiting(arr); char *body=(char*)calloc(1,REQ_CAP); if(!body){response(s,"500 Internal Server Error","application/json","{\"error\":\"Memori tidak cukup\"}");return;}
    FILE *f=tmpfile(); if(!f){free(body);return;} fputc('[',f); for(int i=0;i<n;i++){if(i)fputc(',',f);patient_json(f,arr[i]);} fputc(']',f); rewind(f); size_t r=fread(body,1,REQ_CAP-1,f); body[r]='\0'; fclose(f); response(s,"200 OK","application/json",body);free(body);
}
static void api_history(socket_t s) {
    char body[REQ_CAP]; size_t pos=0; pos+=(size_t)snprintf(body+pos,sizeof(body)-pos,"["); int first=1;
    for(HistoryNode *node=history.head;node && pos<sizeof(body)-512;node=node->next){if(!first)pos+=(size_t)snprintf(body+pos,sizeof(body)-pos,",");first=0; FILE *f=tmpfile();if(!f)break;patient_json(f,node->patient);rewind(f);pos+=fread(body+pos,1,sizeof(body)-pos-1,f);fclose(f);}
    snprintf(body+pos,sizeof(body)-pos,"]");response(s,"200 OK","application/json",body);
}
static void api_stats(socket_t s) { int total=0,waiting=0,served=0;for(Patient*p=all_patients;p;p=p->next_all){total++;if(p->status==STATUS_WAITING)waiting++;else served++;}char b[512];Patient *top=heap_peek(&heap);snprintf(b,sizeof b,"{\"total\":%d,\"waiting\":%d,\"served\":%d,\"heapSize\":%d,\"topPriority\":%d,\"topId\":\"%s\"}",total,waiting,served,heap.size,top?top->priority:0,top?top->id:"");response(s,"200 OK","application/json",b); }
static void api_heap(socket_t s) { char b[8192];size_t pos=0;pos+=(size_t)snprintf(b+pos,sizeof b-pos,"[");for(int i=0;i<heap.size;i++){Patient*p=heap.items[i];int level=0;for(int x=i+1;x>1;x/=2)level++;if(i)pos+=(size_t)snprintf(b+pos,sizeof b-pos,",");pos+=(size_t)snprintf(b+pos,sizeof b-pos,"{\"index\":%d,\"id\":\"%s\",\"priority\":%d,\"level\":%d}",i,p->id,p->priority,level);}snprintf(b+pos,sizeof b-pos,"]");response(s,"200 OK","application/json",b); }
static void get_json_value(const char *body,const char *key,char *out,size_t cap) { char pattern[64];snprintf(pattern,sizeof pattern,"\"%s\"",key);const char*p=strstr(body,pattern);if(!p){out[0]=0;return;}p=strchr(p,':');if(!p){out[0]=0;return;}p++;while(*p==' '||*p=='\t')p++;if(*p!='"'){out[0]=0;return;}p++;size_t i=0;while(*p&&*p!='"'&&i+1<cap){if(*p=='\\'&&p[1])p++;out[i++]=*p++;}out[i]=0; }
static int get_json_int(const char *body,const char *key,int fallback) { char pattern[64];snprintf(pattern,sizeof pattern,"\"%s\"",key);const char*p=strstr(body,pattern);if(!p)return fallback;p=strchr(p,':');if(!p)return fallback;return atoi(p+1); }
static void api_register(socket_t s,const char *body) {
    char id[ID_LEN],name[NAME_LEN];get_json_value(body,"id",id,sizeof id);get_json_value(body,"name",name,sizeof name);int age=get_json_int(body,"age",-1),priority=get_json_int(body,"priority",0);
    if(!id[0]||!name[0]||strchr(id,',')||strchr(name,',')||age<0||age>120||priority<1||priority>5){response(s,"400 Bad Request","application/json","{\"error\":\"Periksa ID, nama, usia (0-120), dan prioritas simulasi (1-5).\"}");return;}
    if(strlen(id)>=ID_LEN||strlen(name)>=NAME_LEN){response(s,"400 Bad Request","application/json","{\"error\":\"ID atau nama terlalu panjang.\"}");return;}
    if(bst_search(index_root,id)){response(s,"409 Conflict","application/json","{\"error\":\"ID pasien sudah terdaftar.\"}");return;}if(heap.size>=HEAP_CAPACITY){response(s,"409 Conflict","application/json","{\"error\":\"Kapasitas antrean penuh.\"}");return;}
    Patient*p=(Patient*)calloc(1,sizeof(Patient));if(!p){response(s,"500 Internal Server Error","application/json","{\"error\":\"Memori tidak cukup.\"}");return;}strcpy(p->id,id);strcpy(p->name,name);p->age=age;p->priority=priority;p->arrival_order=++sequence_no;p->status=STATUS_WAITING;patient_set_timestamp(p->registered_at,sizeof p->registered_at);
    if(!bst_insert(&index_root,p)||!heap_push(&heap,p)){free(p);response(s,"500 Internal Server Error","application/json","{\"error\":\"Gagal memasukkan data.\"}");return;}p->next_all=all_patients;all_patients=p;storage_save(DATA_FILE,all_patients);response(s,"201 Created","application/json","{\"message\":\"Pasien berhasil ditambahkan ke antrean simulasi.\"}");
}
static void api_call_next(socket_t s) { Patient*p=heap_pop(&heap);if(!p){response(s,"409 Conflict","application/json","{\"error\":\"Antrean sedang kosong.\"}");return;}p->status=STATUS_SERVED;history_append(&history,p);storage_save(DATA_FILE,all_patients);char b[1024];FILE*f=tmpfile();if(!f)return;fputs("{\"message\":\"Pasien berhasil dipanggil dalam simulasi.\",\"patient\":",f);patient_json(f,p);fputs("}",f);rewind(f);size_t n=fread(b,1,sizeof b-1,f);b[n]=0;fclose(f);response(s,"200 OK","application/json",b); }
static void api_search(socket_t s,const char *path) { const char*p=strstr(path,"id=");char id[ID_LEN]="";if(p){p+=3;size_t i=0;while(*p&&*p!='&'&&i+1<sizeof id){id[i++]=*p++;}id[i]=0;}Patient*found=bst_search(index_root,id);if(!found){response(s,"404 Not Found","application/json","{\"error\":\"Pasien tidak ditemukan.\"}");return;}char b[2048];FILE*f=tmpfile();if(!f)return;patient_json(f,found);rewind(f);size_t n=fread(b,1,sizeof b-1,f);b[n]=0;fclose(f);response(s,"200 OK","application/json",b);}
static void static_file(socket_t s,const char *path) { const char *file="web/index.html",*type="text/html";if(strcmp(path,"/")==0||strcmp(path,"/index.html")==0){}else if(strcmp(path,"/style.css")==0){file="web/style.css";type="text/css";}else if(strcmp(path,"/app.js")==0){file="web/app.js";type="application/javascript";}else{response(s,"404 Not Found","text/plain","Halaman tidak ditemukan.");return;}FILE*f=fopen(file,"rb");if(!f){response(s,"500 Internal Server Error","text/plain","File web tidak ditemukan. Jalankan server dari folder utama MediQueue.");return;}fseek(f,0,SEEK_END);long len=ftell(f);rewind(f);char*buf=(char*)malloc((size_t)len+1);if(!buf){fclose(f);return;}size_t n=fread(buf,1,(size_t)len,f);buf[n]=0;fclose(f);response(s,"200 OK",type,buf);free(buf);}
static void handle(socket_t s) {
    char req[REQ_CAP];int got=0,n;while(got<REQ_CAP-1&&(n=recv(s,req+got,REQ_CAP-1-got,0))>0){got+=n;req[got]=0;char*end=strstr(req,"\r\n\r\n");if(end){int content=0;char*cl=strstr(req,"Content-Length:");if(cl)content=atoi(cl+15);int have=(int)(req+got-(end+4));if(have>=content)break;}}
    if(got<=0){CLOSESOCK(s);return;}req[got]=0;char method[8]="",path[256]="";sscanf(req,"%7s %255s",method,path);char*body=strstr(req,"\r\n\r\n");body=body?body+4:"";
    if(strcmp(method,"OPTIONS")==0){response(s,"204 No Content","text/plain","");}
    else if(strcmp(method,"GET")==0&&strcmp(path,"/api/patients")==0)api_patients(s);
    else if(strcmp(method,"GET")==0&&strcmp(path,"/api/history")==0)api_history(s);
    else if(strcmp(method,"GET")==0&&strcmp(path,"/api/stats")==0)api_stats(s);
    else if(strcmp(method,"GET")==0&&strcmp(path,"/api/heap")==0)api_heap(s);
    else if(strcmp(method,"GET")==0&&strncmp(path,"/api/search?",11)==0)api_search(s,path);
    else if(strcmp(method,"POST")==0&&strcmp(path,"/api/patients")==0)api_register(s,body);
    else if(strcmp(method,"POST")==0&&strcmp(path,"/api/call-next")==0)api_call_next(s);
    else if(strcmp(method,"GET")==0)static_file(s,path);
    else response(s,"404 Not Found","application/json","{\"error\":\"Endpoint tidak ditemukan.\"}");
    CLOSESOCK(s);
}
int main(void) {
#ifdef _WIN32
    WSADATA wsa;if(WSAStartup(MAKEWORD(2,2),&wsa)!=0){puts("Gagal memulai Winsock.");return 1;}
#endif
    heap_init(&heap);history_init(&history);if(!storage_load(DATA_FILE,&all_patients,&heap,&index_root,&history,&sequence_no))puts("File data belum ada; memulai dengan data kosong.");
    socket_t server=socket(AF_INET,SOCK_STREAM,0);if(server==SOCKET_INVALID){puts("Gagal membuat socket.");return 1;}int yes=1;setsockopt(server,SOL_SOCKET,SO_REUSEADDR,(const char*)&yes,sizeof yes);struct sockaddr_in addr;memset(&addr,0,sizeof addr);addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_ANY);addr.sin_port=htons(PORT);
    if(bind(server,(struct sockaddr*)&addr,sizeof addr)==SOCKET_ERROR_CODE||listen(server,16)==SOCKET_ERROR_CODE){puts("Gagal membuka port 8080. Pastikan port tidak sedang digunakan.");CLOSESOCK(server);return 1;}
    printf("\nMediQueue berjalan di http://localhost:%d\nTekan Ctrl+C untuk menghentikan server.\n",PORT);
    for(;;){socket_t client=accept(server,NULL,NULL);if(client!=SOCKET_INVALID)handle(client);}
    return 0;
}
