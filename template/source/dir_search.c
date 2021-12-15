#include "dir_search.h"

#define NAME_SIZE 257

int find_dir(const char *path , const char *type , P_Node head)
{
    int num = 0;
    char path_name [128];
    printf("Path:%s\n", path);

    //打开目录文件(文件列表)
    DIR *dirp = NULL;
    dirp = opendir(path);

    if (NULL == dirp)
    {
        fprintf( stderr ,"open %s dir error >> %s\n" , path , strerror(errno));
        return -1;
    }
    
    //读取目录项
    while (1)
    {
        struct dirent * dir_info = readdir(dirp);
        if(dir_info == NULL)
        {
            perror("read dir error");
            break;
        }
        
        printf("d_type:%d \t d_name:%s \t ",dir_info->d_type , dir_info->d_name);
        putchar('\n');

        //处理目录
        if( dir_info->d_type == 4 && dir_info->d_off > 2)
        {
            snprintf(path_name , 128 , "%s/%s" , path , dir_info->d_name);
            num += find_dir(path_name , type , head);       // +num  偏移前面已经使用的数组元素
        }

        if(dir_info->d_type == 8)
        {
            char * tmp = strrchr(dir_info->d_name , '.');
            if(NULL == tmp) {continue;}

            if( !strcmp (tmp , type) )
            {
                printf("%s files : %s\n" , type ,dir_info->d_name );
                // 把该文件添加到链表中
                snprintf(path_name , 128 , "%s/%s" , path , dir_info->d_name);// 拼接路径+名字
                P_Node new = new_Node(path_name , 'M');  // 通过新的路径名 来创建要给新的节点
                insert_node(head , new); // 把新节点添加到链表中
                num ++ ;

            }else{
                printf("[%s]-->%d",__FUNCTION__ ,__LINE__);
            }
        }
        
    }
}