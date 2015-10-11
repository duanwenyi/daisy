#ifndef __DAISY_HDL_DRV_H__
#define __DAISY_HDL_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "eva.h"
#include "svdpi.h"

void daisy_hdl_init();

void daisy_monitor();

int evaScopeGet(char *path, int *succ);

#ifdef __cplusplus
}
#endif


#endif
