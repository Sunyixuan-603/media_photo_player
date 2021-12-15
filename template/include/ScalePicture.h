#ifndef __SCALE_H_
#define __SCALE_H_

//--- 系统头文件 ---//
#include <stdio.h>

//--- 函数原型 ---//
//pColorData是原图像素RGP数组
void ScalePicture(unsigned char pColorData[],int o_w,int o_h,unsigned char pColorDataMid[],int d_w,int d_h);



#endif