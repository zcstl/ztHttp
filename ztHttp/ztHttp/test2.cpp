#include <iostream>
#include <string>

using namespace std;

class Solution {
    public:
        string reverseVowels(string s) {
			int siz=s.size();
			if(siz<=1)
				return s;
			int sta=0, ed=siz-1;
			int count=1;
			while(1) {
				while(_vowels.find(s[sta])==string::npos && sta< siz-1)++sta;
				cout<<sta<<"  ";
                if(sta>=ed)break;
                while(_vowels.find(s[ed])==string::npos && ed>0)--ed;
				cout<<ed<<" ";
                if(ed<=sta)break;
                swap(s[sta++], s[ed--]);//错误写法：swap(s[sta], s[ed]);
				//cout<<endl;  2. 此处没注释，ｍａｉｎ会报段错误，注释了便不报,无限打印
				if(++count==10)
					break;
           }
           return s;//1. 若遗漏返回值，ｌｉｎｕｘ下ｇ＋＋没有报错
		}

	private:
		string _vowels="aeiouAEIOU";//´óÐ¡Ð´
};

int mainlee() {
	Solution sol;
	cout<<sol.reverseVowels("hello")<<endl;
	//因为没有返回值，所以随便打印了一个地址，未定义
}
