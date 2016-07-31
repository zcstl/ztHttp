/**
 *
 *
 * **/

#ifndef ZTHTTP_IOBUFFER
#define ZTHTTP_IOBUFFER

#include <vector>
#include <list>
#include <typeinfo>

using namespace std;

namespace ztHttp {


/*设计接口：抽象服务
 *简单的缓存实现，不支持多线程使用同个实例，使用方法：size获取大小，pullDown得连续存储，使用后再调用consume删除部分缓存，然后再到下个周期
 * */
class IOBufferAbstractClass {
public:
    virtual bool append(IOBufferAbstractClass* chunk)=0;
    //virtual bool append(const char* data, unsigned len)=0;  dup
    template <typename T>//templates may not be virtual
    bool append(const T* pT, unsigned num);
    virtual char* pullDown(unsigned num)=0;//返回类型改为share_ptr可好？
    virtual bool consume(unsigned num)=0;
    virtual unsigned size() const=0;
};

class IOBuffer: public IOBufferAbstractClass {
public:

    IOBuffer():_size(0){}
    IOBuffer(const char* data, unsigned len):_size(0){
        append(data, len);
    }
    //析构声明为虚函数的目的？
    //把未使用数据清空，避免内存泄漏
    virtual ~IOBuffer() {}


    bool append(IOBufferAbstractClass* chunk) {
        //typeid(e)检查e的类型
        IOBuffer* echunk=nullptr;
        //typeid关键字，类似于ｓｉｚｅｏｆ，返回ｔｙｐｅｉｎｆｏ，需ｉｎｃｌｕｄｅ
        if(typeid(*chunk) == typeid(IOBuffer)) {
            echunk=dynamic_cast<IOBuffer*>(chunk); //<>()
        }
        else {
            //
            return false;
        }
        //chunk is list<vector<char>>
        for(auto tmp:echunk->_chunks) {
            _chunks.push_back(tmp);
        }
        _size+=echunk->_size;
        delete echunk;//拷贝完数据后，删除原数据
    }

    //方案一：new个vector，然后resize，接着调用memcpy函数
    //方案二：reinterpret该指针成char*，放入vector
    template <typename T>
    bool append(const T* pT, unsigned num) {

        int len=sizeof(T)*num;

        const char* cpT=nullptr;
        cpT=reinterpret_cast<const char*>(pT);//dangerous for using reinterpret_cast
        vector<char>* p_vec=new vector<char>(cpT, cpT+len);//vector的iterator constructor也可以使用数组的首末地址
        _chunks.push_back(*p_vec);
        delete p_vec;
        return true;
    }

    char* pullDown(unsigned num) {
        if(num>_size || _size==0) {//当num不小心为0时，且_size为0，返回空指针
            //
            return nullptr;
        }
        auto iter1=_chunks.begin(), iter=++_chunks.begin();//list iter only: --  ,++
        int len=0;
        len+=iter1->size();
        while(len<num) {//if i==num, break
            len+=iter->size();
            iter1->insert((*iter1).end(), (*iter).begin(), (*iter).end());//从特定位置之前插入
            _chunks.erase(iter1++);
        }
        return &_chunks.begin()->at(0);//>.<  at与[]运算符功能类似，但越界会报错
    }

    //consume与pollDown配合使用，若没有使用pullDown，直接使用consume可能失败
    bool consume(unsigned num) {
        auto iter=_chunks.begin();
        int siz=iter->size();
        if(num>siz || siz==0) {
            //
            return false;
        }
        if(siz==num) {
            _chunks.erase(iter);
        }
        iter->erase(iter->begin(), iter->begin()+num);
        return true;
    }

    unsigned size() const {
        return _size;
    }


private:

    list<vector<char>> _chunks; //将数据拷贝到缓存中，独立于实参；但就要求IO Buffer对象在堆中了，毕竟栈空间有限
    unsigned _size;


    IOBuffer(const IOBuffer&); //将拷贝构造函数声明为私有，编译器会增加其实现，禁止拷贝,禁止原因未知？？

};
}
#endif

