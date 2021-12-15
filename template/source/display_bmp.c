#include "Display_bmp.h"


/**************************************************************************************************************************************
//                                                     显示一张BMP图片
//parameters：
// @ path：BMP图片的路径
// @ start_x：LCD上显示的起始x坐标
// @ start_y：LCD上显示的起始y坐标
// @ LCD：LCD设备信息结构体
//Return Value:
// 无
**************************************************************************************************************************************/
void Dis_BMP(const char * path , int start_x , int start_y , LCD_Info LCD)
{
    int fd_bmp;     //bmp图片文件描述符

    //--- 打开bmp图片文件 ---//
    if((fd_bmp = open(path , O_RDONLY)) == -1)
    {
        printf("open %s failed: %s\n" , path , strerror(errno));
        return;
    }

    //--- 读取文件头信息 ---//
    BITMAPFILEHEADER BMP_fileHead;
    if(read(fd_bmp , &BMP_fileHead , sizeof(BITMAPFILEHEADER)) == -1)
    {
        printf("read BMP file head failed: %s\n" , strerror(errno));
        close(fd_bmp); 
        return;
    }
    
    //--- 读取文件的信息头 ---//
    BITMAPINFOHEADER BMP_infoHead;
    if(read(fd_bmp , &BMP_infoHead , sizeof(BITMAPINFOHEADER)) == -1)
    {
        printf("read BMP info head failed: %s\n" , strerror(errno));
        close(fd_bmp); 
        return;
    }

    int extra;  //记录每行额外需要补偿的字节数

    //--- 计算每行补偿的字节数 ---//
    //位图的大小(其中包含了为了补齐列数是4的倍数而添加的空字节)，以字节为单位（35-38字节）,注意这里是要使
    //得每行的字节数为4的倍数,不是像素点的个数为4的倍数,像素点的个数还是BMP_infoHead.biWidth*BMP_infoHead.biHeight
    if((BMP_infoHead.biWidth * 3) % 4 != 0)       //判断释放要补偿字节
    {
        extra = 4 - ((BMP_infoHead.biWidth * 3) % 4);     //先计算每行字节数对4求余后的结果,看需要补偿几个字节
    }
    else
    {
        extra = 0;
    }

    //注意这里BMP图片的像素点为24位的,即每个像素点为3个字节
    char BMP_crlor_buf[(BMP_infoHead.biWidth * 3 + extra) * BMP_infoHead.biHeight];     //保存bmp图片像素点数据，这里是以字节为单位，每3个字节表示一个像素点（r、g、b）
    int LCD_color_buf[BMP_infoHead.biWidth * BMP_infoHead.biHeight];    //保存bmp图片的像素点，这里是以整型4个字节为单位，LCD上的像素点为4个字节（a、r、g、b）                 

    //--- 清空缓冲区 ---//
    bzero(BMP_crlor_buf , sizeof(BMP_crlor_buf));

    //--- 读取BMP图片像素点数据 ---//
    if(read(fd_bmp , BMP_crlor_buf , sizeof(BMP_crlor_buf)) == -1)
    {
        printf("read BMP color failed: %s\n" , strerror(errno));
        goto read_color_failed;
    }

    //--- 清空缓冲区 ---//
    bzero(LCD_color_buf , sizeof(LCD_color_buf));
 
    //--- 将读取到的字节数据合成为一个像素点数据 ---//
    for(int y=0 ; y<BMP_infoHead.biHeight ; y++)
    {
        for(int x=0 ; x<BMP_infoHead.biWidth ; x++)
        {
            LCD_color_buf[y*BMP_infoHead.biWidth + x] = BMP_crlor_buf[(y*(BMP_infoHead.biWidth*3 + extra) + x*3)  + 2] << 16 |  //Red
                                                        BMP_crlor_buf[(y*(BMP_infoHead.biWidth*3 + extra) + x*3)  + 1] << 8  |  //Green
                                                        BMP_crlor_buf[(y*(BMP_infoHead.biWidth*3 + extra) + x*3)  + 0] << 0;    //Blue
        }
    }

    //--- 将像素点数据写入到屏幕上 ---//
    for(int y=0 ; y<BMP_infoHead.biHeight ; y++)
    {
        for(int x=0 ; x<BMP_infoHead.biWidth ; x++)
        {
            if((start_y+y < 480 && start_y+y >= 0) && (start_x+x < 800 && start_x+x >= 0))
            {
                //这里要注意，从BMP图片文件中读取到的像素点是倒立的，即bmp图片文件中的第一行是LCD屏幕上的最好一行
                *(LCD.mbuf + (start_y+y)*LCD.varinfo.xres + start_x + x) = LCD_color_buf[(BMP_infoHead.biHeight - y - 1) * BMP_infoHead.biWidth + x];
            }
            
        }
    }

    //--- 关闭bmp图片文件 ---//
    close(fd_bmp);

    return;

read_color_failed:
    close(fd_bmp);
}


