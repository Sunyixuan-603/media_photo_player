#include "includes.h"
#include "jpeglib.h"        
#include "LCD_Device.h"

void bmp_photo_rm_func(const char ** bmp_file,int *file_number,int cur_page,int line,int x_s,int y_s)
{
    memset(buf_path,0,sizeof (buf_path));
    switch (line)
    {
    case 1:
        *file_number = (x_s > 50 && x_s < 350)?cur_page*4+1:*file_number;
        *file_number = (x_s > 400 && x_s < 700)?cur_page*4+2:*file_number;
        break;
    
    case 2:
        *file_number = (x_s > 50 && x_s < 350)?cur_page*4+3:*file_number;
        *file_number = (x_s > 400 && x_s < 700)?cur_page*4+4:*file_number;
        break;
    }
    sprintf(buf_path,"rm %s/%s",PIC_PHOTO_BMP_PATH,bmp_file[*file_number]);
    system(buf_path);
}

void font_photo_handle(const char *string,struct LcdDevice* lcd_font)
{
    //打开字体
    font *f_quit = fontLoad("/usr/share/fonts/fang.ttf");
    //字体大小设置
    fontSetSize(f_quit,30);
    //创建一个画板作为点阵图
    //先设置字体画板的长宽
    bitmap *bm = createBitmap(500,35,4);
    fontPrint(f_quit,bm,100,5,string,getColor(0,100,100,100),0);
    //将字体输出框输出到LCD屏幕上面
    show_font_to_lcd(lcd_font->mp,100,5,bm);
    printf("%s\n",string);
    sleep(2);
    fontUnload(f_quit);
    destroyBitmap(bm);
}

//描点函数(size可设置二维码整体大小)
void draw_qrcode_point(int *lcd,int color,int size,int px,int py)
{
	int x=0,y=0;
	for(y=0;y<size;y++)
	    for(x=0;x<size;x++)
		    *(lcd+(y+py*size)*800+x+px*size) = color;	//颜色值控制
}
//二维码显示函数
void qrcode_display(const char *string,int start_x,int start_y)
{
    QRcode *qrcode=NULL;
	//生成二维码数据
  	qrcode = QRcode_encodeString(string, 2, QR_ECLEVEL_L, QR_MODE_8,0);

	//printf date 
	printf("qrcode->version=%d\n",qrcode->version);
	printf("qrcode->width=%d\n",qrcode->width);

	//打开LCD设备
	int  fd = open("/dev/fb0",O_RDWR);	
	if(fd < 0)
		printf("[%s]open lcd fail\n",__FUNCTION__);
	
	//对LCD设备进行映射操作
	int *lcd = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	
	int i=0,j=0;
	for(i=0;i<25;i++)
	{
		for(j=0;j<25;j++)    //i*25  跳行
		{					 //&0x01  //判断是0还是1
			if(qrcode->data[i*25+j]&0x01)
			{
				printf("#");
				draw_qrcode_point(lcd,0x000000,8,j+start_y,i+start_x); //描点
			}
			else
			{
				printf("_");
				draw_qrcode_point(lcd,0xffffff,8,j+start_y,i+start_x);
			}
		}
		printf("\n");
	}
	printf("\n");
    close(fd);
}


void picture_chose_func(const char *path , const char *filename,int * count , int ch)
{
    memset(buf_path , 0 , sizeof (buf_path));
    if(ch)
    {
        sprintf(buf_path , "%s/%s" , path , filename);
        lcd_draw_jpg(0,0,buf_path);printf("buf_path : %s\n",buf_path);
    }else{
        sprintf(buf_path , "%s/%s%d.jpg" , path , filename , *count);
        lcd_draw_jpg(0,0,buf_path);printf("buf_path : %s\n",buf_path);
    }
}

void unlock_display_func(const char * filename, int * count, int ch)
{
    memset(buf_path , 0 , sizeof (buf_path));
    if(ch)
    {
        sprintf(buf_path , "%s/%s" , PIC_UNLOCK_PATH , filename);
        lcd_draw_jpg(0,0,buf_path);printf("buf_path : %s\n",buf_path);
    }else{
        sprintf(buf_path , "%s/%s%d.jpg" , PIC_UNLOCK_PATH , filename , *count);
        lcd_draw_jpg(0,0,buf_path);printf("buf_path : %s\n",buf_path);
    }
}

void music_shell_display(int count,int *music_count,char *music_buf,char **music_file,bitmap *bm_play,font *f_file)
{
    *music_count = count;
    sprintf(music_buf , MPLAY_MUSIC_SHELL , music_file[count]);
    printf("播放的音乐名 : %s\n",music_file[count]);
    system(music_buf);printf("音乐播放中！！！\n");
    fontPrint(f_file,bm_play,30,10,music_file[count],getColor(0,100,100,100),0);
}

