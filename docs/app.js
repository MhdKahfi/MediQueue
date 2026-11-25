// ===== MediQueue versi statis (GitHub Pages) =====
// Backend C diganti penyimpanan di browser (localStorage) dengan struktur data yang sama:
// Min-Heap (antrean prioritas), BST (pencarian ID), dan list riwayat pemanggilan.
const store=(()=>{
  const KEY='mediqueue_v2';
  const now=()=>{const d=new Date(),p=n=>String(n).padStart(2,'0');return `${d.getFullYear()}-${p(d.getMonth()+1)}-${p(d.getDate())} ${p(d.getHours())}:${p(d.getMinutes())}:${p(d.getSeconds())}`};
  let state={seq:0,patients:[]};
  try{const raw=localStorage.getItem(KEY);if(raw)state=JSON.parse(raw)}catch(e){}
  if(!state.patients.length&&!state.seq){
    [['P001','Budi Santoso',45,3],['P002','Siti Aminah',67,1],['P003','Andi Pratama',22,4],['P004','Rina Lestari',34,2]]
      .forEach(([id,name,age,priority])=>state.patients.push({id,name,age,priority,arrivalOrder:++state.seq,registeredAt:now(),status:'Menunggu'}));
  }
  const save=()=>{try{localStorage.setItem(KEY,JSON.stringify(state))}catch(e){}};
  const before=(a,b)=>a.priority!==b.priority?a.priority<b.priority:a.arrivalOrder<b.arrivalOrder;
  // Min-Heap
  const buildHeap=()=>{const h=[];const push=p=>{h.push(p);let i=h.length-1;while(i>0){const par=(i-1)>>1;if(before(h[i],h[par])){[h[i],h[par]]=[h[par],h[i]];i=par}else break}};
    state.patients.filter(p=>p.status==='Menunggu').sort((a,b)=>a.arrivalOrder-b.arrivalOrder).forEach(push);return h};
  const pop=h=>{const top=h[0],last=h.pop();if(h.length){h[0]=last;let i=0;for(;;){let l=2*i+1,r=l+1,m=i;if(l<h.length&&before(h[l],h[m]))m=l;if(r<h.length&&before(h[r],h[m]))m=r;if(m===i)break;[h[i],h[m]]=[h[m],h[i]];i=m}}return top};
  // BST berdasarkan ID
  const buildBST=()=>{let root=null;const ins=p=>{const n={p,l:null,r:null};if(!root){root=n;return}let c=root;for(;;){if(p.id<c.p.id){if(!c.l){c.l=n;return}c=c.l}else{if(!c.r){c.r=n;return}c=c.r}}};state.patients.forEach(ins);return root};
  const bstFind=(root,id)=>{let c=root;while(c){if(id===c.p.id)return c.p;c=id<c.p.id?c.l:c.r}return null};
  const fail=(m)=>{throw new Error(m)};
  return{
    stats(){const w=state.patients.filter(p=>p.status==='Menunggu'),h=buildHeap(),t=h[0];return{total:state.patients.length,waiting:w.length,served:state.patients.length-w.length,heapSize:h.length,topPriority:t?t.priority:0,topId:t?t.id:''}},
    patients(){const h=buildHeap(),out=[];while(h.length)out.push(pop(h));return out},
    history(){return state.patients.filter(p=>p.status==='Dipanggil').sort((a,b)=>a.calledOrder-b.calledOrder)},
    heap(){return buildHeap().map((p,i)=>({index:i,id:p.id,priority:p.priority,level:Math.floor(Math.log2(i+1))}))},
    search(id){const p=bstFind(buildBST(),id);if(!p)fail('Pasien tidak ditemukan.');return p},
    register({id,name,age,priority}){
      if(!id||!name||id.includes(',')||name.includes(',')||!(age>=0&&age<=120)||!(priority>=1&&priority<=5))fail('Periksa ID, nama, usia (0-120), dan prioritas simulasi (1-5).');
      if(id.length>=16||name.length>=60)fail('ID atau nama terlalu panjang.');
      if(bstFind(buildBST(),id))fail('ID pasien sudah terdaftar.');
      state.patients.push({id,name,age,priority,arrivalOrder:++state.seq,registeredAt:now(),status:'Menunggu'});save();return{message:'Pasien berhasil ditambahkan ke antrean simulasi.'}},
    callNext(){const h=buildHeap();if(!h.length)fail('Antrean sedang kosong.');const p=pop(h);const real=state.patients.find(x=>x.id===p.id);real.status='Dipanggil';real.calledOrder=Date.now();save();return{message:'Pasien berhasil dipanggil dalam simulasi.',patient:real}}
  };
})();
const $=s=>document.querySelector(s);
const api=async(path,options={})=>{
  const method=(options.method||'GET').toUpperCase();
  if(method==='GET'){
    if(path==='/api/stats')return store.stats();
    if(path==='/api/patients')return store.patients();
    if(path==='/api/history')return store.history();
    if(path==='/api/heap')return store.heap();
    if(path.startsWith('/api/search?'))return store.search(decodeURIComponent(path.split('id=')[1]||''));
  }else{
    if(path==='/api/patients')return store.register(JSON.parse(options.body));
    if(path==='/api/call-next')return store.callNext();
  }
  throw new Error('Endpoint tidak ditemukan.');
};
function esc(v){return String(v??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));}
function priorityClass(n){return n===1?'p1':n===2?'p2':''}
async function refresh(){try{const [stats,patients,history,heap]=await Promise.all([api('/api/stats'),api('/api/patients'),api('/api/history'),api('/api/heap')]);$('#total').textContent=stats.total;$('#waiting').textContent=stats.waiting;$('#served').textContent=stats.served;$('#topPriority').textContent=stats.waiting?`P${stats.topPriority}`:'—';$('#topId').textContent=stats.waiting?`ID ${stats.topId}`:'Belum ada antrean';$('#queueCount').textContent=`${patients.length} pasien menunggu`;
$('#queueBody').innerHTML=patients.length?patients.map((p,i)=>`<tr><td>${String(i+1).padStart(2,'0')}</td><td>${esc(p.id)}</td><td>${esc(p.name)}</td><td>${p.age}</td><td><span class="priority ${priorityClass(p.priority)}">Prioritas ${p.priority}</span></td><td>${esc(p.registeredAt)}</td><td><span class="status">Menunggu</span></td></tr>`).join(''):'<tr><td colspan="7" class="empty">Belum ada pasien dalam antrean.</td></tr>';
if(heap.length){const levels=[];heap.forEach(node=>(levels[node.level]??=[]).push(node));$('#heapView').innerHTML=levels.map((level,i)=>`<div class="heap-level">${level.map(node=>`<div class="heap-node ${node.index===0?'root':''}"><b>${esc(node.id)}</b><span>Prioritas ${node.priority}</span></div>`).join('')}</div>`).join('')}else{$('#heapView').innerHTML='<div class="empty">Heap masih kosong.</div>'}
$('#historyList').innerHTML=history.length?history.slice().reverse().map(p=>`<div class="history-item"><div><b>${esc(p.name)} <small>(${esc(p.id)})</small></b><small>Prioritas simulasi ${p.priority} · ${esc(p.registeredAt)}</small></div><span>Dipanggil</span></div>`).join(''):'<div class="empty">Belum ada riwayat pemanggilan.</div>';
}catch(e){$('#queueBody').innerHTML=`<tr><td colspan="7" class="empty">${esc(e.message)}</td></tr>`}}
$('#patientForm').addEventListener('submit',async e=>{e.preventDefault();const msg=$('#formMessage');msg.className='message';msg.textContent='';try{await api('/api/patients',{method:'POST',body:JSON.stringify({id:$('#patientId').value.trim(),name:$('#patientName').value.trim(),age:Number($('#patientAge').value),priority:Number($('#patientPriority').value)})});msg.textContent='✓ Data berhasil didaftarkan.';e.target.reset();await refresh()}catch(err){msg.className='message error';msg.textContent=err.message}});
$('#callNext').addEventListener('click',async()=>{const msg=$('#callMessage');msg.className='message';try{const d=await api('/api/call-next',{method:'POST',body:'{}'});msg.textContent=`✓ ${d.patient.name} (${d.patient.id}) berhasil dipanggil.`;await refresh()}catch(err){msg.className='message error';msg.textContent=err.message}});
$('#refresh').addEventListener('click',refresh);
$('#searchForm').addEventListener('submit',async e=>{e.preventDefault();const id=$('#searchId').value.trim();const out=$('#searchResult');try{const p=await api('/api/search?id='+encodeURIComponent(id));out.innerHTML=`<b>${esc(p.name)}</b> (${esc(p.id)})<br>Usia: ${p.age} tahun · Prioritas simulasi: ${p.priority}<br>Status: ${esc(p.status)}<br>Waktu daftar: ${esc(p.registeredAt)}`}catch(err){out.textContent=err.message}});
refresh();setInterval(refresh,10000);


