#ifndef __DIR_SEARCH_H__
#define __DIR_SEARCH_H__

#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "list.h"

int find_dir(const char * path , const char * type , P_Node head );


#endif
