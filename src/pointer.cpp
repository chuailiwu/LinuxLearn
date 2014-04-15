#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <string>
using namespace std;

template <typename T>
class Pointer {
public:
	Pointer(T* p):pt(p) {}
	T* operator->() {
		cout << "Pointer operator->() is called" << endl;
		return pt;
	}
private:
	T* pt;
};

template <typename T>
class Pointer2 {
public:
	Pointer2(T* p):pt(p) {}
	T& operator->() {
		cout << "Pointer2 operator->() is called" << endl;
		return *pt;
	}
private:
	T* pt;
};

class T1 {
public:
	int a;
	int b;
	void print() { cout << "========" << endl; }
	T1* operator->() {
		cout << "T1 operator->() is called" << endl;
		return this;
	}
};

class T2 {
public:
	int a;
	int b;
	void print() { cout << "++++++++" << endl; }
	T2* operator->() {
		cout << "T2 operator->() is called" << endl;
		return this;
	}	// 此处不能为*this，否则会递归
};

int main(int argc, char* argv[])
{
	Pointer<T1> p(new T1);
	p->print();
	(p.operator->())->print();

	cout << endl;
	Pointer2<T2> p2(new T2);
	p2->print();
	(p2.operator->()).print();
	((p2.operator->())->operator->())->print();

	return 0;
}

