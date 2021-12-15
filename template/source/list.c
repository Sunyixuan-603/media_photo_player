#include "list.h"

P_Node new_Node(const char * path_Name , char Type)
{
    //创建一个新的节点空间
    P_Node head = calloc(1 , sizeof (Node));
    if(NULL == head)
    {
        perror("calloc error >>> ");
        return NULL;
    }

    //初始化两个指针 前驱/后驱
    head->next = head;
    head->prev = head;

    //初始化数据域
    head->Type = Type;
    if(path_Name != NULL)
    {
        strncpy(head->Path_Name , path_Name , 128);
    }

    return head;
}

bool insert_node(P_Node head , P_Node new)
{
    if(head == NULL || new == NULL)
        return false;

    new->prev = head;
    new->next = head->next;
    head->next->prev = new;
    head->next = new;

    return true;
}

bool list_4_eath (P_Node head , int * num)
{
    if(head == NULL || head->next == head)
        return false;

    //定义一个用来遍历目录下文件的前驱节点
    P_Node tmp = head->prev ;

    for( *num = 0; tmp != head ; tmp = tmp->prev , *num++)
        printf("FileName:%s \t Type:%c \n" , tmp->Path_Name , tmp->Type);

    
    return true;
}

bool Delete_Node(P_Node node)
{
    if(node == NULL)    
        return false;

    node->next->prev = node->prev;
    node->prev->next = node->next;
    //最后一定要释放节点

    free(node);
    return true;
}

//销毁链表
void destroy_list(P_Node node)
{
    if(node == NULL)
        return ;

    P_Node del_node = node->next;
    while (del_node->next != node)
    {
        Delete_Node(del_node);
        del_node = del_node->next;
    }
    free(del_node);
    return ;
    
}