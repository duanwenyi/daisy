
#include <signal.h> 
#include "daisy_hdl_drv.h"
#include "vpi_user.h"

#ifndef USING_VCS_COMPILER
#include "vpi_user_cds.h"
#endif

#include <string.h>

DAISY_FACE_p face_t;

void daisy_hdl_init(){
  memset(&face_t, 0, sizeof(DAISY_FACE));

  face_t = eva_map(1);

  fprintf(stderr, " @Daisy's Boy (HDL) is set ALIVE OK .\n");  

}

void daisy_monitor(){

  if( face_t->action == DAISY_CLEAN ){
    face_t->ob_tok = 0;
    fprintf(stderr, " @Daisy Clean tok counter OK.\n");  
    face_t->action = DAISY_IDLE;
  }else if( face_t->action == DAISY_ASK ){

    face_t->action = DAISY_IDLE;
  }else if( face_t->action == DAISY_STOP ){
  
    fprintf(stderr, " @Daisy's STOP OP detected , exit simulation now.\n");  
    face_t->action = DAISY_IDLE;
    exit(0);
    
  }
  // increase cycle counter 
  face_t->ob_tik++;
  face_t->ob_tok++;
}

int evaScopeGet(char *path){
  vpiHandle   net;
  s_vpi_value val;

  net = vpi_handle_by_name(path, NULL);

  if( net == NULL){
    fprintf(stderr,"@evaScopeGet: %s  not exist !\n", path);
    return 0;
  }

  int Vector_size = vpi_get(vpiSize, net);

  if(Vector_size > 32){
    fprintf(stderr,"@evaScopeGet: %s vector size %d > 32 not support !\n", path, Vector_size);
    return 0;
  }{
    val.format = vpiIntVal;
    vpi_get_value(net, & val);

    return val.value.integer;
  }
  
}
