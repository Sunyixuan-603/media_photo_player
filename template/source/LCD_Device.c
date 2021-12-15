#include "LCD_Device.h"


/************************************************************************************************************************************
//                                                         打开LCD
//parameters：
// @ path：LCD设备文件的路径
// @ flags：以何种方式打开LCD
// @ info：存放LCD信息结构体的缓冲区
//return value：
// 成功：返回true
// 失败：返回false
************************************************************************************************************************************/
bool LCD_open(const char * path , const int flags , LCD_Info * info)
{
    //--- 缓冲区不存在则之间返回false ---//
    if(info == NULL)
    {
        return false;
    }

    //--- 清空LCD信息结构体 ---//
    bzero(info , sizeof(LCD_Info));

    //--- 以指定方式打开LCD设备文件，并记录设备文件的文件描述符 ---//
	info->fd = open(path , flags); 	
	if(info->fd == -1)
	{
        printf("open LCD failed: %s\n" , strerror(errno));
		goto open_lcd_failed;	
	}

    //--- 获取LCD的固定属性 ---//
    if(LCD_get_fixinfo(info) == false)
    {
        printf("get LCD fixinfo failed\n");
        goto init_info_failed;
    }
	
    //--- 获取LCD的可变属性 ---//
    if(LCD_get_varinfo(info) == false)
    {
        printf("get LCD varinfo failed\n");
        goto init_info_failed;
    }

    //--- 计算映射内存的大小（以字节为单位），注意LCD屏幕的一个像素点为4个字节 ---//
    size_t size = info->varinfo.xres_virtual * info->varinfo.yres_virtual * 4;

    //--- 映射一片内存，对内存进行写操作相对于对LCD进行写操作，但对映射内存的操作更为快速 ---//
	info->mbuf = mmap(NULL ,        //映射内存的起始地址设置为NULL，表示由系统自动寻找一个合适的起始地址
                      size ,        //映射内存的大小
                      PROT_WRITE | PROT_READ ,  //映射内存的权限为可读可写
                      MAP_SHARED ,  //共享映射内存，所有的同时映射了这块内存的进程对数据的变更均可见，而且数据的变更会直接同步到对应的文件（有时候还需要调用msync()或者munmap()才会真正起作用）
                      info->fd ,    //要映射的文件的描述符
                      0);  		    //文件映射的开始区域偏移量 
	if(info->mbuf == MAP_FAILED)//返回MAP_FAILED(-1)则内存映射失败
	{
		printf("mmap LCD memory failed: %s\n" , strerror(errno));
		goto init_info_failed;	
	}

    // 将起始可见区设定为A区
    info->varinfo.xoffset = 0;
    info->varinfo.yoffset = 0;
    ioctl(info->fd , FBIOPAN_DISPLAY, &info->varinfo);

    return true;

init_info_failed:
    close(info->fd);

open_lcd_failed:
    return false;
}


/************************************************************************************************************************************
//                                                         关闭LCD
//parameters：
// @ info：存放LCD信息结构体的缓冲区
//return value：
// 无
************************************************************************************************************************************/
void LCD_close(LCD_Info * info)
{
    //--- 判断映射内存是否存在 ---//
    if(info->mbuf != NULL)
    {
        //--- 解除内存映射 ---//
        if(munmap(info->mbuf , info->varinfo.xres_virtual * info->varinfo.yres_virtual * 4) == -1)
        { 
            perror("munmap LCD buf failed: ");
        }
        else
        {
            info->mbuf = NULL;
        }
    }
    else 
    {
        printf("LCD mbuf is NULL\n");
    }

    //--- 关闭打开的文件 ---//
    close(info->fd);
}


/************************************************************************************************************************************
//                                                         获取LCD的固定属性
//parameters：
// @ info：存放LCD信息结构体的缓冲区
//return value：
// 成功：返回true
// 失败：返回false
************************************************************************************************************************************/
bool LCD_get_fixinfo(LCD_Info * LCD)
{
    if(ioctl(LCD->fd, FBIOGET_FSCREENINFO , &LCD->fixinfo) != 0)
    {
        return false;
    }

    return true;
}


/************************************************************************************************************************************
//                                                         获取LCD的可变属性
//parameters：
// @ info：存放LCD信息结构体的缓冲区
//return value：
// 成功：返回true
// 失败：返回false
************************************************************************************************************************************/
bool LCD_get_varinfo(LCD_Info * LCD)
{
    if(ioctl(LCD->fd, FBIOGET_VSCREENINFO , &LCD->varinfo) != 0)
    {
        return false;
    }

    return true;
}


