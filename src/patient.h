#ifndef PATIENT_H
#define PATIENT_H

#define ID_LEN 16
#define NAME_LEN 60
#define TIME_LEN 32

typedef enum { STATUS_WAITING = 0, STATUS_SERVED = 1 } PatientStatus;

typedef struct Patient {
    char id[ID_LEN];
    char name[NAME_LEN];
    int age;
    int priority;                 /* Nilai simulasi 1..5; angka kecil didahulukan. */
    unsigned long arrival_order;  /* Pemecah seri: yang datang lebih dahulu. */
    char registered_at[TIME_LEN];
    PatientStatus status;
    struct Patient *next_all;
} Patient;

void patient_print_header(void);
void patient_print(const Patient *p);
void patient_set_timestamp(char *buffer, unsigned long size);

#endif
