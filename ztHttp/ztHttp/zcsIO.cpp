/**
 *1.The wrapper of standard c library of POSIX.1
 *2,Useful customize function
 * **/

#include <glog/logging.h>
#include <fcntl.h>
#include <signal.h>

#include "zcsIO.h"

namespace ztHttp{

sigset_t mask;

volatile sig_atomic_t quit_wait_flag=0;
volatile sig_atomic_t register_flag=0;




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

/*
 *signal的可靠实现
 */
sigfunc* z_signal(int signo, sigfunc *p_func) {

    struct sigaction act, old_act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = p_func;

    //除SIGALRM外，被其他signal中断的系统调用可以重启；
    //SIGALRM可用于中断低速系统调用，该signal中断的系统调用不会被重启；
    if (signo==SIGALRM) {

        #ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
        #endif

    } else {

        #ifdef SA_RESTGART
        act.sa_flags |= SA_RESTART;
        #endif

    }

    //0,-1
    if (sigaction(signo, &act, &old_act)) {

        LOG(INFO)<<"z_signal(): sigaction error";
        return SIG_ERR;

    }

    return old_act.sa_handler;

}


void *signal_handle(void *arg) {

    int signo=0, err=0;

    while(1) {

        if((err=sigwait(&mask, &signo)) != 0)
            LOG(ERROR)<<"signal_handle(): sigwait error";

        switch (signo) {

            case SIGINT:
                quit_wait_flag=2;
                //
                break;

            case SIGUSR1:
                ++register_flag;
                break;

            default:
                LOG(ERROR)<<"signal_handle(): unexpected signal"<<signo;

        }

    }



}

}
