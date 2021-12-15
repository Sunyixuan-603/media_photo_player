#ifndef __INCLUDES_H__
#define __INCLUDES_H__

/*标准C函数库*/
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <stdbool.h>

/*自定义库*/
#include "lcd_tch_event.h"      //触摸屏坐标捕获
#include "lcd_jpeg.h"            //jpeg
#include "dir_search.h"     //目录检索
#include "list.h"           
#include "font.h"           
#include "ScalePicture.h"
#include "display_bmp.h"
#include "lcd_bmp_effect.h"
#include "LCD_Device.h"
#include "qrencode.h"

/*全局宏声明*/
#define UP     1
#define DOWN  -1
#define PRESS  3
#define RIGHT  2
#define LEFT  -2
#define QUIT   4


#define PASSWD_STA            "603328"
#define PIC_PATH              "/template/picture"
#define PIC_UNLOCK_PATH       "/template/picture/unlock"
#define PIC_LOGIN_PATH        "/template/picture/login"
#define PIC_SCRAPE_PATH       "/template/picture/scrape"
#define PIC_PHOTO_PATH        "/template/picture/photo"
#define PIC_PHOTO_LIST_PATH   "/template/picture/photo_list"
#define PIC_PHOTO_BMP_PATH    "/template/picture/photo_list/bmp"
#define MUSIC_PATH            "/template/music/"
#define VIDEO_PATH            "/template/video/"

#define MPLAY_MUSIC_SHELL     "mplayer -quiet /template/music/%s &"
#define MPLAY_VIDEO_SHELL     "mplayer -quiet -x 800 -y 480 /template/video/%s &"
#define MPLAY_PHOTO_BGM       "mplayer -quiet /template/picture/photo/background.mp3 &"
#define MPLAY_QUIT_PROGRAM    "mplayer -quiet -x 800 -y 480 /template/systemlogin/close.mp4 &"
#define MPLAY_START_PROGRAM   "mplayer -quiet -slave -x 800 -y 480 /template/systemlogin/start_fast.mp4"
#define UDISK_PATH            "/mnt/udisk"
#define UDISK_MUSIC_PATH      "/mnt/udisk/template/music"
#define UDISK_VIDEO_PATH      "/mnt/udisk/template/video"
#define UDISK_PIC_PATH        "/mnt/udisk/template/picture"


/*全局变量*/
static unsigned char g_color_buf[FB_SIZE]={0};
static char buf_path[128] = {0};
static int  lcd_fd;
static char  qrcode_passwd[6] = {0};
LCD_Info Mylcd;

/*菜单*/
void login_menu_interface(void);
void interface_menu(void);
int music_display_func(void);
int video_display_func(void);
int photo_display_func(void);

void music_shell_display(int count,int *music_count,char *music_buf,char **music_file,bitmap *bm_play,font *f_file);

/*相册*/
void bmp_photo_rm_func(const char ** bmp_file,int *file_number,int cur_page,int line,int x_s,int y_s);
void font_photo_handle(const char *string,struct LcdDevice* lcd_font);

struct LcdDevice *lcd_font_init(const char *device );
void font_display_func(const char **string, int loop_count);
void unlock_display_func(const char * , int * , int );
void login_display_func(const char * );
void photo_pic_ch_display(const char *filename, int * count,int ch);

/*qrcode二维码*/
void draw_qrcode_point(int *lcd,int color,int size,int px,int py);
void qrcode_display(const char *string,int start_x,int start_y);

/*刮刮乐*/
int scrape_display_func(void);
int scrape_tch_event(int * , int * , int);
int scrape_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path);  

/*目录检索*/
int dir_cpy_func(const char *src_file ,const char *tar_file );
bool udisk_list_func(P_Node head, int * num ,const char * path);

/*宏函数定义*/


#endif