int music_display_func(void)
{
    static int x_s = 0, y_s = 0;
    char music_buf [64] = {0};  //用作mplayer的shell路径
    bool play_sta = true;
    memset(buf_path , 0 , sizeof buf_path);
    sprintf(buf_path , "%s/%s.jpg", PIC_PATH , "music");    
    lcd_draw_jpg(0,0,buf_path);
    
    P_Node  music_list =  new_Node( NULL , 'U' );

    // 检索目录 --> 添加节点到链表中
    int num = find_dir("/template/music" , ".mp3" , music_list);

    // 遍历显示链表
    int count_member = 0;
    list_4_eath(music_list , &count_member);

    //遍历取出每个mp3
    P_Node tmp = music_list->next;
    char * music_file  [64] = {0};
    int count = 0;
    for(count = 0 ; tmp != music_list ; count++ )
    {
        music_file[count] = strrchr(tmp->Path_Name,'/')+1;
        //strcpy(music_file[count],strrchr(tmp->Path_Name,'/')+1);
        printf("music_file[%d]:%s\n",count,music_file[count]);
        tmp = tmp->next;
    }

    printf("显示检索的mp3文件\n");

    //初始化Lcd
	struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");

    //打开字体
    font *f_title = fontLoad("/usr/share/fonts/fang.ttf");
    font *f_file =  fontLoad("/usr/share/fonts/fang.ttf");
    //字体大小的设置
    fontSetSize(f_title,28);
    fontSetSize(f_file, 38);
    
    //创建一个画板（点阵图）
	bitmap *bm = createBitmap(310 , 340 , 4);
    
    //将字体写到点阵图上
	fontPrint(f_title,bm,0,35,"当前目录:/template/music/",getColor(0,100,100,100),0);

    int line = 90;
    for(int i = 0;i < count;i++ , line+=50)
        fontPrint(f_file,bm,60,line,music_file[i],getColor(0,100,100,100),0);

    //把字体框输出到LCD屏幕上
	show_font_to_lcd(lcd_font->mp,15,70,bm);

    while (1)
    {    
        int music_count = 0;
        lcd_tch_event(&x_s , &y_s);
        //创建一个画板（点阵图）
        bitmap *bm_stat = createBitmapWithInit(368 , 40 , 4 , getColor(0,255,255,255));
	    bitmap *bm_play = createBitmapWithInit(368 , 68 , 4 , getColor(0,255,255,255));
        //检测是否退出
        if((x_s > 75 && x_s < 260)&&(y_s > 5 && y_s < 70))
        {
            printf("退出音乐播放器!\n");
            //关闭字体，关闭画板
            fontUnload(f_title);
            fontUnload(f_file);
            destroyBitmap(bm);
            destroy_list(music_list);
            system("killall -SIGKILL mplayer");  // 关闭播放器 强制杀死

            return -1;
        }    

        //切换至下一首
        if((x_s > 575 && x_s < 625) && (y_s > 170 && y_s < 260))
        {
            if(music_count > 4)
                music_count = 0;

            music_count+=1;
            system("killall -SIGKILL mplayer");  // 关闭播放器 强制杀死
            usleep(10000);
            memset(music_buf , 0 , sizeof (music_buf));
            sprintf(music_buf , MPLAY_MUSIC_SHELL , music_file[music_count]);
            printf("播放的音乐名 : %s\n",music_file[music_count]);
            system(music_buf);printf("音乐播放中！！！\n");
            fontPrint(f_file,bm_play,30,10,music_file[music_count],getColor(0,100,100,100),0);
            show_font_to_lcd(lcd_font->mp,370,365,bm_play);
        }

        //切换至上一首
        if((x_s > 450 && x_s < 505) && (y_s > 170 && y_s < 255))
        {
            if(music_count == 0)
                music_count = 4;
            
            music_count-=1;
            system("killall -SIGKILL mplayer");  // 关闭播放器 强制杀死
            usleep(10000);
            memset(music_buf , 0 , sizeof (music_buf));
            sprintf(music_buf , MPLAY_MUSIC_SHELL , music_file[music_count]);
            printf("播放的音乐名 : %s\n",music_file[music_count]);
            system(music_buf);printf("音乐播放中！！！\n");
            fontPrint(f_file,bm_play,30,10,music_file[music_count],getColor(0,100,100,100),0);
            show_font_to_lcd(lcd_font->mp,370,365,bm_play);
        }

        if(x_s > 55 && x_s < 160)
        {
            //音乐播放
            memset(music_buf , 0 , sizeof (music_buf));
            system("killall -SIGKILL mplayer");  // 关闭播放器 强制杀死
            
            if(y_s > 165 && y_s < 220)
                music_shell_display(0,&music_count,music_buf,music_file,bm_play,f_file);
            if(y_s > 220 && y_s < 275)
                music_shell_display(1,&music_count,music_buf,music_file,bm_play,f_file);
            if(y_s > 275 && y_s < 320)
                music_shell_display(2,&music_count,music_buf,music_file,bm_play,f_file);
            if(y_s > 320 && y_s < 375)
                music_shell_display(3,&music_count,music_buf,music_file,bm_play,f_file);
            if(y_s > 385 && y_s < 440)
                music_shell_display(4,&music_count,music_buf,music_file,bm_play,f_file);

            show_font_to_lcd(lcd_font->mp,370,365,bm_play);
            
        }
        //赞停与继续播放
        if((x_s > 520 && x_s < 570) && (y_s > 180 && y_s < 265)) 
        {
            if(play_sta)
            {
                printf("暂停播放!\n");
                fontPrint(f_file,bm_stat,80,10,"暂停中",getColor(0,100,100,100),0);
                system("killall -SIGSTOP mplayer");//  暂停运行播放器
                play_sta = false;
            }else{
                printf("继续播放!\n");
                system("killall -SIGCONT mplayer");//  继续运行播放器
                fontPrint(f_file,bm_stat,80,10,"继续播放",getColor(0,100,100,100),0);
                sleep(2);
                play_sta = true;
            }
            show_font_to_lcd(lcd_font->mp,410,20,bm_stat);
        }
    }

}

