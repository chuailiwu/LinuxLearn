#include <iostream>
#include <iterator>
#include <vector>
#include <algorithm>
#include <assert.h>

using namespace std;

// 求两个自然数间最大公约数
unsigned long gcd(unsigned long n, unsigned long m)
{
	assert(n > 0 && m > 0);
	for (;;)
	{
		if (n > m)
		{
			n = n % m;
		} else
		{
			m = m % n;
		}
		if ( n == 0)
			return m;
		if ( m == 0)
			return n;
	}
	return 1;
}

// 求两个自然数间最小公倍数
unsigned long lcm(unsigned long n, unsigned long m)
{
	int g = gcd(n,m);
	return n * m / g;
}

class MyRational {
public:
	MyRational() {}
	MyRational(unsigned long num, unsigned long den);
	MyRational(const MyRational& r): numer(r.numer), denom(r.denom) {}
	MyRational& operator+=(const MyRational& r);

	unsigned long numerator() const { return numer; }
	unsigned long denominator() const { return denom; }
	friend ostream& operator<<(ostream& os, const MyRational&);
private:
	unsigned long numer;
	unsigned long denom;
};

MyRational::MyRational(unsigned long num, unsigned long den)
{
	if (num == 0)
	{
		numer = num;
		denom = den;
		return;
	}
	int g = gcd(num, den);
	numer = num / g;
	denom = den / g;
}

MyRational& MyRational::operator+=(const MyRational& r)
{
	int den = lcm(denom, r.denom);
	int num = den / denom * numer + den / r.denom * r.numer;
	int g = gcd(num, den);
	numer = num / g;
	denom = den / g;
	return *this;
}

ostream& operator<<(ostream& os, const MyRational& r)
{
	os << r.numerator() << "/" << r.denominator();
	return os;
}

MyRational operator+(const MyRational& lhs, const MyRational& rhs)
{
	MyRational tmp(lhs);
	return tmp+= rhs;
}

struct RationalNumber {
	RationalNumber():numer(1), denom(0) {}
	MyRational operator()() {
		denom += 2;
		return MyRational(numer, denom);
	}
private:
	unsigned long numer;
	unsigned long denom;
};

// 有一分数序列：1/2,1/4,1/6,1/8……，用函数调用的方法，求此数列前20项的和
MyRational accu_rational_seq(int seq_len)
{
	vector<MyRational> ratVec(seq_len);
	generate(ratVec.begin(), ratVec.end(), RationalNumber());
	copy(ratVec.begin(), ratVec.end(), ostream_iterator<MyRational>(cout, " "));
	return accumulate(ratVec.begin(), ratVec.end(), MyRational(0,1), plus<MyRational>());
}

// 使用自定义类的方法
int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "请输入序列的长度！" << endl;
		exit(-1);
	}
	int seq_len = atoi(argv[1]);
	if (seq_len == 0)
	{
		cout << "请输入正确的序列长度！" << endl;
		exit(-1);
	}
	MyRational res = accu_rational_seq(seq_len);
	cout << "res:" << res << endl;
	return 0;
}

