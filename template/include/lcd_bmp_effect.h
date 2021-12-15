#ifndef __LCD_BMP_EFFECT_H__
#define __LCD_BMP_EFFECT_H__

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FB_PATH "/dev/fb0"      //LCD�豸

extern unsigned int *mem_p; //�����޷�������ָ�룬���ڴ��ӳ���ַ
//extern int lcd_fd;
//��Ļ��ʼ��
extern void LCD_init(void);

//LCD����
extern void LCD_UnInit(void);


//const char *pathname:ͼƬ·��
//int start_x��ͼƬ��ʼλ�� x��
//int start_y��ͼƬ��ʼλ�� y��
//int left_offset:ͼƬ��ƫ����
//int right_offset:ͼƬ��ƫ����
extern  void show_bmp_Xoffset(const char *pathname,int start_x,int start_y,int left_offset,int right_offset);
extern  void show_bmp_Yoffset(const char *pathname,int start_x,int start_y,int up_offset,int down_offset);
extern int show_scalebmp(const char *pathname,int start_x,int start_y,int d_w,int d_h);
//const char *pathname:ͼƬ·��
//int start_x��ͼƬ��ʼλ�� x��
//int start_y��ͼƬ��ʼλ�� y��
extern  void show_bmp(const char *pathname,int start_x,int start_y);
extern void Big_PictureByPoint(const char *pathname,int pointX,int pointY);
extern void show_smallbmp(unsigned char o_picture[],int o_w,int o_h,int start_x,int start_y,int d_w,int d_h);
extern void ExpandPicture(const char *pathname,int pointX,int pointY);
extern int bmp_display_effect( const char * FileName , int * p_lcd ,  int x_s , int y_s , int mode);
#endif