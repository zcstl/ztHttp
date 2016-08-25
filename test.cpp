
#include <iostream>
#include <typeinfo>
#include <map>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
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

class Base;
class Der;

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


class Base {

};

class Der: public Base {

};

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

struct testArr {
	int a;
	char b[]; //no space

};


struct Rval{
	Rval()=default;
	Rval(Rval && r): a(r.a){}
	int a;
};


class Ba{
	virtual void test()=0;
};
class De{
public:
	De(Ba&);
};

void testNameDup() {
	cout<<"this is a func"<<endl;
}

void testest(const int &a) {
    cout<<a<<endl;
}

//无效static int testStatic=0;
namespace {
    static int teststatic=1;
}

struct TestStaticF {
    static constexpr int a=0;
};

//此处若不定义，则testest调用出错；
constexpr  int TestStaticF::a; 


struct Base1 {
    int i;
};

class Der1: private Base1 {
    public:
        using Base1::i;
};

struct DerDer1: private Der1 {
    void test(){
        cout<<"DerDer1  "<<i<<endl;
    }
};


int main(int argc, char* argv[]){
    
    DerDer1 pp;
    pp.test();
    
    testest(TestStaticF::a);

	testNameDup();
	const char* testNameDup="this is a c_str";
	cout<<testNameDup<<endl;
	//testNameDup();改名字被覆盖，不是函数了
    
	cout<<"*******"<<endl;
    for(int i=0; i<argc; ++i)
        cout<<argv[i]<<endl;
    cout<<"*******"<<endl;

	cout<<typeid(nullptr).name()<<endl;
	//cout<<"\"rval\": "<<typeid("rval")<<endl;
	//cout<<"1 : "<<typeid(1)<<endl;
	int && a=1;
	a=2;
	//cout<<a<<endl;
	const int & bb=1;
	Rval rval;
	cout<<typeid(std::move(rval)).name()<<endl;
	cout<<typeid(rval).name()<<endl;
	const Rval & pr=std::move(rval);
	int && aaa=std::move(1);
	Rval && pr1=std::move(rval);
	Rval && pr2=std::move(pr1);
	pr1.a=22;
	cout<<1+rval.a<<"－－－"<<endl;
	//cout<<"a: "<<typeid(a)<<endl;
	//cout<<"std::move(a): "<<typeid(std::move(a))<<endl;
	//error int aa[];

	cout<<sizeof(struct sockaddr)<<" <<<"<<sizeof(struct sockaddr_in6)<<"<<<"<<sizeof(struct sockaddr_in)<<"<<<"<<sizeof(struct testArr)<<endl;

	//struct testArr addr{1,{1,2,3}};
	//addr.a=1;
	char aa[0];
	//addr.b={1,2,3};
	cout<<sizeof(aa)<<"<<"<<endl;

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

	Base b;
	Base* pb=&b;
	Der* pd=static_cast<Der*>(pb);
	Der d;
	pb=&d;
	return 0;

}


