#ifndef __LCD_TCH_EVENT_H__
#define __LCD_TCH_EVENT_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "linux/input.h"


int lcd_tch_event(int * x_s , int * y_s );
int lcd_tch_slip_event(int * x_tch , int * y_tch);


#endif