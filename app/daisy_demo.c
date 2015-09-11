#include "daisy_drv.h"


int main(int argc, char **argv){
  
  char signal[] = "TH.cnt";
  int  val;

  daisy_drv_init();

  fprintf(stderr, " A simple test ... \n");
  eva_delay(100);

  val = daisy_query(signal);
  fprintf(stderr, " daisy_query : %s : 0x%x \n", signal, val);

  val = daisy_query(signal);
  fprintf(stderr, " daisy_query : %s : 0x%x \n", signal, val);

  daisy_wait(signal, val + 0x1000, 1);

  sleep(30);
  
  daisy_stop();

  return 0;
}