/**************************************************************************************************************************************
//                                                    获取BMP图片的像素点数据
//parameters：
// @ path：BMP图片的路径
// @ start_x：图片上的起始x坐标（这个坐标是相对于图片长宽高的）
// @ start_y：图片上的起始y坐标（这个坐标是相对于图片长宽高的）
// @ width：获取像素点的宽度
// @ heigh：获取像素点的高度
// @ colorbuf：存放像素点数据的缓冲区
//Return Value:
// 成功：返回true
// 失败：返回false
//
// 通过该函数可以获取bmp图片任意区域范围内的像素点，即通过start_x与start_y可以设置图片上的起始坐标，通过width与heigh可以设置要获取的像素点的区域
//范围
**************************************************************************************************************************************/
bool BMP_GetColorBuf(const char * path , int start_x , int start_y , int width , int heigh , int * colorbuf)
{
    int fd_bmp;     //bmp图片文件描述符

    //--- 判断缓冲区释放存在 ---//
    if(colorbuf == NULL)
    {
        printf("buffer is NULL\n");
        return false;
    }

    //--- 打开bmp图片 ---//
    if((fd_bmp = open(path , O_RDONLY)) == -1)
    {
        printf("open %s failed: %s\n" , path , strerror(errno));
        return false;
    }

    //--- 读取文件头信息 ---//
    BITMAPFILEHEADER BMP_fileHead;
    if(read(fd_bmp , &BMP_fileHead , sizeof(BITMAPFILEHEADER)) == -1)
    {
        printf("read BMP file head failed: %s\n" , strerror(errno));
        close(fd_bmp); 
        return false;
    }

    //--- 读取文件的信息头 ---//
    BITMAPINFOHEADER BMP_infoHead;
    if(read(fd_bmp , &BMP_infoHead , sizeof(BITMAPINFOHEADER)) == -1)
    {
        printf("read BMP info head failed: %s\n" , strerror(errno));
        close(fd_bmp); 
        return false;
    }

    int extra;  //记录每行额外需要补偿的字节数

    //--- 计算每行补偿的字节数 ---//
    //位图的大小(其中包含了为了补齐列数是4的倍数而添加的空字节)，以字节为单位（35-38字节）,注意这里是要使
    //得每行的字节数为4的倍数,不是像素点的个数为4的倍数,像素点的个数还是BMP_infoHead.biWidth*BMP_infoHead.biHeight
    if((BMP_infoHead.biWidth * 3) % 4 != 0)       //判断释放要补偿字节
    {
        extra = 4 - ((BMP_infoHead.biWidth * 3) % 4);     //先计算每行字节数对4求余后的结果,看需要补偿几个字节
    }
    else
    {
        extra = 0;
    }

    //注意这里BMP图片的像素点为24位的,即每个像素点为3个字节
    char BMP_crlor_buf[(BMP_infoHead.biWidth * 3 + extra) * BMP_infoHead.biHeight]; //保存bmp图片像素点数据，这里是以字节为单位，每3个字节表示一个像素点（r、g、b）

    //--- 清空缓冲区 ---//
    bzero(BMP_crlor_buf , sizeof(BMP_crlor_buf));

    //--- 读取BMP图片像素点字节数据 ---//
    if(read(fd_bmp , BMP_crlor_buf , sizeof(BMP_crlor_buf)) == -1)
    {
        printf("read BMP color failed: %s\n" , strerror(errno));
        goto read_color_failed;
    }

    //--- 将像素点字节数据合成为一个像素点数据存放到缓冲区中 ---//
    for(int y=0 ; y<heigh ; y++)
    {
        for(int x=0 ; x<width ; x++)
        {

            colorbuf[y*width + x] = BMP_crlor_buf[((BMP_infoHead.biHeight - start_y - y -1)*(BMP_infoHead.biWidth*3 + extra) + (x+start_x)*3)  + 2] << 16 | 
                                    BMP_crlor_buf[((BMP_infoHead.biHeight - start_y - y -1)*(BMP_infoHead.biWidth*3 + extra) + (x+start_x)*3)  + 1] << 8  |
                                    BMP_crlor_buf[((BMP_infoHead.biHeight - start_y - y -1)*(BMP_infoHead.biWidth*3 + extra) + (x+start_x)*3)  + 0] << 0;
        }
    }

    //--- 关闭bmp图片文件 ---//
    close(fd_bmp);
    return true;


read_color_failed:
    close(fd_bmp); 
    return NULL;
}


