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
	
		uint64_t pre_tick = 0;
	
		uint64_t max_rate = 0;
		uint64_t min_rate = 0xFFFFFFFFFFF00000;
		uint64_t eva_rate = 0;
	
		uint64_t initial = 1;

		while(1){
		
			pre_tick = face_t->tick;
			sleep(1);
			eva_rate = face_t->tick - pre_tick;

			if(initial ){
				if(eva_rate != 0){
					max_rate = eva_rate;
					min_rate = eva_rate;
				}
			}
		
			local_time++;
			if(eva_rate > max_rate)
				max_rate = eva_rate;
			
			if( (eva_rate < min_rate)  && 
				(eva_rate !=0 ) &&
				( (uint64_t)(min_rate - eva_rate) < (eva_rate/4) )  // fix stop action case
				)
				min_rate = eva_rate;

			fprintf(stderr, " @Daisy's Monitor: %lld S  - HDL: 0x%llx CYCLE  --> %lld (CYCLE/S) [MAX/MIN][%lld / %lld] CYCLE/S\r",
					local_time, face_t->tick, eva_rate, max_rate,  min_rate);  

			initial = 0;
		}
}

 void daisy_drv_init(){
	 int ret;
	 face_t = (DAISY_FACE *)eva_map(0);
	 fprintf(stderr, " @Daisy's Boy (HDL) is 0x%x.\n",face_t->action);  
	 face_t->action = DAISY_DOOR;

	 while(face_t->action == DAISY_DOOR ){
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

uint32_t daisy_query(char *signal){
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
    usleep(1);
  }

  barrier();
  
  if(face_t->action == DAISY_CRY){
    fprintf(stderr," @Daisy CRY : please check signal [%s] \n",face_t->lang );
    return 0;
  }else{
    return face_t->answer;
  }
  
}

void daisy_wait(char *signal, uint32_t val, int mode){
   // mode :  1: equal to    0: not equal to
  while(face_t->action != DAISY_IDLE){
  }

  barrier();

  memcpy(face_t->lang , signal, DAISY_LANG_NUMS);
  face_t->wmode  = mode;
  face_t->val    = val;

  barrier();
  face_t->action = DAISY_WAIT;

  while(face_t->action == DAISY_WAIT){
    usleep(1);
  }

  fprintf(stderr," @Daisy waited : [%s] %s 0x%x \n",
	  face_t->lang, face_t->wmode ? "==":"!=", face_t->val );
}

void eva_delay(int cycle){
  uint64_t mark = face_t->tick;
  uint64_t mark2;
  int grap;
  do{
    mark2 = face_t->tick;
    grap = mark2 - mark;
    if(cycle > 20)
      usleep(1);
  }while( grap < cycle);
  
  fprintf(stderr," @EVA delayed  %d HDL CYCLE [%lld -> %lld]\n", cycle, mark, mark2 );
}
