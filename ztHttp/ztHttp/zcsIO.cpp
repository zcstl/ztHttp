/**
 *1.The wrapper of standard c library of POSIX.1
 *2,Useful customize function
 * **/

#include <glog/logging.h>
#include <fcntl.h>

#include "zcsIO.h"

void file_flg_set(int fd, int flags, bool turn_on) {

    int val;

    if ((val=fcntl(fd, F_GETFL, 0))==-1)
        LOG(ERROR)<<"fcntl: F_GETFL";

    if (turn_on) 
        val |=flags;
    else
        val &=~flags;

    if (fcntl(fd, F_SETFL, val)==-1)
        LOG(ERROR)<<"fcntl: F_SETFL";

} 
