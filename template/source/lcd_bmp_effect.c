#include "lcd_bmp_effect.h"
#include "ScalePicture.h"
#include <unistd.h>
#include <errno.h>

//--- 用户类型声明 ---//
typedef     long    LONG ;
typedef     short   WORD;
typedef     int     DWORD;


//--- BMP图片文件信息头结构体 ---//
typedef struct tagBITMAPINFOHEADER{
    DWORD biSize;           //本结构所占用字节数（15-18字节）
    LONG biWidth;           //位图的宽度，以像素为单位（19-22字节）
    LONG biHeight;          //位图的高度，以像素为单位（23-26字节）
    WORD biPlanes;          //目标设备的级别，必须为1(27-28字节）
    WORD biBitCount;        //每个像素所需的位数，必须是1位（双色），4位(16色），8位(256色）16位(高彩色)或24位(真彩色)之一，（29-30字节）
    DWORD biCompression;    //位图压缩类型，必须是0（不压缩），1(BI_RLE8压缩类型）或2(BI_RLE4压缩类型）之一，（31-34字节）

    DWORD biSizeImage;      //位图的大小(其中包含了为了补齐列数是4的倍数而添加的空字节)，以字节为单位（35-38字节）
    LONG biXPelsPerMeter;   //位图水平分辨率，每米像素数（39-42字节）
    LONG biYPelsPerMeter;   //位图垂直分辨率，每米像素数（43-46字节)
    DWORD biClrUsed;        //位图实际使用的颜色表中的颜色数（47-50字节）
    DWORD biClrImportant;   //位图显示过程中重要的颜色数（51-54字节）
}__attribute__((packed)) BITMAPINFOHEADER;

//--- BMP图片文件头结构体 ---//
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType;            //位图文件的类型，必须为BM(1-2字节）
    DWORD bfSize;           //位图文件的大小，以字节为单位（3-6字节，低位在前）
    WORD bfReserved1;       //位图文件保留字，必须为0(7-8字节）
    WORD bfReserved2;       //位图文件保留字，必须为0(9-10字节）
    DWORD bfOffBits;        //位图数据的起始位置，以相对于位图（11-14字节，低位在前）
    //文件头的偏移量表示，以字节为单位
}__attribute__((packed)) BITMAPFILEHEADER;


int lcd_fd;
unsigned int *mem_p; //定义无符号类型指针，用于存放映射地址
unsigned int W=450;
unsigned int H=315;
unsigned int L_W=1000;
unsigned int L_H=600;
unsigned int N_W=600;
unsigned int N_H=420;

//LCD初始化
void LCD_init(void)
{
	//打开屏幕
	lcd_fd = open(FB_PATH,O_RDWR);
	if(lcd_fd == -1)
	{
		printf("open lcd failure!\n");
		return ;
	}
	
	//I/O映射
	mem_p = (unsigned int *)mmap( NULL, 800*480*4 ,    	 	//映射fb内存空间长度 	 
					PROT_READ | PROT_WRITE,	//可读可写
					MAP_SHARED,     		//进程间共享机制
					lcd_fd, 				//lcd的文件描述符
					0);
	
	if(mem_p == MAP_FAILED)
	{
		printf("mmap failure!\n");
		return;
	}
}

