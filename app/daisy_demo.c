#include "daisy_drv.h"


int main(int argc, char **argv){
  daisy_drv_init();

  fprintf(stderr, " A simple test ... \n");
  eva_delay(100);

  sleep(30);
  
  daisy_stop();

  return 0;
}