// Workspace multi-halaman: setiap menu menampilkan ruang kerja tersendiri.
(()=>{
 const titles={dashboard:['Dashboard','Pantau dan kelola simulasi antrean pasien dalam satu tempat.'],registration:['Pendaftaran pasien','Kelola data pasien yang masuk ke simulasi antrean.'],queue:['Antrean pasien','Pantau urutan antrean dan lakukan pemanggilan pasien.'],heap:['Visualisasi Min-Heap','Lihat representasi struktur Priority Queue secara visual.'],history:['Riwayat panggilan','Tinjau daftar pasien yang telah dipanggil.'],search:['Pencarian pasien','Temukan data pasien berdasarkan ID menggunakan BST.']};
 const move=(selector,target)=>{const el=document.querySelector(selector),mount=document.querySelector(target);if(el&&mount)mount.appendChild(el)};
 // Pindahkan komponen yang sudah ada ke halaman workspace masing-masing tanpa mengubah ID/API.
 move('.form-card','#registrationMount');
 // Kartu pendaftaran berada di dalam primary-grid pada versi sebelumnya.
 const primary=document.querySelector('.primary-grid');
 if(primary){const reg=primary.querySelector('.registration-card'),call=primary.querySelector('.call-card');if(reg)document.querySelector('#registrationMount').appendChild(reg);if(call)document.querySelector('#callMount').appendChild(call);primary.remove();}
 move('.queue-card','#queueMount');move('.heap-card','#heapMount');
 const bottom=document.querySelector('.bottom-grid');if(bottom){const hist=bottom.querySelector('.history-card'),search=bottom.querySelector('.search-card');if(hist)document.querySelector('#historyMount').appendChild(hist);if(search)document.querySelector('#searchMount').appendChild(search);bottom.remove();}
 function show(){let key=(location.hash||'#dashboard').slice(1);if(!titles[key])key='dashboard';document.querySelectorAll('.workspace-page').forEach(p=>p.classList.toggle('active',p.dataset.view===key));document.querySelectorAll('.nav-link[data-page]').forEach(a=>a.classList.toggle('active',a.dataset.page===key));document.querySelector('#pageTitle').textContent=titles[key][0];document.querySelector('#pageDescription').textContent=titles[key][1];window.scrollTo({top:0,behavior:'smooth'});}
 window.addEventListener('hashchange',show);show();
})();