//LCD撤消
void LCD_UnInit(void)
{
	close(lcd_fd);
	munmap(mem_p,800*480*4);
}
/***********************************************************************
* 功能：左右偏移图片
* 参数:  pathname 图片名字 (必须是在同工程目录下的图片)
*          start_x 起点X
*			start_y 起点Y
*			left_offset 左移偏移量
*			right_offset 右移偏移量
**********************************************************************/
void show_bmp_Xoffset(const char *pathname,int start_x,int start_y,int left_offset,int right_offset)
{
	int bmp_fd,i,x,y;
	unsigned short heigh,weight;
	unsigned char head_buff[54];//存储头信息的buff
	//打开屏幕
	bmp_fd = open(pathname,O_RDWR);
	if(bmp_fd == -1)
	{
		printf("open bmp failure!\n");
		return ;
	}
	
	read(bmp_fd,head_buff,54);
	//得到图片宽度
	weight = head_buff[19]<<8 | head_buff[18];
	
	//得到图片高度
	heigh = head_buff[23]<<8 | head_buff[22];
	
	if((weight+start_x) > 800)
	{
		printf("bmp weight+start_x big 800\n");
		return ;
	}
	
	if((heigh+start_y) > 480)
	{
		printf("bmp heigh+start_y big 800\n");
		return ;
	}
	
	
	//定义用于存放图片rgb的buff
	unsigned char bmp_buff[weight*heigh*3];
	unsigned int buff[weight*heigh];
	unsigned int T_buff[weight*heigh];
	
	//将图片像素读到buffer当中
	read(bmp_fd,bmp_buff,sizeof(bmp_buff));
	
	//将rgb合成一个像素点unsigned int
	for(i=0; i<weight*heigh; i++)
		buff[i] = bmp_buff[3*i+0] | bmp_buff[3*i+1]<<8 | bmp_buff[3*i+2]<<16;
	
	for(y=0; y<heigh; y++)
		for(x=0; x<weight; x++)
			T_buff[y*weight+x]=buff[(heigh-1-y)*weight+x];
		
	//处理T_buff 剔除 屏幕左边界至某列的图像像素点，再显示出来
	if(left_offset==0 && right_offset == 0 )
	{
		for(y=0;y<heigh;y++)  //图片高度 420  上下间距:30    
		{
				for(x=0;x<weight;x++)  //图片宽度 600  左右间距:100
				{
					*(mem_p + (y+start_y)*800 + x +start_x ) = T_buff[y*weight+x];	
				}
		}
		return;
	}
	//若有左偏移，则是剔除左边界至left_offset列的像素点
	if(left_offset>0 && left_offset<weight)
	{
			for(y=0;y<heigh;y++)
			{
				for(x=left_offset;x<weight;x++)
				{
					//*(mem_p + y*weight+ x - left_offset) = T_buff[y*weight+x];	//原本无偏移的代码
					//加上x,y偏移后的代码
					*(mem_p + (y+start_y)*800+ x - left_offset+start_x) = T_buff[y*weight+x];	
				}
			}
	}
	else if(right_offset>0 && right_offset<weight)//若有右偏移，则是剔除右边界至right_offset列的像素点
	{
			for(y=0;y<heigh;y++)
			{
				for(x=0;x<weight-right_offset;x++)
				{
					//*(mem_p + y*weight+ x + right_offset) = T_buff[y*weight+x];	
					
					//加上x,y偏移后的代码
					*(mem_p + (y+start_y)*800+ x + right_offset +start_x) = T_buff[y*weight+x];	
				}
			}
	}
	
	close(bmp_fd);
	
}

