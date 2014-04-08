#ifndef BITVECTOR_H
#define BITVECTOR_H
#include <vector>
#include <iostream>
using namespace std;
class BitVector{
	const int size()const{ return value.size();}


	public:
		vector<bool> value;
		bool isSet(int index){return value[index];}
		friend ostream& operator<<(ostream &os, const BitVector &bitVector);
		void ini(int size);
		void set(int index);
		void clear(int index);
		BitVector& operator-=(const BitVector &rhs);
		BitVector& operator+=(const BitVector &rhs);
		friend BitVector operator-(BitVector lhs, const BitVector &rhs);
		friend BitVector operator+(BitVector lhs, const BitVector &rhs);
		void join(const BitVector &rhs)const;
		void setAll();
		static BitVector join(BitVector lhs, BitVector rhs);
		//inline BitVector operator+(BitVector lhs, const BitVector &rhs);
};
		//inline BitVector operator-(BitVector lhs, const BitVector &rhs);
		//inline BitVector operator+(BitVector lhs, const BitVector &rhs);
#endif
