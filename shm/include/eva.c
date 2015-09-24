#include "eva.h"

void *eva_map(int do_init){
  void *shm = NULL;  
  key_t key = ftok(".", 1); 
  DAISY_FACE_p face;
  int   shmid = shmget(key, sizeof(DAISY_FACE), 0666|IPC_CREAT);  
  if(shmid == -1)  
    {  
      fprintf(stderr, "shmget failed\n");  
      exit(EXIT_FAILURE);  
    }  

  shm = shmat(shmid, NULL , 0);  

  if(shm == (void*)-1)  
    {  
      fprintf(stderr, "shmat failed\n");  
      exit(EXIT_FAILURE);  
    }  
  fprintf(stderr, " @Daisy initialed @key %x [%d]\n",key,do_init);  

  if(do_init){
    face = (DAISY_FACE *)shm;

    face->action    = DAISY_IDLE;
  }

  return shm;
}

void eva_unmap(void *map){
  if(shmdt(map) == -1)  
    {  
      fprintf(stderr, "shmdt failed\n");  
      exit(EXIT_FAILURE);  
    }  
}


void eva_destory(){
  void *shm = NULL;  
  key_t key = ftok(".", 1); 
  int   shmid = shmget(key, sizeof(DAISY_FACE), 0666|IPC_CREAT);  
  if(shmid == -1)  
    {  
      fprintf(stderr, "shmget failed\n");  
      exit(EXIT_FAILURE);  
    }  

  shm = shmat(shmid, NULL , 0);  
  
  if(shm == (void*)-1)  
    {  
      fprintf(stderr, "shmat failed\n");  
      exit(EXIT_FAILURE);  
    }  

  if(shmdt(shm) == -1)  
    {  
      fprintf(stderr, "shmdt failed\n");  
      exit(EXIT_FAILURE);  
    }  

  //删除共享内存  
  if(shmctl(shmid, IPC_RMID, 0) == -1)  
    {  
      fprintf(stderr, "shmctl(IPC_RMID) failed\n");  
      exit(EXIT_FAILURE);  
    }  

  fprintf(stderr, " @Daisy Destory @key %x\n",key);  
  exit(EXIT_SUCCESS);  
}

