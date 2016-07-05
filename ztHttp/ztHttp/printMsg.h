#include <iostream>

#ifndef PRINT_MSG
#define PRINT_MSG

#define ERROR_DIE(str, num) {std::cout<<"The program die in "<<str<<" exit "<<num<<"!!"<<std::endl; exit(num);}  //宏的位置?

#define MSG_PRINT(str) {std::cout<<str<<std::endl;}

#endif