int video_display_func(void)
{
    int x_s = 0;int y_s = 0;
    char video_buf [64] = {0};  //用作mplayer的shell路径

video_init:   
    memset(buf_path , 0 , sizeof buf_path);
    sprintf(buf_path , "%s/%s.jpg", PIC_PATH , "video_menu");    
    lcd_draw_jpg(0,0,buf_path);
    
    P_Node  video_list =  new_Node( NULL , 'U' );

    // 检索目录 --> 添加节点到链表中
    int num = find_dir("/template/video" , ".mp4" , video_list);

    // 遍历显示链表
    int count_member = 0;
    list_4_eath(video_list , &count_member);

    //遍历取出每个mp3
    P_Node tmp = video_list->next;
    char * video_file  [64] = {0};
    int count = 0;
    printf("----显示检索的mp4文件----\n");
    for(count = 0 ; tmp != video_list ; count++ )
    {
        video_file[count] = strrchr(tmp->Path_Name,'/')+1;
        printf("video_file[%d]:%s\n",count,video_file[count]);
        tmp = tmp->next;
    }

    //初始化Lcd
	struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");

    //打开字体
    font *f_title = fontLoad("/usr/share/fonts/fang.ttf");
    font *f_list  = fontLoad("/usr/share/fonts/fang.ttf");
    font *f_play  = fontLoad("/usr/share/fonts/fang.ttf");

    //字体大小的设置
    fontSetSize(f_title,32);
    fontSetSize(f_list,30);
    fontSetSize(f_play,32);

    //创建一个画板（点阵图）
	bitmap *bm = createBitmap(350 , 340 , 4);

    //将字体写到点阵图上
	fontPrint(f_title,bm,0,35,"当前目录:/template/video/",getColor(0,100,100,100),0);

    int line = 90;
    for(int i = 0;i < count;i++ , line+=50)
        fontPrint(f_list,bm,60,line,video_file[i],getColor(0,100,100,100),0);
        
    //把字体框输出到LCD屏幕上
	show_font_to_lcd(lcd_font->mp,15,70,bm);

    while (1)
    {   
        static int exit = 0;
        static char video_path[64] = {0}; 
        P_Node del_node = NULL;
        int video_count = 0;
        lcd_tch_event(&x_s , &y_s);

        //创建一个画板（点阵图）
	    bitmap *bm_play = createBitmapWithInit(368 , 68 , 4 , getColor(0,255,255,255));
        //检测是否退出
        if((x_s > 75 && x_s < 260)&&(y_s > 5 && y_s < 70))
        {
            printf("退出视频播放器!\n");
            //关闭字体，关闭画板
            fontUnload(f_title);
            fontUnload(f_list);
            fontUnload(f_play);

            destroyBitmap(bm);
            destroy_list(video_list);
            system("killall -SIGKILL mplayer");  // 关闭播放器 强制杀死
            return -1;
        }    

        
        if(x_s > 55 && x_s < 160)
        {
            if(y_s > 165 && y_s < 220)
            {
                video_count = 0;
                fontPrint(f_list,bm_play,30,10,video_file[0],getColor(0,100,100,100),0);
            }
            if(y_s > 220 && y_s < 255)
            {
                video_count = 1;
                fontPrint(f_list,bm_play,30,10,video_file[1],getColor(0,100,100,100),0);
            }
            if(y_s > 255 && y_s < 290)
            {
                video_count = 2;
                fontPrint(f_list,bm_play,30,10,video_file[2],getColor(0,100,100,100),0);
            }
            if(y_s > 290 && y_s < 325)
            {
                video_count = 3;
                fontPrint(f_list,bm_play,30,10,video_file[3],getColor(0,100,100,100),0);
            }
            if(y_s > 360 && y_s < 395)
            {
                video_count = 4;
                fontPrint(f_list,bm_play,30,10,video_file[4],getColor(0,100,100,100),0);
            }
            show_font_to_lcd(lcd_font->mp,370,100,bm_play);
            
        }

        if(y_s > 300 && y_s < 370)
        {
            //进入播放
            if(x_s > 505 && x_s < 625)
            {
                memset(video_buf , 0 , sizeof(video_buf));
                sprintf(video_buf , MPLAY_VIDEO_SHELL , video_file[video_count]);
                system(video_buf);printf("视频播放中\n");
                while(1)
                {
                    static int x_p = 0;
                    static int y_p = 0;
                    static bool play_sta = true;
                    int direction = lcd_tch_slip_event(&x_p , &y_p);
                    if(direction == LEFT)
                    {
                        printf("退出视频播放器!\n");
                        system("killall -SIGKILL mplayer");  // 关闭播放器 强制杀死
                        exit = 1;break;
                    }else if(direction == PRESS)
                    {
                        if(play_sta)
                        {
                            printf("视频暂停中\n");
                            system("killall -SIGSTOP mplayer");//  暂停运行播放器
                            play_sta = false;
                        }else{
                            printf("恢复播放\n");
                            system("killall -SIGCONT mplayer");//  继续运行播放器
                        }
                        continue;
                    }
                }
            }

            if(exit == 1)
                goto video_init;

            //删除视频
            if(x_s > 635 && x_s < 745) 
            {
                memset(video_path , 0 , sizeof(video_path));
                sprintf(video_path , "rm /template/video/%s -r" , video_file[video_count]);
                system(video_path);
                fprintf(stdout , "%s%s", "已删除" , video_file[video_count]);
            }
        }
    }
}

