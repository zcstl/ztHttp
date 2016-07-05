#include<stdio.h>
char a[5]="test";
char (* func())[5]{
	printf("%s\n","meiwenti");
	int c;
	return &a;
}

int main(){
	printf("%s\n",*func());
	int x;
	x=0;
	extern int y;
	y=2;
	return 0;
}