/***********************************************************************
* 功能：上下偏移图片
* 参数:  pathname 图片名字 (必须是在同工程目录下的图片)
*          start_x 起点X
*			start_y 起点Y
*			up_offset 上移偏移量
*			down_offset 下移偏移量
**********************************************************************/
void show_bmp_Yoffset(const char *pathname,int start_x,int start_y,int up_offset,int down_offset)
{
	if(up_offset>0&&down_offset>0)
	{
		printf("up_offset 和 down_offset 不能都有值");
		return;
	}
	int bmp_fd,i,x,y;
	unsigned short heigh,weight;
	unsigned char head_buff[54];//存储头信息的buff
	//打开屏幕
	bmp_fd = open(pathname,O_RDWR);
	if(bmp_fd == -1)
	{
		printf("open bmp failure!\n");
		return ;
	}
	
	read(bmp_fd,head_buff,54);
	//得到图片宽度
	weight = head_buff[19]<<8 | head_buff[18];
	
	//得到图片高度
	heigh = head_buff[23]<<8 | head_buff[22];
	
	if((weight+start_x) > 800)
	{
		printf("bmp weight+start_x big 800\n");
		return ;
	}
	
	if((heigh+start_y) > 480)
	{
		printf("bmp heigh+start_y big 800\n");
		return ;
	}
	
	
	//定义用于存放图片rgb的buff
	unsigned char bmp_buff[weight*heigh*3];
	unsigned int buff[weight*heigh];
	unsigned int T_buff[weight*heigh];
	
	//将图片像素读到buffer当中
	read(bmp_fd,bmp_buff,sizeof(bmp_buff));
	
	//将rgb合成一个像素点unsigned int
	for(i=0; i<weight*heigh; i++)
		buff[i] = bmp_buff[3*i+0] | bmp_buff[3*i+1]<<8 | bmp_buff[3*i+2]<<16;
	
	for(y=0; y<heigh; y++)
		for(x=0; x<weight; x++)
			T_buff[y*weight+x]=buff[(heigh-1-y)*weight+x];
		
	//若有左偏移，则是剔除左边界至left_offset列的像素点
	if(up_offset>0 && up_offset<heigh)
	{
			for(y=up_offset;y<heigh;y++)
			{
				for(x=0;x<weight;x++)
				{					
					//加上x,y偏移后的代码
					*(mem_p + (y-up_offset+start_y)*800+ x +start_x) = T_buff[y*weight+x];	
				}
			}
	}
	else if(down_offset>0 && down_offset<heigh)//若有右偏移，则是剔除右边界至right_offset列的像素点
	{
			for(y=0;y<heigh-down_offset;y++)
			{
				for(x=0;x<weight;x++)
				{
					//*(mem_p + y*weight+ x + right_offset) = T_buff[y*weight+x];	
					
					//加上x,y偏移后的代码
					*(mem_p + (y+start_y+down_offset)*800+ x +start_x) = T_buff[y*weight+x];	
				}
			}
	}
	
	close(bmp_fd);
	
}
/***********************************************************************
* 功能：放大缩小图片
* 参数:  pathname 原图名字 (必须是在同工程目录下的图片)
*          start_x 起点X
*			start_y 起点Y
*			d_w  目标图片宽度
*			d_h  目标图片高度
**********************************************************************/
int show_scalebmp(const char *pathname,int start_x,int start_y,int d_w,int d_h)
{
	int bmp_fd,i,x,y;
	unsigned short heigh,weight;
	unsigned char head_buff[54];//存储头信息的buff
	//打开屏幕
	bmp_fd = open(pathname,O_RDWR);
	if(bmp_fd == -1)
	{
		printf("open bmp failure!\n");
		return -1;
	}
	
	read(bmp_fd,head_buff,54);
	//得到图片宽度
	weight = head_buff[19]<<8 | head_buff[18];
	
	//得到图片高度
	heigh = head_buff[23]<<8 | head_buff[22];
	printf("所读取的图片高度为%d,宽度为%d\n",heigh,weight);

	if((d_w+start_x) > 800)
	{
		printf("bmp d_w+start_x big 800\n");
		return -1;
	}
	
	if((d_h+start_y) > 480)
	{
		printf("bmp d_h+start_y big 480\n");
		return -1;
	}
	
	//定义用于存放图片rgb的buff
	unsigned char bmp_buff[weight*heigh*3];
	
	unsigned int buff[d_w*d_h];

	//将原图图片像素读到buffer当中
	read(bmp_fd,bmp_buff,sizeof(bmp_buff));
	
	unsigned char bmp_scalebuff[d_w*d_h*3];
	printf("进入scalepicture\n");
	ScalePicture(bmp_buff,weight,heigh,bmp_scalebuff,d_w,d_h);
	
	printf("[%s]%d\n",__FUNCTION__,__LINE__);
	//将rgb合成一个像素点unsigned int
	for(i=0; i<d_w*d_h; i++)
		buff[i] = bmp_scalebuff[3*i+0] | bmp_scalebuff[3*i+1]<<8 | bmp_scalebuff[3*i+2]<<16;
	printf("像素点合成完成\n");
	
	//图像倒序及显示
	for(y=0; y<d_h; y++)
	{		
		for(x=0; x<d_w; x++)
		{
			*(mem_p + (y+start_y)*800+ start_x+x) = buff[(d_h-1-y)*d_w+x];
		}
	}
	printf("图像倒序完成\n");
	close(bmp_fd);
	
}
/*
const char *pathname:图片路径
int start_x：图片起始位置 x轴
int start_y：图片起始位置 y轴
*/
void show_bmp(const char *pathname,int start_x,int start_y)
{
	int bmp_fd,i,x,y;
	unsigned short heigh,weight;
	unsigned char head_buff[54];//存储头信息的buff
	//打开屏幕
	bmp_fd = open(pathname,O_RDWR);
	if(bmp_fd == -1)
	{
		printf("open bmp failure!\n");
		return ;
	}
	
	read(bmp_fd,head_buff,54);
	//得到图片宽度
	weight = head_buff[19]<<8 | head_buff[18];
	
	//得到图片高度
	heigh = head_buff[23]<<8 | head_buff[22];
	
	if((weight+start_x) > 800)
	{
		printf("bmp weight+start_x big 800\n");
		return ;
	}
	
	if((heigh+start_y) > 480)
	{
		printf("bmp heigh+start_y big 800\n");
		return ;
	}
	
	
	//定义用于存放图片rgb的buff
	unsigned char bmp_buff[weight*heigh*3];
	unsigned int buff[weight*heigh];

	
	//将图片像素读到buffer当中
	read(bmp_fd,bmp_buff,sizeof(bmp_buff));
	
	//将rgb合成一个像素点unsigned int
	for(i=0; i<weight*heigh; i++)
		buff[i] = bmp_buff[3*i+0] | bmp_buff[3*i+1]<<8 | bmp_buff[3*i+2]<<16;
	
	
	//图像倒序及显示
	for(y=0; y<heigh; y++)
		for(x=0; x<weight; x++)
			*(mem_p + (y+start_y)*800+ start_x+x) = buff[(heigh-1-y)*weight+x];
		
	close(bmp_fd);
	
}
#if 0
void Big_PictureByPoint(const char *pathname,int pointX,int pointY)
{
	if(pointX<=100||pointX>=800-100-W) // 500
		return;
	if(pointY<=450-H)
		return;
	int bmp_fd,i,x,y;
	unsigned short heigh,weight;
	unsigned char head_buff[54];//存储头信息的buff
	//打开屏幕
	bmp_fd = open(pathname,O_RDWR);
	if(bmp_fd == -1)
	{
		printf("open bmp failure!\n");
		return ;
	}
	
	read(bmp_fd,head_buff,54);
	//得到图片宽度
	weight = head_buff[19]<<8 | head_buff[18];
	
	//得到图片高度
	heigh = head_buff[23]<<8 | head_buff[22];
	
	//pointY=480-pointY;
	pointY = 450 - pointY;	
	pointX = pointX-100;
	
	if(pointX>=600-W)
		return;
	if(pointY>=420-H)
		return;

	printf("I am lcd.c X:%d,Y:%d\n",pointX,pointY);
	//定义用于存放图片rgb的buff
	unsigned char bmp_buff[weight*heigh*3];
	unsigned char bmp_small[W*H*3];
	//将图片像素读到buffer当中
	read(bmp_fd,bmp_buff,sizeof(bmp_buff));
	int k,l;
	//截取图片 A[pointX,pointY] B[pointX+W,pointY] C[pointX,pointY+H],D[pointX+W,pointY+H]
	for(k=pointY;k<pointY+H;k++)
	{
		for(l=pointX;l<pointX+W;l++){
			bmp_small[((k-pointY)*W+l-pointX)*3]=bmp_buff[(k*W+l)*3];
			bmp_small[((k-pointY)*W+l-pointX)*3+1]=bmp_buff[(k*W+l)*3+1];
			bmp_small[((k-pointY)*W+l-pointX)*3+2]=bmp_buff[(k*W+l)*3+2];
		}
	}

	show_smallbmp(bmp_small,W,H,100,30,600,420);
			
	close(bmp_fd);
}


