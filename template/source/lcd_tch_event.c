#include "lcd_tch_event.h"
#include <stdio.h>

int lcd_tch_event(int * x_s , int * y_s )
{
    int fd_tch;
    //打开触摸屏设备文件
    if((fd_tch = open("/dev/input/event0" , O_RDONLY)) == -1)
    {
        perror("open lcd_touch_event0 error , errno >> ");
        return -1;
    }

    while (1)
    {
        //循环读取事件信息
        struct  input_event tch_event;
        int ret_val = read(fd_tch , &tch_event , sizeof tch_event);

        if(tch_event.type == EV_ABS && tch_event.code == ABS_X)    
            *x_s = tch_event.value*800/1024;
        else if(tch_event.type == EV_ABS && tch_event.code == ABS_Y)
            *y_s = tch_event.value*480/600;
    
        if(tch_event.type == EV_KEY && 
           tch_event.code == BTN_TOUCH && 
           tch_event.value == 0)
        {
            printf(" [%s] (x : %d , y : %d) \n",__FUNCTION__,*x_s,*y_s);
            break;
        }

    }
    
    close(fd_tch);
    return 0;

}

int lcd_tch_slip_event(int * x_tch , int * y_tch)
{
    int fd_tch;
    int x_fs , y_fs;
    int x_es , y_es;
    int x_temp , y_temp;
    //打开触摸屏设备文件
    if((fd_tch = open("/dev/input/event0" , O_RDONLY)) == -1)
    {
        perror("open lcd_touch_event0 error , errno >> ");
        return -1;
    }

    while (1)
    {
        //循环读取事件信息
        struct  input_event tch_event;
        int ret_val = read(fd_tch , &tch_event , sizeof tch_event);

        //当手指按下时
        if(tch_event.type == EV_ABS && tch_event.code == ABS_X)    
            x_temp = tch_event.value*800/1024;
        else if(tch_event.type == EV_ABS && tch_event.code == ABS_Y)
            y_temp = tch_event.value*480/600;
        
        if(tch_event.type == EV_KEY && tch_event.code == BTN_TOUCH)
        {
            //边界处理
            if(x_temp > 800)
                x_temp = 800;
            if(y_temp > 480)
                y_temp = 480;
            //释放
            if(tch_event.value == 0)
            {
                x_es = x_temp ; y_es = y_temp;
                if(y_fs > 60 && y_fs < 70 && y_es < 70 && y_es > 60)
                {
                    if(x_fs > 595 && x_fs < 610 && x_es < 745 && x_es > 725)
                    {
                        printf("[lcd_tch_slip_event]解锁退出\n");return 4;
                    }
                }
                if((x_es - x_fs) > 50)
                {
                    printf("[lcd_tch_slip_event]右滑\n");return 2;
                }                    
                else if((x_es - x_fs) < -50)
                {
                    printf("[lcd_tch_slip_event]左滑\n");return -2;
                }    
                else if((y_es - y_fs) > 50)
                {
                    printf("[lcd_tch_slip_event]上滑\n");return 1;
                }    
                else if((y_es - y_fs) < -50)
                {
                    printf("[lcd_tch_slip_event]下滑\n");return -1;
                }
                else 
                {
                    *x_tch = x_fs ; *y_tch = y_fs;
                    return 3;
                }    
            }
            //按下
            else if(tch_event.value == 1)
            {
                x_fs = x_temp ; y_fs = y_temp;
            }
        }

    }    
    close(fd_tch);
    return 0;

}