int photo_display_func(void)
{
    static int x_s  = 0;static int y_s  = 0;
    static int x_rt = 0;static int y_rt = 0;   
    bool play_sta = true;
photo_init:
    play_sta = true;
    P_Node bmp_photo_list  = new_Node (NULL , 'U');
    P_Node jpeg_photo_list = new_Node (NULL , 'U');

    picture_chose_func(PIC_PHOTO_PATH,"pic_open_chose.jpg",NULL,1);
    system(MPLAY_PHOTO_BGM);

    while (1)
    {
        int ret = lcd_tch_slip_event(&x_s , &y_s);
        printf("[x] : %d , [y] : %d\n", x_s , y_s);
        //退出音乐播放器
        if(ret == LEFT)
        {
            system("killall -SIGKILL mplayer");
            return -1;
        }
            
        if(ret == PRESS)
        {
            //背景音乐处理
            if(x_s > 15 && x_s < 170 &&  y_s > 45 && y_s < 155)
            {
                if(play_sta = true)
                {system("killall -SIGKILL mplayer");printf("音乐关闭\n");}
                else 
                {
                    system(MPLAY_PHOTO_BGM);play_sta = false;
                    printf("音乐播放\n");
                }
            }
            
            if(x_s > 240 && x_s < 325)
            {
                //jpeg
                if(y_s > 150 && y_s < 220)
                {
                    picture_chose_func(PIC_PHOTO_PATH,"jpeg_background.jpg",NULL,1);
                    
                    memset(buf_path , 0 ,sizeof(buf_path));
                    sprintf(buf_path , "%s/%s" , PIC_PHOTO_LIST_PATH , "4.jpg");
                    // jpeg_control_display(50,50,200,120,buf_path);
                    show_video_data(50,50,buf_path,file_size_get(buf_path));

                    while(lcd_tch_slip_event(&x_rt,&y_rt) != LEFT);
                    system("killall -SIGKILL mplayer");
                    printf("返回上一级菜单\n");goto photo_init;
                }

                //bmp
                if(y_s > 270 && y_s < 345)
                {
                    picture_chose_func(PIC_PHOTO_PATH,"jpeg_background.jpg",NULL,1);
                    char bmp_buf[128] = {0};

                    memset(buf_path , 0 , sizeof buf_path);
                    int num = find_dir("/template/picture/photo_list/bmp" , ".bmp" , bmp_photo_list);
                    
                    //遍历显示链表
                    int count_member = 0;
                    list_4_eath(bmp_photo_list,&count_member);

                    P_Node tmp = bmp_photo_list->next;
                    char * bmp_file[64] = {0};int count = 1;
                    printf("----显示检索到的bmp文件----\n");
                    for(;tmp != bmp_photo_list;count++)
                    {
                        bmp_file[count] = strrchr(tmp->Path_Name,'/')+1;
                        printf("bmp_file[%d]:%s\n",count,tmp->Path_Name);
                        tmp = tmp->next;
                    }count--;
                    
                    //初始化Lcd
	                struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");                    
                    
                    //统计页数
                    int page;
                    if(count <= 4)
                        page = 1;
                    else if(!(count%4))
                        page = count/4;
                    else 
                        page = count/4+1;

                    printf("[将要显示的图片页数]page:%d\n",page);

                    int cur_page = 0;
                    LCD_open("/dev/fb0",O_RDWR,&Mylcd);
                    
                    while(1)
                    {
                    bmp_photo_init:
                        picture_chose_func(PIC_PHOTO_PATH,"jpeg_background.jpg",NULL,1);
                        int x_offset = 0;int y_offset = 0;bool display_flag = false;
                        lcd_close();
                        LCD_open("/dev/fb0",O_RDWR,&Mylcd);
                        for(int b = 0;b < 4;b++)
                        {
                            switch (b)
                            {                            
                            case 1:
                                x_offset = 350;
                                break;
                            case 2:
                                x_offset = 0;
                                y_offset = 215;
                                break;
                            case 3:
                                x_offset = 350;
                                y_offset = 215;
                                break;
                            }
                            memset(buf_path,0,sizeof (buf_path));
                            sprintf(buf_path,"%s/%s",PIC_PHOTO_BMP_PATH,bmp_file[cur_page*4+b+1]);
                            Dis_ScaleBMP(buf_path,50+x_offset,60+y_offset,300,200,Mylcd);
                        }
                        
                        int ret = lcd_tch_slip_event(&x_s,&y_s);
                        
                        //初始化字体Lcd
	                    struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");
                        //打开字体
                        font *f_page = fontLoad("/usr/share/fonts/fang.ttf");
                        //字体大小设置
                        fontSetSize(f_page,30);
                        //创建一个画板作为点阵图
                        //先设置字体画板的长宽
                        bitmap *bm = createBitmap(500,70,4);
                        static int file_number = 0;
                        printf("当前页数为:%d\n",cur_page+1);
                        switch (ret)
                        { 
                        case PRESS:
                            printf("[tch_photo]检测到触摸事件\n");
                            if(y_s > 60 && y_s < 260)
                            {
                                file_number = (x_s > 50 && x_s < 350)?cur_page*4+1:file_number;
                                file_number = (x_s > 400 && x_s < 700)?cur_page*4+2:file_number;
                                
                                printf("当前获取的图片->bmp_file[%d]\n",file_number);
                                display_flag = true;
                            }
                            else if(y_s > 280 && y_s < 480)
                            {
                                file_number = (x_s > 50 && x_s < 350)?cur_page*4+3:file_number;
                                file_number = (x_s > 400&& x_s < 700)?cur_page*4+4:file_number;
                                
                                printf("当前获取的图片->bmp_file[%d]\n",file_number);
                                display_flag = true;
                            }
                            else if(x_s > 90 && x_s < 180 && y_s > 10 && y_s < 25)
                            {
                                printf("返回上一级菜单\n");
                                system("killall -SIGKILL mplayer");
                                goto photo_init;
                            }
                            else if(x_s > 720 && x_s < 800 && y_s > 0 && y_s < 30)
                            {
                                printf("进入图片删除模式");
                                //打开字体
                                font *f_rm = fontLoad("/usr/share/fonts/fang.ttf");
                                //字体大小设置
                                fontSetSize(f_rm,30);
                                //创建一个画板作为点阵图
                                //先设置字体画板的长宽
                                bitmap *bm = createBitmap(500,20,4);
                                fontPrint(f_rm,bm,100,5,"请点击你要删除的图片",getColor(0,100,100,100),0);
                                //将字体输出框输出到LCD屏幕上面
                                show_font_to_lcd(lcd_font->mp,100,5,bm);
                                while(1)
                                {
                                    static int x_rm = 0;static int y_rm = 0;
                                    int ret = lcd_tch_slip_event(&x_rm,&y_rm);
                                    bool tch_sta = false;
                                    
                                    if(y_rm > 60 && y_rm < 260)
                                    {bmp_photo_rm_func(bmp_file,&file_number,cur_page,1,x_rm,y_rm);tch_sta = true;}
                                    else if(y_rm > 280 && y_rm < 480)
                                    {bmp_photo_rm_func(bmp_file,&file_number,cur_page,2,x_rm,y_rm);tch_sta = true;}
                                    
                                    if(x_rm > 720 && x_rm < 800 && y_rm > 0 && y_rm < 15)
                                    {
                                        fontUnload(f_rm);
                                        destroyBitmap(bm);
                                        if(!tch_sta)
                                        {
                                            struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");
                                            font_photo_handle("即将退出删除模式",lcd_font);
                                            break;
                                        }else{
                                            char *string = NULL;
                                            string = (char *)calloc(128,sizeof (char));
                                            sprintf(string,"%s %s","你所删除的是",bmp_file[file_number]);
                                            struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");
                                            font_photo_handle((const char *)string,lcd_font);                                            
                                            font_photo_handle("即将退出删除模式",lcd_font);
                                            printf("删除成功\n");
                                            goto bmp_photo_init;
                                        }
                                    }
                                }

                            }
                            printf("坐标(%d,%d)\n",x_s,y_s);
                            break;
                        
                        case LEFT:
                            if(cur_page+1 == page)
                            {
                                //初始化字体Lcd
	                            struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");
                                //建立字体画板
                                //将字体写到点阵图上
                                fontPrint(f_page,bm,100,10,"当前已是最后一页^_^",getColor(0,100,100,100),0);
                                //将字体框输出到LCD屏幕上面
                                show_font_to_lcd(lcd_font->mp,150,70,bm);
                                sleep(3);
                                //删除字体画板
                                fontUnload(f_page);
                                destroyBitmap(bm);
                            }else{
                                cur_page++;
                            }
                            break;
                        case RIGHT:
                            if(cur_page == 0)
                            {
                                //初始化字体Lcd
	                            struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");
                                //建立字体画板
                                //将字体写到点阵图上
                                fontPrint(f_page,bm,100,10,"当前已经是第一页^_^",getColor(0,100,100,100),0);
                                //将字体框输出到LCD屏幕上面
                                show_font_to_lcd(lcd_font->mp,150,70,bm);
                                usleep(3);
                                //删除字体画板
                                fontUnload(f_page);
                                destroyBitmap(bm);
                            }else{
                                cur_page--;
                            }
                            break;
                        }


                        /*进入相册后的判断*/
                        if(display_flag)
                        {
                            bool exit_photo = false;
                            display_flag = false;

                            LCD_close(&Mylcd);
                            LCD_init();
                            
                            memset(buf_path,0,sizeof (buf_path));
                            sprintf(buf_path,"%s/%s",PIC_PHOTO_BMP_PATH,bmp_file[file_number]);
                            show_bmp(buf_path,0,0);

                            //初始化Lcd
	                        struct LcdDevice* lcd_font = lcd_font_init("/dev/fb0");                    
                            //打开字体
                            font *f_page = fontLoad("/usr/share/fonts/fang.ttf");
                            //字体大小设置
                            fontSetSize(f_page,50);
                            //创建一个画板作为点阵图
                            //先设置字体画板的长宽
                            bitmap *bm = createBitmap(500,70,4);
                            printf("[count]:%d\n",count);
                            while (1)
                            {
                                static int x_s = 0;
                                static int y_s = 0;

                                printf("当前为第%d张图片\n",file_number);
                                bmp_display_effect(buf_path,mem_p,0,0,rand()%7);
                                memset(buf_path,0,sizeof(buf_path));
                                
                                int ret = lcd_tch_slip_event(&x_s,&y_s);
                                switch (ret)
                                {
                                case UP:
                                    exit_photo = true;
                                    LCD_UnInit();
                                    lcd_open();
                                    break;
                                
                                case RIGHT:
                                    if(file_number == 1)
                                    {
                                        file_number = 1;
                                        fontPrint(f_page,bm,100,10,"当前已是第一页>_<",getColor(0,100,100,100),0);
                                        //将字体框输出到LCD屏幕上面
                                        show_font_to_lcd(lcd_font->mp,150,70,bm);
                                        sleep(3);
                                        //删除字体面板
                                        fontUnload(f_page);destroyBitmap(bm);
                                    }else{
                                        file_number--;
                                    }
                                    
                                    sprintf(buf_path,"%s/%s",PIC_PHOTO_BMP_PATH,bmp_file[file_number]);
                                    continue;
                                    break;
                                
                                case LEFT:
                                    if(file_number == count)
                                    {    
                                        file_number = count;
                                        fontPrint(f_page,bm,100,10,"当前已是最后一页>_<",getColor(0,100,100,100),0);
                                        //将字体框输出到LCD屏幕上面
                                        show_font_to_lcd(lcd_font->mp,150,70,bm);
                                        sleep(3);
                                        //删除字体面板
                                        fontUnload(f_page);destroyBitmap(bm);
                                    }else{
                                        file_number++;
                                    }

                                    sprintf(buf_path,"%s/%s",PIC_PHOTO_BMP_PATH,bmp_file[file_number]);
                                    continue;
                                    break;
                                }

                                if(exit_photo)
                                    goto bmp_photo_init;
                            }       
                        }
                    }       
                }
            }   
        }
    }
}