/************************************************************************************************************************************
//                                                         显示LCD的相关属性
//parameters：
// @ info：存放LCD信息结构体的缓冲区
//return value：
// 成功：返回true
// 失败：返回false
************************************************************************************************************************************/
void LCD_show_info(const LCD_Info LCD)
{
    //获取LCD设备硬件fix属性
    printf("[ID]: %s\n", LCD.fixinfo.id);
    printf("[FB类型]: ");
    switch(LCD.fixinfo.type)
    {
    case FB_TYPE_PACKED_PIXELS:      printf("组合像素\n");break;
    case FB_TYPE_PLANES:             printf("非交错图层\n");break;
    case FB_TYPE_INTERLEAVED_PLANES: printf("交错图层\n");break;
    case FB_TYPE_TEXT:               printf("文本或属性\n");break;
    case FB_TYPE_VGA_PLANES:         printf("EGA/VGA图层\n");break;
    }
    printf("[FB视觉]: ");
    switch(LCD.fixinfo.visual)
    {
    case FB_VISUAL_MONO01:             printf("灰度. 1=黑;0=白\n");break;
    case FB_VISUAL_MONO10:             printf("灰度. 0=黑;1=白\n");break;
    case FB_VISUAL_TRUECOLOR:          printf("真彩色\n");break;
    case FB_VISUAL_PSEUDOCOLOR:        printf("伪彩色\n");break;
    case FB_VISUAL_DIRECTCOLOR:        printf("直接彩色\n");break;
    case FB_VISUAL_STATIC_PSEUDOCOLOR: printf("只读伪彩色\n");break;
    }
    printf("[行宽]: %d 字节\n", LCD.fixinfo.line_length);

    //LCD设备硬件var属性
    printf("[可见区分辨率]: %d×%d\n", LCD.varinfo.xres, LCD.varinfo.yres);
    printf("[虚拟区分辨率]: %d×%d\n", LCD.varinfo.xres_virtual, LCD.varinfo.yres_virtual);
    printf("[从虚拟区到可见区偏移量]: (%d,%d)\n", LCD.varinfo.xoffset, LCD.varinfo.yoffset);
    printf("[色深]: %d bits\n", LCD.varinfo.bits_per_pixel);
    printf("[像素内颜色结构]:\n");
    printf("  [红] 偏移量:%d, 长度:%d bits\n", LCD.varinfo.red.offset, LCD.varinfo.red.length);
    printf("  [绿] 偏移量:%d, 长度:%d bits\n", LCD.varinfo.green.offset, LCD.varinfo.green.length);
    printf("  [蓝] 偏移量:%d, 长度:%d bits\n", LCD.varinfo.blue.offset, LCD.varinfo.blue.length);
    printf("  [透明度] 偏移量:%d, 长度:%d bits\n", LCD.varinfo.transp.offset, LCD.varinfo.transp.length);
    printf("\n");
}


/************************************************************************************************************************************
//                                                        显示像素点到LCD上
//parameters：
// @ colorbuf：存放像素点数据的缓冲区
// @ start_x：LCD上的起始x坐标
// @ start_y：LCD上的起始y坐标
// @ width：期望显示的宽度
// @ heigh：期望显示的高度
// @ info：存放LCD信息结构体的缓冲区
//return value：
// 无
//
// 通过该函数可以实现，将一个图片像素点，显示到LCD的任意位置，这里有进行越界处理
************************************************************************************************************************************/
void LCD_Draw_Color(const int * colorbuf , int start_x , int start_y , int width , int heigh , LCD_Info LCD)
{
    if(colorbuf == NULL)
    {
        printf("color buf is NULL\n");
        return;
    }

    for(int y=0 ; y<heigh ; y++)
    {
        for(int x=0 ; x<width ; x++)
        {
            //--- 注意这里要进行越界判断，防止出现段错误 ---//
            if((start_x + x < LCD.varinfo.xres) && (start_y + y < LCD.varinfo.yres))
            {
                *(LCD.mbuf + (start_y + y)*LCD.varinfo.xres + start_x + x) = *(colorbuf + y*width + x);
            } 
        }
    }
}


/************************************************************************************************************************************
//                                                  在LCD的指定区域范围内显示一种颜色
//parameters：
// @ color：要显示的颜色数据
// @ start_x：LCD上的起始x坐标
// @ start_y：LCD上的起始y坐标
// @ width：期望显示的宽度
// @ heigh：期望显示的高度
// @ info：存放LCD信息结构体的缓冲区
//return value：
// 无
************************************************************************************************************************************/
void LCD_Draw_Point(const int  color , int start_x , int start_y , int width , int heigh , LCD_Info LCD)
{
    for(int y=0 ; y<heigh ; y++)
    {
        for(int x=0 ; x<width ; x++)
        {
            //--- 注意这里要进行越界判断，防止出现段错误 ---//
            if((start_x + x < LCD.varinfo.xres) && (start_y + y < LCD.varinfo.yres))
            {
                *(LCD.mbuf + (start_y + y)*LCD.varinfo.xres + start_x + x) = color;
            } 
        }
    }
}