void show_smallbmp(unsigned char o_picture[],int o_w,int o_h,int start_x,int start_y,int d_w,int d_h)
{
	int i,x,y;
	if((d_w+start_x) > 800)
	{
		printf("bmp d_w+start_x big 800\n");
		return ;
	}
	
	if((d_h+start_y) > 480)
	{
		printf("bmp d_h+start_y big 480\n");
		return ;
	}
	
	
	unsigned int buff[d_w*d_h];
	
	unsigned char bmp_scalebuff[d_w*d_h*3];
	
	ScalePicture(o_picture,o_w,o_h,bmp_scalebuff,d_w,d_h);
	
	
	//将rgb合成一个像素点unsigned int
	for(i=0; i<d_w*d_h; i++)
		buff[i] = bmp_scalebuff[3*i+0] | bmp_scalebuff[3*i+1]<<8 | bmp_scalebuff[3*i+2]<<16;
	
	
	//图像倒序及显示
	for(y=0; y<d_h; y++)
		for(x=0; x<d_w; x++)
			*(mem_p + (y+start_y)*800+ start_x+x) = buff[(d_h-1-y)*d_w+x];
	
}

#endif

/****************************************************************
*	局部放大图片，根据用户点击的位置，例如：左下角（设定一个区域的），然后放大从这个点开始的右上角（从放大后的图片(1000*600) 取600*420像素点）
*	pathname : 图片名字
*	pointX : 点击位置X
*  pointY ：点击位置Y
****************************************************************/
void ExpandPicture(const char *pathname,int pointX,int pointY)
{
	double r_x,r_y;	
	int bmp_fd,i,x,y;
	unsigned short heigh,weight;
	unsigned char head_buff[54];//存储头信息的buff
	pointY = 450 - pointY;	
	pointX = pointX-100;
	
	if(pointX>600||pointX<0)
		return;
	if(pointY>420||pointY<0)
		return;

	r_x = pointX/(double)600;
	r_y = pointY/(double)420;
	
	printf("r_x:%f,r_y:%f\n",r_x,r_y);
	
	//打开屏幕
	bmp_fd = open(pathname,O_RDWR);
	if(bmp_fd == -1)
	{
		printf("open bmp failure!\n");
		return ;
	}
	
	//read(bmp_fd,head_buff,54);
	//得到图片宽度
	weight = head_buff[19]<<8 | head_buff[18];
	
	//得到图片高度
	heigh = head_buff[23]<<8 | head_buff[22];
	
	
	
	//定义用于存放图片rgb的buff
	unsigned char bmp_buff[600*420*3];
	
	//将图片像素读到buffer当中
	read(bmp_fd,bmp_buff,sizeof(bmp_buff));
	
	//放大图片后的像素数组
	unsigned char bmp_BigBuff[L_W*L_H*3];//bug (1) 1200*840*3 过于大
	
	//放大图片操作
	ScalePicture(bmp_buff,600,420,bmp_BigBuff,L_W,L_H);
	
	//放大后的图片数组
	unsigned int buff[L_W*L_H];
	
	//倒序放大后的图片数组
	unsigned int T_buff[L_W*L_H];
	
	//从放大后图片的数组中截取到的数组
	unsigned int G_buff[N_W*N_H];
	
	//将rgb合成一个像素点unsigned int
	for(i=0; i<L_W*L_H; i++)
		buff[i] = bmp_BigBuff[3*i+0] | bmp_BigBuff[3*i+1]<<8 | bmp_BigBuff[3*i+2]<<16;
	
	//获取放大后的图片数组的倒序数组
	for(y=0; y<L_H; y++)
		for(x=0; x<L_W; x++)
			T_buff[y*L_W+x]=buff[(L_H-1-y)*L_W+x];
		
	//下面，开始针对这个倒序数组进行截取操作，截取600*420,放入G_buff
			
	//一共分4种情况，左上 左下  右上 右下
	//按比例计算出位于放大2倍图片的点击坐标b_x,b_y
	int b_x,b_y;
	b_x=(int)(L_W*r_x);
	b_y=(int)(L_H*r_y);
	printf("b_x:%d, b_y:%d\n",b_x,b_y);
	//点击左上角，从该点往右取600像素点，往下取420像素点得到一个600*420的图片
	if(b_x<L_W-600&&b_y<L_H-420)
	{
		printf("1\n");	
		
		for(y=b_y;y<b_y+N_H;y++)
		{
			for(x=b_x;x<b_x+N_W;x++)
			{
				G_buff[(y-b_y)*N_W+x-b_x]=T_buff[y*L_W+x];
			}
		}
	}
	else if(b_x<L_W-600&&b_y>L_H-180)//点击左下角，取右上
	{
		printf("2\n");
		for(y=b_y-N_H;y<b_y;y++)
		{
			for(x=b_x;x<b_x+N_W;x++)
			{
				G_buff[(y-b_y+N_H)*N_W+x-b_x]=T_buff[y*L_W+x];
			}
		}
	}
	else if(b_x>L_W-400&&b_y<L_H-420)//点击右上角,取左下
	{
		printf("3\n");
		for(y=b_y;y<b_y+N_H;y++)
		{
			for(x=b_x-N_W;x<b_x;x++)
			{
				G_buff[(y-b_y)*N_W+x-b_x+N_W]=T_buff[y*L_W+x];
			}
		}
	}
	else if(b_x>L_W-400&&b_y>L_H-180)//点击右下角，取左上    //L_W1000  L_H600
	{
		printf("4\n");
		for(y=b_y-N_H;y<b_y;y++)
		{
			for(x=b_x-N_W;x<b_x;x++)
			{
				G_buff[(y-b_y+N_H)*N_W+x-b_x+N_W]=T_buff[y*L_W+x];   
			}
		}
	}
	//让它完美的方法，1.扩大L_W,L_H，但是因为放大这个后会报错所以不行。
	//2.往左 取不到600像素点时，要往右取像素点，同理，往右，往上，往下都这样做，就可以点击任意地方 取到600*420 图片了。
	//3.为什么必须要取600*420？因为这个局部放大原理是：将原图（600*420）放大为 （L_W*L_H） 然后从放大后的图片(L_W*L_H)获取 600*420，获取到了就输出到屏幕
		
	
	//显示G_buff到屏幕上
	for(y=0; y<N_H; y++)
		for(x=0; x<N_W; x++)
			*(mem_p + (y+30)*800+ 100+x) = G_buff[y*N_W+x];
		
	//关闭文件流
	close(bmp_fd);
	
}

