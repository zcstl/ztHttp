#include<stdio.h>
#include <iostream>
#include <typeinfo>
namespace zz {
using namespace std;
}
using namespace zz;

char a[5]="test";
char (* func())[5]{
	printf("%s\n","meiwenti");
	int c;
    int *d=0;
    int tt=1;
    cout<<tt<<endl;
	return &a;
}

struct A{
  virtual void test()=0;
};
struct AA:public A{
  virtual void tt()=0;
  void test(){
    cout<<"test****"<<endl;
  }
};
struct AB:public A{
  void test(){
    cout<<"test****"<<endl;
  }
};


void testArray(int i[10]){
    cout<<"****"<<sizeof(i)<<endl;
    cout<<"****"<<typeid(i).name()<<endl;
}

void tt(){
const char r[4]="123";
const char a[2]="b";
int *b =0;
const char (&c)[3]="ff";
cout<<1<<endl;
}

const char* pp="this in gobal!";
namespace zcs{
const char* p="this is in a namespace";
void test(){
    printf("%s\n",p);
    printf("%s\n",pp);
    char aa[2]={'a','b'};
    bool* used=new bool[10];
    cout<<sizeof(used)<<sizeof(aa)<<endl;
}
}



int _aaa, __aaaa, _A;
const int i=0, &j=i, *g=&i;
const int *const h=NULL, f=0;
int main(){
    using TTT=int;
    TTT ttt=1;
    //cout<<sizeof(A)<<" "<<sizeof(AA)<<" "<<sizeof(AB)<<endl;
    int arr[10]={0};
    cout<<"****"<<typeid(arr).name()<<endl;
    cout<<"++++"<<typeid(func()).name()<<endl;
    testArray(arr);
    g=NULL;
    zcs::test();
	printf("%s\n",*func());
	int x;
	x=0;
	extern int y;
	y=2;
    unsigned char xx=-1;
    printf("%d\n",xx);
	return 0;
}