struct LcdDevice *lcd_font_init(const char *device )
{
	//申请空间
	struct LcdDevice* lcd_font = malloc(sizeof(struct LcdDevice));
	if(lcd_font == NULL)
	{
		return NULL;
	} 

	//1打开设备
	lcd_font->fd = open(device, O_RDWR);
	if(lcd_font->fd < 0)
	{
		perror("open lcd fail");
		free(lcd_font);
		return NULL;
	}
	
	//映射
	lcd_font->mp = mmap(NULL,800*480*4,
                        PROT_READ|PROT_WRITE,MAP_SHARED,
                        lcd_font->fd,0);

	return lcd_font;
}

/*刮刮乐相关函数*/
#if 1
//显示正常jpg图片                                           
int scrape_draw_jpg(unsigned int x,unsigned int y,const char *pjpg_path)  
{
	/*定义解码对象，错误处理对象*/
	struct 	jpeg_decompress_struct 	cinfo;
	struct 	jpeg_error_mgr 			jerr;	
	
	unsigned char 	*pcolor_buf = g_color_buf;
	char 	*pjpg;
	
	unsigned int 	i=0;
	unsigned int	color =0;
	unsigned int	count =0;
	
	unsigned int 	x_s = x;
	unsigned int 	x_e ;	
	unsigned int 	y_e ;
	unsigned int	y_n	= y;
	unsigned int	x_n	= x;
	
			 int	jpg_fd;
	unsigned int 	jpg_size;


	if(pjpg_path!=NULL)
	{
		/* 申请jpg资源，权限可读可写 */	
		jpg_fd=open(pjpg_path,O_RDWR);
		
		if(jpg_fd == -1)
		{
		   printf("open %s error ,errno >> %s\n",pjpg_path , strerror(errno));
		   
		   return -1;	
		}	
		
		/* 获取jpg文件的大小 */
		jpg_size=file_size_get(pjpg_path);	
		if(jpg_size<3000)
			return -1;
		
		/* 为jpg文件申请内存空间 */	
		pjpg = malloc(jpg_size);

		/* 读取jpg文件所有内容到内存 */		
		read(jpg_fd,pjpg,jpg_size);
	}
	else
	{
		printf("jpeg path error : %s \n", pjpg_path );
		return -1;
	}

	/*注册出错处理*/
	cinfo.err = jpeg_std_error(&jerr);

	/*创建解码*/
	jpeg_create_decompress(&cinfo);

	/*直接解码内存数据*/		
	jpeg_mem_src(&cinfo,pjpg,jpg_size);
	
	/*读文件头*/
	jpeg_read_header(&cinfo, TRUE);

	/*开始解码*/
	jpeg_start_decompress(&cinfo);	
    

	/*读解码数据*/
	while(cinfo.output_scanline < cinfo.output_height )     //216次循环
	{		
		pcolor_buf = g_color_buf;
		/* 读取jpg一行的rgb值 */
		jpeg_read_scanlines(&cinfo,&pcolor_buf,1);
    
        //未到区域换行
        if(cinfo.output_scanline+248 < y-40)
        {
            for (i = 0; i < cinfo.output_width; i++)
            {
                printf("[1]:%d\n",__LINE__);
                //pcolor_buf +=3;    
                count++;
            }
            continue;
        }
        
        printf("[count_scan_line]%d\n",count);
        count = 0;
        if(cinfo.output_scanline+248 >= y-40 && cinfo.output_scanline+248 <= y+40)
        {
            for(i = x-40;i < x+40;i++)
            {
                /* 获取rgb值 */
                // color = 		*(pcolor_buf+2);
                // color = color | *(pcolor_buf+1)<<8;
                // color = color | *(pcolor_buf)<<16;	
                // lcd_draw_point(i,cinfo.output_scanline,color);
                printf("[2]:%d\n",__LINE__);
                lcd_draw_point(i,248+cinfo.output_scanline,0xffffff);
                //pcolor_buf +=3;
                count++;
            }
            
        }else{
            printf("[count_draw_point]%d\n",count);
            count = 0;
            break;
        }
    		
	}		

	/*解码完成*/
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	if(pjpg_path!=NULL)
	{
		/* 关闭jpg文件 */
		close(jpg_fd);	
		
		/* 释放jpg文件内存空间 */
		free(pjpg);		
	}
	return 0;
}

