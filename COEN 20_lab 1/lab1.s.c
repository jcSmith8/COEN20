// Cooper Smith
// COEN 20 lab 1
// 1/15/2020

#include <stdint.h>
#include <math.h>

//Converts the given array of bits to an UNSIGNED int
uint32_t Bits2Unsigned(int8_t bits[8]) {	
	uint32_t n = 0;
	for(int i=7;i>=0;i--){
		n = (2 * n + bits[i]);
	}
	return 0;
}

//Converts the given array into a SIGNED int
int32_t Bits2Signed(int8_t bits[8]) {
	int32_t result = 0;
	result -= pow(2,7)*bits[7];
	for(int i=0;i<8;i++){
		result += (pow(2,i)*bits[i]);
	}
	return 0;
}

//Increments the given array of bits
void Increment(int8_t bits[8]) {
	for(int i=0;i<8;i++){
		if(bits[i]==0){
			bits[i]=1;
			break;
		}
		bits[i]=0;
	}
}

//Converts the given n value (UNSIGNED) to an array of bits
void Unsigned2Bits(uint32_t n, int8_t bits[8]) {
	int i=0;
	while(n>0){
		bits[i] = n%2;
		i++;
		n /= 2;
	}
}



