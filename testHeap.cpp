// range heap example
#include <iostream>     // std::cout
#include <algorithm>    // std::make_heap, std::pop_heap, std::push_heap, std::sort_heap
#include <vector>       // std::vector

using namespace std; //名字得先声明，再使用

#define PRINT(a) {cout<<"****"<<endl; for(auto it:a)cout<<it<<" "; cout<<endl;}

int main () {
  int myints[] = {10,20,30,5,15};
  std::vector<int> v(myints,myints+5);

  //std::make_heap (v.begin(),v.end());PRINT(v);
  std::sort_heap (v.begin(),v.end());PRINT(v);
  std::cout << "initial max heap   : " << v.front() << '\n';

  std::pop_heap (v.begin(),v.end());PRINT(v); v.pop_back();PRINT(v);
  std::cout << "max heap after pop : " << v.front() << '\n';

  v.push_back(99);PRINT(v); std::push_heap (v.begin(),v.end());PRINT(v);
  std::cout << "max heap after push: " << v.front() << '\n';

  std::sort_heap (v.begin(),v.end());PRINT(v);

  std::cout << "final sorted range :";
  for (unsigned i=0; i<v.size(); i++)
    std::cout << ' ' << v[i];

  std::cout << '\n';

  return 0;
}

