#include <string.h>
#include <pthread.h>
#include "daisy_drv.h"

static DAISY_FACE_p face_t = NULL;

pthread_t daisy_monitor;

#define EVA_SAFE_MODE
//#define EVA_DEBUG

#ifdef __cplusplus
void *daisy_monitor_handler(void *){
#else
void daisy_monitor_handler(void){
#endif
  uint64_t local_time = 0;

  uint64_t max_rate = 0;
  uint64_t min_rate = 0xFFFFFFFFFFFFFFFF;
  uint64_t eva_rate = 0;

  while(1){
    sleep(1);
    local_time++;
    eva_rate = face_t->ob_tok/local_time;
    if(eva_rate > max_rate)
      max_rate = eva_rate;

    if(eva_rate < min_rate)
      min_rate = eva_rate;

    fprintf(stderr, " @Daisy's Monitor: %lld S  - HDL: [0x%llx | 0x%llx] CYCLE  --> %lld (CYCLE/S) [MAX/MIN][%lld / %lld] CYCLE/S\r",
	    local_time, face_t->ob_tok, face_t->ob_tik,  eva_rate, max_rate,  min_rate);  
    
  }

}

void daisy_drv_init(){
  int ret;
  face_t = (DAISY_FACE *)eva_map(0);
  if( face_t->action != DAISY_IDLE){
    fprintf(stderr, " @Daisy's Boy (HDL) is not detected start first , exit .\n");  
    exit(EXIT_FAILURE);  
  }
  
  face_t->action = DAISY_CLEAN;

  while(face_t->action == DAISY_CLEAN ){
    usleep(1);
  }
  
#ifdef __cplusplus
  ret = pthread_create(&daisy_monitor, NULL, daisy_monitor_handler, NULL);
#else
  ret = pthread_create(&daisy_monitor, NULL, (void *)daisy_monitor_handler, NULL);
#endif

  if(ret != 0){
    fprintf(stderr, " @Daisy's Monitor thread created failed , exit .\n");  
    exit(EXIT_FAILURE);  
  }
  
  fprintf(stderr, " @Daisy's nice day ...\n");  

}

void daisy_stop(){
  face_t->action = DAISY_STOP;
}

uint64_t daisy_query(char *signal){
  while(face_t->action != DAISY_IDLE){
  }
  
  memcpy(face_t->lang , signal, DAISY_LANG_NUMS);
  barrier();

  face_t->action = DAISY_ASK;

#ifdef EVA_DEBUG
  fprintf(stderr," face_t->lang : %s\n",face_t->lang );
  fprintf(stderr,"       signal : %s\n",signal );
#endif
  while(face_t->action == DAISY_ASK){
  }

  barrier();
  
  if(face_t->action == DAISY_CRY){
    fprintf(stderr," @Daisy CRY : please check signal [%s] \n",face_t->lang );
    return 0;
  }else{
    return face_t->answer;
  }

}

void eva_delay(int cycle){
  uint64_t mark = face_t->ob_tok;
  uint64_t mark2;
  int grap;
  do{
    mark2 = face_t->ob_tok;
    grap = mark2 - mark;
    if(cycle > 20)
      usleep(1);
  }while( grap < cycle);
  
  fprintf(stderr," @EVA delayed  %d HDL CYCLE [%lld -> %lld]\n", cycle, mark, mark2 );
}
