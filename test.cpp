
#include <iostream>
#include <typeinfo>
#include <map>
#include <vector>
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



struct Contain {
	Contain(){}
	virtual ~Contain(){}
	long long a=1;
	double b={1};//() error
	double c{1};
};

struct CA: Contain {
	
};

int _aaa, __aaaa, _A;
const int i=0, &j=i, *g=&i;
const int *const h=NULL, f=0;
int main(){
    map<int, int> am;
    am[1]=1;
    am[2]=2;
    for(auto tmp: am)
        cout<<tmp.first<<endl;
	CA ca;
	/*Contain con;
	//bad error
	//vector<Contain&> testVec{con}; cout<<sizeof(testVec)<<endl;	
	vector<Contain> testVec1{con}; cout<<sizeof(vector<Contain>)<<"  "<<sizeof(testVec1)<<endl;	
	Contain & r_con=con;
	testVec1.push_back(r_con);
	testVec1.reserve(100);
	cout<<sizeof(vector<Contain>)<<"  "<<sizeof(testVec1)<<endl;	
	testVec1.resize(100);
	cout<<sizeof(vector<Contain>)<<"  "<<sizeof(testVec1)<<endl;	
   */
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
