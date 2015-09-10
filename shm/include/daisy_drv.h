#ifndef __DAISY_DRV_H__
#define __DAISY_DRV_H__
#include "eva.h"

void daisy_drv_init();
void daisy_stop();

uint64_t daisy_query(char *signal);

void eva_delay(int cycle);

#endif
