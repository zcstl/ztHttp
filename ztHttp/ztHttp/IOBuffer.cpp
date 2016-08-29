/**


**/

#include <vector>
#include <list>
#include <typeinfo>

#include "gtest/gtest.h"
#include "glog/logging.h"

#include "../../IOBuffer.h"

using namespace std;

namespace ztHttp {


IOBuffer::IOBuffer():_size(0) {

}

IOBuffer::IOBuffer(const char* data, unsigned elem_size):_size(0) {

    append(data, elem_size);

}

//virtual outside class declaration error:  virtual只能在类定义内使用
IOBuffer::~IOBuffer() {

}

//类似于深拷贝
bool IOBuffer::append(IOBufferAbstractClass* chunk) {

    if(!chunk)
        return true;

    //typeid(e)检查e的类型
    IOBuffer* echunk=nullptr;
    //typeid关键字，类似于ｓｉｚｅｏｆ，返回ｔｙｐｅｉｎｆｏ，需ｉｎｃｌｕｄｅ
    if(typeid(*chunk) == typeid(IOBuffer)) {

        echunk=dynamic_cast<IOBuffer*>(chunk); //<>()

    }
    else {

        LOG(FATAL)<<"IOBuffer::append(): typeid error!";
        return false;

    }
    //chunk is list<vector<char>>
    for(auto tmp:echunk->_chunks) {
        _chunks.push_back(tmp);
    }

    _size+=echunk->_size;

    //delete echunk;//拷贝完数据后，删除原数据


    return true;
}

//方案一：new个vector，然后resize，接着调用memcpy函数
//方案二：reinterpret该指针成char*，放入vector
template <typename T>
bool IOBuffer::append(const T* pT, unsigned num) {

    int len=sizeof(T)*num;
    _size+=len;

    const char* cpT=nullptr;
    cpT=reinterpret_cast<const char*>(pT);//dangerous for using reinterpret_cast
    vector<char>* p_vec=new vector<char>(cpT, cpT+len);//vector的iterator constructor也可以使用数组的首末地址
    _chunks.push_back(*p_vec);

    delete p_vec;
    p_vec=nullptr;

    return true;

}

char* IOBuffer::pullDown(unsigned num) {

    if(num>_size || _size==0) {//当num不小心为0时，且_size为0，返回空指针
        //
        LOG(INFO)<<"IOBuffer::pull_down() bad num size";
        return nullptr;

    }

    auto iter_begin = _chunks.begin(), iter = ++_chunks.begin();//list iter only: --  ,++
    int len=0;
    len+=iter_begin->size();

    while(len<num) {//if i==num, break

        len+=iter->size();
        iter_begin->insert( (*iter_begin).end(), (*iter).begin(), (*iter).end() );//从特定位置之前插入
        _chunks.erase(iter++);

    }

    return &_chunks.begin()->at(0);//>.<  at与[]运算符功能类似，但越界会报错

}

//consume与pollDown配合使用，若没有使用pullDown，直接使用consume可能失败
bool IOBuffer::consume(unsigned num) {

    auto iter =_chunks.begin();
    int siz=iter->size();
    if(num > siz || siz == 0) {
        //
        return false;
    }

    if(siz == num) {

        _chunks.erase(iter);
        _size -= num;
        return true;

    }

    iter->erase(iter->begin(), iter->begin()+num);
    _size -= num;

    return true;

}

unsigned IOBuffer::size() const {

    return _size;

}

/**
 *unit test
 * **/
TEST(IOBuffer_test, test_all) {

    char c1[]="test1";
    char c2[]="test2";
    char c3[]="test3";

    IOBuffer *bf=new IOBuffer(c1, strlen(c1));
    ASSERT_EQ(5, bf->size());

    ASSERT_TRUE(bf->append(new IOBuffer(c2, strlen(c2))));

    //strlen  null char
    bf->append(c3, sizeof(c3)/sizeof(char));
    ASSERT_EQ(16, bf->size());

    //char t1[7]={0};
    //strcpy(t1, bf->pullDown(6));
    //ASSERT_STREQ("test1t", t1);

    bf->pullDown(6);
    bf->consume(6);
    ASSERT_EQ(10, bf->size());

}


}
