/**
 *1.The wrapper of standard c library of POSIX.1
 *2,Useful customize function
 * **/
#include <signal.h>

using sigfunc=void(int);


namespace ztHttp {

extern sigset_t mask;

extern volatile sig_atomic_t quit_wait_flag;
extern volatile sig_atomic_t register_flag;

/*
 * turn on/off the file statue flags
 * */
void file_flg_set(int fd, int flags, bool trunOn);

/**/
sigfunc* z_signal(int signo, sigfunc *p_func);


void *signal_handle(void *arg);


}
