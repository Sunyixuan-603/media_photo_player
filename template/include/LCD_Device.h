#ifndef __LCD_DEVICE_H
#define __LCD_DEVICE_H
 

//--- 系统头文件 ---//
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>


//--- LCD设备信息结构体 ---//
typedef struct LCD
{
    int    fd;                        //LCD设备文件描述符
    int *  mbuf;                      //LCD映射内存
    struct fb_fix_screeninfo fixinfo; //固定属性
    struct fb_var_screeninfo varinfo; //可变属性
}LCD_Info;


//--- 函数原型 ---//
bool LCD_open(const char * path , const int flags , LCD_Info * info);
void LCD_close(LCD_Info * info);
bool LCD_get_fixinfo(LCD_Info * LCD);
bool LCD_get_varinfo(LCD_Info * LCD);
void LCD_show_info(const LCD_Info LCD);
void LCD_Draw_Color(const int * colorbuf , int start_x , int start_y , int width , int heigh , LCD_Info LCD);
void LCD_Draw_Point(const int  color , int start_x , int start_y , int width , int heigh , LCD_Info LCD);



#endif