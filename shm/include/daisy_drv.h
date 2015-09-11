#ifndef __DAISY_DRV_H__
#define __DAISY_DRV_H__
#include "eva.h"

void daisy_drv_init();
void daisy_stop();

uint32_t daisy_query(char *signal);
void     daisy_wait(char *signal, uint32_t val, int mode);

void eva_delay(int cycle);

#endif