int scrape_display_func(void)
{
    static int rand   = 0;
    static unsigned int x_s = 0; static int unsigned y_s = 0;
    char scrape_path_buf[64] = {0};
restart_scrape:
    rand = random()%5;
    memset(scrape_path_buf , 0 , sizeof (scrape_path_buf));
    sprintf(scrape_path_buf , "%s/%s" , PIC_SCRAPE_PATH , "scrape_start.jpg");
    lcd_draw_jpg(0,0,scrape_path_buf);

    while (1)
    {
        static int x_slip = 0;static int y_slip = 0;
        lcd_tch_event(&x_s , &y_s);
        x_slip = x_s;y_slip = y_s;
        //退出刮刮乐
        if((x_s > 55 && x_s < 195) && (y_s > 15 && y_s < 95))
        {
            memset(scrape_path_buf , 0 , sizeof(scrape_path_buf));
            return -1;
        }

        //重新擦除
        if((x_s > 575 && x_s < 785) && (y_s > 10 && y_s < 85))
        {
            memset(scrape_path_buf , 0 ,sizeof(scrape_path_buf));
            printf("再来一次\n");
            goto restart_scrape; 
        }

        //刮刮乐
        if((x_s > 67 && x_s < 736) && (y_s > 248 && y_s < 359))
        {
            printf("检测到刮刮乐事件\n");
            memset(scrape_path_buf , 0 ,sizeof(scrape_path_buf));
            sprintf(scrape_path_buf,"%s/%s%d.jpg" , PIC_SCRAPE_PATH , "scrape_" , rand);
            scrape_draw_jpg(x_slip,y_slip,scrape_path_buf);
        }else{
            continue;
        }
    }
}
#endif

