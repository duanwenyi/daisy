#ifdef __cplusplus
extern "C" {
#endif

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

  face_t = (DAISY_FACE_S*) eva_map(1);

  fprintf(stderr, " @Daisy's Boy (HDL) is set ALIVE OK .\n");  

}

void daisy_monitor(){
  int         succ;  // 1:success get value  0: failure

  if(face_t->action == DAISY_DOOR){
	  
	  fprintf(stderr, " @Daisy is coming.\n");  
	  face_t->action = DAISY_IDLE;

  }else if( face_t->action == DAISY_ASK ){

	  face_t->answer = evaScopeGet(face_t->lang, &succ);
	  if(succ){
		  fprintf(stderr, " @Daisy query : %s  : 0x%x\n", face_t->lang, face_t->answer);  
		  face_t->action = DAISY_IDLE;
	  }else{
		  fprintf(stderr, " @Daisy query : %s  : 0x%x  failured !\n", face_t->lang, face_t->answer);  
		  face_t->action = DAISY_CRY;
	  }
  }else if( face_t->action == DAISY_WAIT ){

	  face_t->answer = evaScopeGet(face_t->lang, &succ);
	  if(succ){
		  if( (face_t->wmode && (face_t->val == face_t->answer)) ||
			  (!face_t->wmode && (face_t->val != face_t->answer)) ){
			  fprintf(stderr," @Daisy waited : [%s] %s 0x%x \n",
					  face_t->lang, face_t->wmode ? "==":"!=", face_t->val );
			  face_t->action = DAISY_IDLE;
		  }
	  }else{
		  fprintf(stderr, " @Daisy query : %s  : 0x%x  failured !\n", face_t->lang, face_t->answer);  
		  face_t->action = DAISY_CRY;
	  }
  }else if( face_t->action == DAISY_STOP ){
  
	  fprintf(stderr, " @Daisy's STOP OP detected , exit simulation now.\n");  
	  face_t->action = DAISY_IDLE;
	  exit(0);
    
  }
  // increase cycle counter 
  face_t->tick++;
}

int evaScopeGet(char *path, int *succ){
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
    *succ = 0;
    return 0;
  }{
    val.format = vpiIntVal;
    vpi_get_value(net, & val);
    *succ = 1;

    return val.value.integer;
  }
  
}


#ifdef __cplusplus
}
#endif