/**************************************************************************************************************************************
//                                                    任意大小显示一张BMP图片
//parameters：
// @ path：BMP图片的路径
// @ start_x：LCD上显示的起始x坐标
// @ start_y：LCD上显示的起始y坐标
// @ d_w：期望显示的图片宽度
// @ d_h：期望显示的图片高度
// @ LCD：LCD信息结构体
//Return Value:
// 无
//
// 通过该函数可以将一张bmp图片进行任意缩放，并显示在LCD的指定位置上
**************************************************************************************************************************************/
void Dis_ScaleBMP(const char * path , int start_x , int start_y , int d_w , int d_h , LCD_Info LCD)
{
    int fd_bmp;     //bmp图片文件描述符

    //--- 打开bmp图片文件 ---//
    fd_bmp = open(path , O_RDONLY);
    if(fd_bmp == -1)
    {
        printf("open %s failed: %s\n" , path , strerror(errno));
        return;
    }

    //--- 读取文件头信息 ---//
    BITMAPFILEHEADER BMP_fileHead;
    read(fd_bmp , &BMP_fileHead , sizeof(BITMAPFILEHEADER));
 

    //--- 读取文件的信息头 ---//
    BITMAPINFOHEADER BMP_infoHead;
    read(fd_bmp , &BMP_infoHead , sizeof(BITMAPINFOHEADER));


    //注意这里BMP图片的像素点为24位的,即每个像素点为3个字节
    char BMP_crlor_buf[BMP_infoHead.biWidth * BMP_infoHead.biHeight * 3];   //保存bmp图片像素点数据，这里是以字节为单位，每3个字节表示一个像素点（r、g、b）
    char d_colorbuf[d_w * d_h * 3];     //存放图像缩放后像素点数据

    //--- 清空缓冲区 ---//
    bzero(BMP_crlor_buf , sizeof(BMP_crlor_buf));

    //--- 读取BMP图片像素点字节数据 ---//
    read(fd_bmp , BMP_crlor_buf , sizeof(BMP_crlor_buf));

    //--- 清空缓冲区 ---//
    bzero(d_colorbuf , sizeof(d_colorbuf));

    //--- 将原像素点字节数据进行缩放，变为指定大小bmp图片的像素点字节数据 ---//
    ScalePicture(BMP_crlor_buf , BMP_infoHead.biWidth , BMP_infoHead.biHeight , d_colorbuf , d_w , d_h);

    int colorbuf[d_w * d_h];     //存放要显示的bmp图片的像素点数据

    //--- 清空缓冲区 ---//
    bzero(colorbuf , sizeof(colorbuf));

    //--- 将缩放后的像素点字节数据合成为一个像素点数据 ---//
    for(int y=0 ; y<d_h ; y++)
    {
        for(int x=0 ; x<d_w ; x++)
        {
            colorbuf[y*d_w + x] = d_colorbuf[(d_h - y -1)*d_w*3 + x*3  + 2] << 16 | 
                                  d_colorbuf[(d_h - y -1)*d_w*3 + x*3  + 1] << 8  |
                                  d_colorbuf[(d_h - y -1)*d_w*3 + x*3  + 0] << 0;
        }
    }

    //--- 将像素点数据显示到LCD屏幕的指定坐标处 ---//
    LCD_Draw_Color(colorbuf , start_x , start_y , d_w , d_h , LCD);

    //--- 关闭bmp图片文件 ---//
    close(fd_bmp);

read_color_failed:
    close(fd_bmp); 
}