bool udisk_list_func(P_Node head , int * num ,const char * path)
{
    if(head == NULL || head->next == head)
        return false;

    //定义一个用来遍历目录下文件的前驱节点
    P_Node tmp = head->prev ;

    for( *num = 0; tmp != head ; tmp = tmp->prev , *num++)
    {
        printf("FileName:%s \t Type:%c \n" , tmp->Path_Name , tmp->Type);
        if(dir_cpy_func(tmp->Path_Name , path) == -1)
            return false;
    }
        
    return true;
}

int dir_cpy_func(const char *src_file ,const char *tar_file )
{
    FILE *fp_s = NULL;
    FILE *fp_t = NULL;

    //以只读方式打开源文件
    if((fp_s = fopen(src_file , "r")) == NULL)
    {
        fprintf(stderr , "打开 %s 失败 , errno >> %s\n" , 
                        src_file , strerror(errno));
        return -1;
    }

    //打开目标文件  ==> 不存在则创建 , 存在则清空打开
    if((fp_t = fopen(tar_file , "w+")) == NULL)
    {
        fprintf(stderr , "打开 %s 失败 , errno >> %s\n" , 
                        tar_file , strerror(errno));
        return -1;
    }

    //循环读取源文件数据
    char buf[50];
    int ret;
    long ft1 , ft2;

    while (1)
    {
        memset(buf , 0 , sizeof (buf));
        ft1 = ftell(fp_s);              //记录每次读取之前文件指针相对于文件开头的偏移量
        ret = fread(buf,10,5,fp_s);//每次从源文件中读取50个字节 --> 5块数据，每块数据10字节
        ft2 = ftell(fp_s);

        if(ft2 - ft1 < 5)   //此时是最后一次读取的大数据量  --> ft2 - ft1
        {
            fwrite(buf,ft2-ft1,1,fp_t);
            break;
        }fwrite(buf,10,5,fp_t);
    }
    
    fclose(fp_s); fclose(fp_t);
}

