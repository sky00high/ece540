#include <vector>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "BitVector.h"


using namespace std;
ostream& operator<<(ostream &os, const BitVector &bitVector){
	for(int i = 0; i < bitVector.size(); i++){
		os<<bitVector.value[i]?1:0;
	}
	return os;
}
void BitVector::ini(int size){ 
	if(this->size() != size){
		this->value.resize(size);
	}
	for(int i = 0; i < size; i ++){
		this->value[i] = false;
	}
}
void BitVector::set(int index){
	assert(index < size());
	value[index] = true;
}
void BitVector::clear(int index){
	assert(index < size());
	value[index] = false;
}

BitVector& BitVector::operator-=(const BitVector &rhs){
	assert(this->size() == rhs.size());

	for(int i = 0; i < this->size(); i++){
		if(this->value[i] == true){
			if(rhs.value[i] == true){
				this->value[i] = false;
			}
		}
	}
	return *this;
}

BitVector& BitVector::operator+=(const BitVector &rhs){
	assert(this->size() == rhs.size());
	for(int i = 0; i < this->size(); i++){
		if(this->value[i] == false){
			if(rhs.value[i] == true){
				this->value[i] = true;
			}
		}
	}
	return *this;
}
void BitVector::join(const BitVector &rhs) const{
	assert(this->size() == rhs.size());
	for(int i = 0; i < this->size(); i++){
		if(rhs.value[i] == false){
		//	this->clear(i);
		}
	}
}
BitVector BitVector::join(BitVector lhs, BitVector rhs){
	assert(lhs.size() == rhs.size());
	for(int i = 0; i < lhs.size(); i++){
		if(rhs.value[i] == false){
			lhs.value[i] = false;
		}
	}
	return lhs;
}

void BitVector::setAll(){
	for(int i = 0; i < this->size(); i++){
		this->value[i] = true;
	}
}

	

/*
		BitVector operator-(BitVector lhs, const BitVector &rhs){
			lhs -= rhs;
			return lhs;
		}

		BitVector operator+(BitVector lhs, const BitVector &rhs){
			lhs += rhs;
			return lhs;
		}

*/
