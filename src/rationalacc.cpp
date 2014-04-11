#include <iostream>
#include <vector>
#include <algorithm>
#include <boost/rational.hpp>
using namespace boost;
using namespace std;

struct RationalNumber {
	RationalNumber(int num, int den, int inc):numer(num), denom(den), i(inc) {}
	rational<int> operator()() {
		denom += i;
		return rational<int>(numer, denom);
	}
private:
	int numer;
	int denom;
	int i;
};

// 有一分数序列：1/2,1/4,1/6,1/8……，用函数调用的方法，求此数列前20项的和
rational<int> accu_rational_seq(int seq_len)
{
	vector<rational<int> > ratVec(seq_len);
	generate(ratVec.begin(), ratVec.end(), RationalNumber(1, 0, 2));
	copy(ratVec.begin(), ratVec.end(), ostream_iterator<rational<int> >(cout, " "));
	return accumulate(ratVec.begin(), ratVec.end(), rational<int>(0,1), plus<rational<int> >());
}

// 使用boost rational的方法
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
	rational<int> res = accu_rational_seq(seq_len);
	cout << "res:" << res << endl;
	return 0;
}

