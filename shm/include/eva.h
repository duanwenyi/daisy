#ifndef __EVA_H__
#define __EVA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>  
#include <stdint.h>  
#include <sys/shm.h>  
#include <sys/types.h>
#include <sys/ipc.h>

#define DAISY_IDLE   0x51339
#define DAISY_DOOR   0x77777      // door bell

#define DAISY_ASK   0xa5a5a5a5    // ask
#define DAISY_ANS   DAISY_IDLE    // answer
#define DAISY_CRY   0x44444       // daisy cry ! the OP failed !

#define DAISY_WAIT  0x5a5a5a5a    // wait a signal for special value

#define DAISY_STOP  0x33333333    // stop the HDL simulator

#define DAISY_LANG_NUMS 256         // this may not enough  !!!  update later !

/* 
 Daisy Process : 
 HDL:  DAISY_IDLE   |             | DAISY_ECHO    |              | DAISY_ANS (DAISY_IDLE)
 C  :               | DAISY_REQ   |               | DAISY_ACK    | 
    
 HDL:  DAISY_IDLE   |             | ob_tok = 0 , DAISY_IDLE
 C  :               | DAISY_CLEAN |               
  
 */ 


typedef struct DAISY_FACE_S {
  // One 32 bits counter be full to 0xFFFF_FFFF only need about ~ 3 hour
  // So 64 bits is better
  uint64_t tick;  // always increase when posedge clock

  uint32_t action;  // Daisy controller

  char     lang[DAISY_LANG_NUMS];

  uint32_t wmode;   // wait mode : 1:equal  0: not equal
  uint32_t val;
  uint32_t answer;

}DAISY_FACE, *DAISY_FACE_p;


void *eva_map(int do_init);
void eva_unmap(void *map);

void eva_destory();

/* Optimization barrier */  
#define barrier() __asm__ __volatile__("": : :"memory")  

#ifdef __cplusplus
}
#endif

#endif