void login_menu_interface(void)
{
    int x_s , y_s ;
    int count_jpg = 0;
    int count_char = 0;
    char buf_path[64] = {0};
    char buf_passwd[6] = {0}; 
    int count_failed = 0;
    
    lcd_fd = lcd_open();

    system("killall -SIGKILL mplayer");
	memset(qrcode_passwd,0,sizeof (qrcode_passwd));
    
    time_t t;
    srand((unsigned)time(&t));
    /*随机密码生成*/
    for(int count_random = 0;count_random < 6;count_random++)
        qrcode_passwd[count_random] = rand()%10+'0';
    
passwd_failed_handle:
    count_jpg = 0; count_char = 0;
    memset(buf_passwd , 0 ,sizeof (buf_passwd));
    unlock_display_func("unlock_pic1.jpg", &count_jpg , 1);
    while(1)
    {
        lcd_tch_event(&x_s , &y_s);
        //判断是否为验证密码
        if((x_s > 630 && x_s < 800) && (y_s > 15 && y_s < 100))    
            break;

        if(y_s > 380 && y_s < 450)
        {
            //判断是否为删除
            if(x_s > 240 && x_s < 300)
            {
                if(count_char == 0 || count_jpg == 0)
                {
                    unlock_display_func("unlock_pic1.jpg" , NULL , 1);
                    continue;
                }else{
                    count_char--;
                    count_jpg--;
                    unlock_display_func("unlock_passwd" , &count_jpg , 0);
                    lcd_draw_jpg(0,0,buf_path); 
                    continue;
                }
            }

            //0
            if(x_s > 350 && x_s < 440)
            {
                buf_passwd[count_char++] = '0'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
        }
        
        
        if(y_s > 220 && y_s < 260)
        {
            //1
            if(x_s > 240 && x_s < 300)
            {
                buf_passwd[count_char++] = '1'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
            //2
            if(x_s > 350 && x_s < 440)
            {
                buf_passwd[count_char++] = '2'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
            //3
            if(x_s > 450 && x_s < 520)
            {
                buf_passwd[count_char++] = '3'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
        }
        
        if(y_s > 265 && y_s < 315)
        {
            if(x_s > 240 && x_s < 300)
            {
                buf_passwd[count_char++] = '4'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
            //5
            if(x_s > 350 && x_s < 440) 
            {
                buf_passwd[count_char++] = '5'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
            //6
            if(x_s > 450 && x_s < 520)
            {
                buf_passwd[count_char++] = '6'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
        }
        
        if(y_s > 330 && y_s < 370)
        {
            //7
            if(x_s > 240 && x_s < 300) 
            {
                buf_passwd[count_char++] = '7'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
            //8
            if(x_s > 350 && x_s < 440) 
            {
                buf_passwd[count_char++] = '8'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;
                continue;
            }
            //9
            if(x_s > 450 && x_s < 520) 
            {
                buf_passwd[count_char++] = '9'; 
                unlock_display_func("unlock_passwd" , &count_jpg , 0);
                printf("password:%s\n",buf_passwd);count_jpg++;    
                continue;
            }
        }
        
    }

    if(!strcmp(PASSWD_STA,buf_passwd) || (!strcmp(qrcode_passwd,buf_passwd)))
    {
        unlock_display_func("unlock_success.jpg" , NULL , 1);
        while(1)
        {
            lcd_tch_event(&x_s , &y_s);
            if((x_s > 655 && x_s < 780) && (y_s > 10 && y_s < 100))
                break;
        }
    }else{
        if(count_failed == 2)
        {
            unlock_display_func("unlock_failed_qr.jpg" , NULL , 1);
            
            char *qrcode_str = NULL;
            qrcode_str = (char *)calloc(64,sizeof (char));

            sprintf(qrcode_str,"临时密码==>[%s]",qrcode_passwd);
            qrcode_display(qrcode_str,10,10);

            count_failed = 0;
            while (1)
            {
                lcd_tch_event(&x_s , &y_s);
                if(x_s > 605 && x_s < 790 && y_s > 25 && y_s < 105)
                    break;
            }
            goto passwd_failed_handle;
        }
        count_failed++;
        unlock_display_func("unlock_failed.jpg" , NULL , 1);
        while (1)
        {
            lcd_tch_event(&x_s , &y_s);
            if((x_s > 15 && x_s < 160) && (y_s > 25 && y_s < 120))
                break;
        }
        goto passwd_failed_handle;
    }   
	
    printf("即将进入管理界面\n");
    
}

void interface_menu(void)
{
    int flag = 0; size_t page = 0;
    static int x_s = 0 , y_s = 0;

    static int x_rt = 0, y_rt = 0;
    DIR * dir_cpy = NULL; 

    /*作为目录检索功能中提取文件信息的链表*/
    P_Node udisk_music = new_Node (NULL , 'U');
    P_Node udisk_video = new_Node (NULL , 'U');
    P_Node udisk_picture = new_Node (NULL , 'U');
    
    char music_path_buf[64] = {0};
    char video_path_buf[64] = {0};
    int first = 0;
init_menu:
    
    picture_chose_func(PIC_LOGIN_PATH , "background1.jpg" , NULL , 1);
    while (1)
    {
        
        x_s = y_s = 0;
        switch (flag = lcd_tch_slip_event(&x_s , &y_s))
        {
            case UP:
                picture_chose_func(PIC_LOGIN_PATH,"background1.jpg",NULL,1); page = 1;
                break;
            case DOWN:
                picture_chose_func(PIC_LOGIN_PATH,"background2.jpg",NULL,1); page = 2;
                break;
            case PRESS:
                printf("[tch_menu_event](x : %d, y : %d)\n", x_s , y_s);
                break;
            case QUIT:
                printf("[tch_menu_event] exit the program\n");
                system(MPLAY_QUIT_PROGRAM);
                exit(1);
                break;
        }

        flag = 0;
        //检测上下滑动切换页面
        if(flag == UP || flag == DOWN)    
            continue;
        
        switch (page)
        {
            //第一页的菜单检测
            case 1:
            if(x_s > 190 && x_s < 305) 
            {
                if(y_s > 80 && y_s < 170)
                {    
                    printf("即将进入相册\n");
                    if(photo_display_func() == -1)
                        {puts("退出相册\n");goto init_menu;}
                }
                else if(y_s > 200 && y_s < 265)
                {
                    printf("即将进入音频播放\n");
                    if(music_display_func() == -1)
                        {puts("退出音频播放\n");goto init_menu;}
                }
                else if(y_s > 300 && y_s < 370)
                {
                    printf("即将进入视频播放\n");
                    if(video_display_func() == -1)
                        {puts("退出视频播放\n");goto init_menu;}
                }
            }   
                                    
            break;
            //第二页的菜单检测
            case 2:

            if(x_s > 160 && x_s < 285)
            {
                if(y_s > 110 && y_s < 210)
                {
                    printf("即将进入刮刮乐!\n");
                    if(scrape_display_func() == -1)
                        {puts("退出刮刮乐\n");goto init_menu;}
                }

                if(y_s > 220 && y_s > 315)
                {
                    printf("即将进入目录检索\n");

                    if((dir_cpy = opendir(UDISK_PATH)) == NULL)
                    {
                        lcd_draw_jpg(0,0,"/template/picture/udisk/udisk_noexist.jpg");
                        fprintf(stderr , "请检查U盘是否插入 , errno >> %s\n" , strerror(errno));
                        while ((lcd_tch_slip_event(&x_rt , &y_rt)) != LEFT); 
                        goto init_menu;
                    }else{
                        lcd_draw_jpg(0,0,"/template/picture/udisk/udisk_loading.jpg");
                        
                        // 检索目录 --> 添加节点到链表中
                        int num_music = find_dir(UDISK_MUSIC_PATH, ".mp3" , udisk_music);
                        // 遍历显示链表
                        int count_member = 0;
                        if(!udisk_list_func(udisk_music , &count_member , MUSIC_PATH))
                        {
                            lcd_draw_jpg(0,0,"/template/picture/udisk/udisk_failed.jpg");
                            while ((lcd_tch_slip_event(&x_rt , &y_rt)) != LEFT); 
                            destroy_list(udisk_music);
                            goto init_menu;
                        }
                        // 检索目录 --> 添加节点到链表中
                        int num_video = find_dir(UDISK_VIDEO_PATH , ".mp4" , udisk_music);
                        // 遍历显示链表
                        count_member = 0;
                        if(!udisk_list_func(udisk_video , &count_member , VIDEO_PATH))
                        {
                            lcd_draw_jpg(0,0,"/template/picture/udisk/udisk_failed.jpg");
                            while ((lcd_tch_slip_event(&x_rt , &y_rt)) != LEFT); 
                            destroy_list(udisk_video);
                            goto init_menu;
                        }
                        
                        // 检索目录 --> 添加节点到链表中
                        int num_jpg = find_dir(UDISK_PIC_PATH , ".jpg" , udisk_music);
                        // 遍历显示链表
                        count_member = 0;
                        if(!udisk_list_func(udisk_picture , &count_member , PIC_PATH))
                        {
                            lcd_draw_jpg(0,0,"/template/picture/udisk/udisk_failed.jpg");
                            while ((lcd_tch_slip_event(&x_rt , &y_rt)) != LEFT); 
                            destroy_list(udisk_picture);
                            goto init_menu;
                        }

                        lcd_draw_jpg(0,0,"/template/picture/udisk/udisk_success.jpg");
                        while ((lcd_tch_slip_event(&x_rt , &y_rt)) != LEFT); 
                        destroy_list(udisk_picture);
                        destroy_list(udisk_video);
                        destroy_list(udisk_music);
                        goto init_menu;
                    }
                }
            }
            break;
        }
    }
}
