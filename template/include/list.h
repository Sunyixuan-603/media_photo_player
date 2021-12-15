#ifndef __LIST_H__
#define __LIST_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>



// 节点设计
typedef struct list_node 
{
    // 数据域
    char Path_Name[128] ;
    char Type ; //   B   J   3   A 4

    // 指针域
    struct list_node * next , * prev ;
}Node , * P_Node ;


bool list_4_eath( P_Node head , int *);
bool insert_node( P_Node head , P_Node new);
P_Node new_Node( const char * Path_Name ,  char Type ) ;
bool Delete_Node(P_Node node);
void destroy_list(P_Node node);
#endif
