#ifndef __BIG_INTEGER_H_
#define __BIG_INTEGER_H_

#include <iostream>
	using std::ostream;

// type definition of word-level datatype
typedef unsigned long bigIntType;

// Number of 32-bit words concatenated to BigInteger
#define BIG_INTEGER_MAX_WORDS 12

//---------------------------------------------------------------
/*
 * Datatype to handle big numbers
 * Information is handled in value[], an unsigned long array of the size BigInteger_MAX_WORDS
 */
class BigInteger
{
	friend ostream& operator << (ostream&, const BigInteger &);
private:
	// The value of the BigInteger
	//	value[0] is the least-significant word
	//	value[max_32bitwords-1] is the most-significant word
	bigIntType value[BIG_INTEGER_MAX_WORDS];
public:
	BigInteger(void);							// default constructor
	BigInteger(bigIntType val);					// constructor to initialize instance with long
	BigInteger(const char str[]);				// constructor to initialize instance with string
	~BigInteger(void);							// destructor
	BigInteger& addc(const BigInteger &addend);	// multi-precision addition
	BigInteger& adda(const BigInteger &addend);	// multi-precision addition
	BigInteger& shiftLeft(unsigned int val);	// multi-precision shift-left
	BigInteger& modp192c();						// modular reduction
	BigInteger& modp192a();						// modular reduction
	bool compare(const BigInteger &testme);		// multi-precision comparison
};

// Input- and output operators
ostream& operator << (ostream& outstr, const BigInteger &bigint);

// some BigInteger constants: 0, 1
static const BigInteger BIG0, BIG1(1);

#endif	// __BIG_INTEGER_H_