#if 1
int bmp_display_effect( const char * FileName , int * p_lcd ,  int x_s , int y_s , int mode)
{
    // 打开图像文件
    int fd_bmp =  open( FileName  , O_RDONLY);
    if (-1 == fd_bmp )
    {
        // perror("open bmp error ");
        fprintf( stderr , " open bmp %s error : %s \n" , FileName , strerror(errno));
        return -1 ;
    }


    // 读取图片头部信息
    BITMAPFILEHEADER fileHead ; // 定义文件头的结构体变量
    read( fd_bmp , &fileHead , sizeof(fileHead) ); // 14 

    BITMAPINFOHEADER infoHead ; // 定义信息头的结构体变量
    read( fd_bmp , &infoHead , sizeof(infoHead) ); //40


    printf("图像类型：%hd \n" , fileHead.bfType );
    printf("图像色深：%d \n" , infoHead.biBitCount );
    printf("图像大小：%d \n" , infoHead.biSizeImage );

    int biWidth = infoHead.biWidth ;
    int biHeight = infoHead.biHeight ;

    int half_Width = biWidth/2;
    int half_Height = biHeight/2;

    printf("该图像的宽度：%d 高度：%d\n" , biWidth , biHeight);

    // 计算图像中补齐的字节数
    int num = 0 ;
    if ( (biWidth*3)%4 != 0 ) // 判断余数是否为0 ， 如果不是0 则说明需要补齐
    {
        num = 4 - (biWidth*3)%4 ;  // 通过 4 减去余数的知具体补齐的字节数
    }

    //读取图像 BGR 颜色值
    char buf_bmp [ biHeight*(biWidth*3+num) ] ;
    int ret_val = read( fd_bmp , buf_bmp , sizeof(buf_bmp) );
    printf("成功读取 %d 个字节\n" , ret_val  );

    // 把BGR 转换为 ARGB 
    int buf_lcd [ biHeight ][ biWidth ] ;
    for (int y = 0; y < biHeight ; y++)
    {
        for (int x = 0; x < biWidth ; x++)
        {
            buf_lcd [biHeight - y-1] [x] =      buf_bmp[(x+y*biWidth)*3+0+y*num] << 0 |   // 蓝色 
                                                buf_bmp[(x+y*biWidth)*3+1+y*num] << 8 |   // 绿色
                                                buf_bmp[(x+y*biWidth)*3+2+y*num] << 16 |  // 红色
                                                0x00 << 24 ;        // 透明度
        }
    }

    switch (mode)
    {
    case 1:
    /*先1.3象限后2.4象限*/
        for (int y = 0; y < biHeight/2; y++)
        {
            for(int x = 0; x < biWidth/2; x++)
            {
                if ((x+x_s) < 800 && (y+y_s) < 480)
                {
                    *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x];                                                                                      
                    *(p_lcd+(x+x_s+half_Width)+(y+y_s+half_Height)*800) = buf_lcd[y+half_Height][x+half_Width];     
                } 
            }
            usleep(2000);

        }
        usleep(3000);
        for (int y = 0; y < biHeight/2; y++)
        {
            for(int x = 0; x < biWidth/2; x++)
            {
                if ((x+x_s) < 800 && (y+y_s) < 480)
                {
                    *(p_lcd+(x+x_s+half_Width)+(y+y_s)*800) = buf_lcd[y][x+half_Width];
                    *(p_lcd+(x+x_s)+(y+y_s+half_Height)*800) = buf_lcd[y+half_Height][x];                       
                } 
            }
            usleep(2000);
        }
        break;
    
    case 2:
    /*右滚动*/
        for(int x = 0;x < biWidth;x++)
        {
            for(int y = 0;y < biHeight;y++)
            {
                *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x]; 
            }
            usleep(2000);
        }
        break;

    case 3:
    /*左滚动*/
        printf("\n",__LINE__);
        for(int x = biWidth-1;x > 0;x--)
        {
            for(int y = biHeight-1;y > 0;y--)
            {
                *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x]; 
            }
            usleep(2000);
        }
        break;

    case 4:
    /*上滚动*/
        for(int y = biHeight-1;y > 0;y--)
        {
            for(int x = 0;x < biWidth;x++)
            {
                *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x]; 
            }
            usleep(2000);
        }
        break;

    case 5:
    /*下滚动*/
        for(int y = 0;y < biHeight;y++)
        {
            for(int x = 0;x < biWidth;x++)
            {
                *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x]; 
            }
            usleep(2000);
        }
        break;

    case 6:
    /*先1,2后3,4*/
        for(int x = 0;x < biWidth;x++)
        {
            for(int y = 0; y < biHeight/2; y++)
            {
                if ((x+x_s) < 800 && (y+y_s) < 480)
                {
                    *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x];
                }
            }
            usleep(2000);
        }
        usleep(800);
        for(int x = biWidth-1 ; x > 0;x--)
        {
            for(int y = biHeight/2; y < biHeight; y++)
            {
                if ((x+x_s) < 800 && (y+y_s) < 480)
                {
                     *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x];
                } 
            }
            usleep(2000);
        }
    break;

    case 7:
        for(int y = biHeight-1;y > 0;y--)
        {
            for(int x = 0;x < biWidth/2;x++)
            {
                if ((x+x_s) < 800 && (y+y_s) < 480)
                {
                     *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x];
                }
            }
            usleep(2000);
        }
        usleep(800);
        for(int y = 0;y < biHeight;y++)
        {
            for(int x = biWidth/2;x < biWidth;x++)
            {
                if ((x+x_s) < 800 && (y+y_s) < 480)
                {
                     *(p_lcd+(x+x_s)+(y+y_s)*800) = buf_lcd[y][x];
                }
            }
            usleep(2000);
        }
    default:
        break;

    }

    
    // 资源回收
    close(fd_bmp);
   

    return 0;
}
#endif
