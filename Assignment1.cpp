#include <iostream>
	using std::cout;
	using std::cin;
	using std::endl;
#include "BigInteger.h"
#include <stdio.h>

//---------------------------------------------------------------
/*
 * print some constants
 */
void printConstants()
{
	cout << endl << "Running printConstants()" << endl;
	cout << endl;
    cout << "BIG0 = " << BIG0 << endl;
	cout << "BIG1 = " << BIG1 << endl;

	BigInteger big12345678(0x12345678);
	cout << "big12345678 = " << big12345678 << endl;
	BigInteger big_string("000123456789abcDEF");
	cout << "big_string = " << big_string << endl;
}

//---------------------------------------------------------------
/*
 * Some testcases to test add and addAsm
 */
void testAdd()
{
	cout << endl << "Running testAddition()" << endl;
    cout << endl;
    
	// Testing the addition algorithm
	BigInteger big12345678(0x12345678);
	cout << "0x12345678 + 0x00000001 = " << big12345678.addc(BIG1) << endl;

	BigInteger big_msb(0x80000000);
	cout << "0x80000000 + 0x80000000 = " << big_msb.addc(big_msb) << endl;
    
    BigInteger big_anzi(0x12345678);
	BigInteger big_max(0xFFFFFFFF);
	cout << "0xFFFFFFFF + 0x12345678 = " << big_max.adda(big_anzi) << endl;

	BigInteger big_val1("A5F05A0FA5F05A0F");
	BigInteger big_val2("5A0FA5F05A0FA5F1");
	cout << "A5F05A0FA5F05A0F + 5A0FA5F05A0FA5F1 = " << big_val1.addc(big_val2) << endl;
}

//---------------------------------------------------------------
/*
 * Some testcases to test modp192
 */
void testModp192()
{
	cout << endl << "Running testModp192()" << endl;
	// insert your testcases here!
	//bfb0ea3d5bf9cef301ea608e20ab8f6fe601f02123323d59ffffffff00000001ffffffff111111110000ffffffffffff
    //ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
    //ffffffff11111111ffffffff11111111ffffffff11111111ffffffff11111111ffffffff11111111ffffffffffffffff
    //65e635321cb6aa3de6a9704125856bd5c46e4c531f0f3c420bd650e6a48e90b672f57708def13a9dc172d66da06783d9
    //eeeeeeeeffffffffeeeeeeeeffffffffeeeeeeeeffffffffeeeeeeeeffffffffeeeeeeeeffffffffeeeeeeeeffffffff
    //90a54935dbc97516efad8ae7de72a15290f44f19545dfcdbd5e058757b862406b05ac89c8075988ff5a302d3bfaf9665
    //ffffffffffffffffffffffffffffffffffffffffffffffff000000000000000000000000000000000000000000000000
    //f1111111f1111111f1111111f1111111f1111111f1111111f1111111f1111111f1111111f1111111f1111111f1111111
    //0fffffff0fffffff0fffffff0fffffff0fffffff0fffffff0fffffff0fffffff0fffffff0fffffff0fffffff0fffffff
    //9c96882ee787d2ffd1d5203b37ac0f93be00fc4cc384b000048720f98635133048f2dcb0ea813410ec470b15b6827bbf
    
    cout << endl;
	cout << "v1 = FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF4444444444444445678887543890789AB0CEFFFFFFFFFFFFFFFFFFFFFFF" << endl;
	
    BigInteger v1("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF4444444444444445678887543890789AB0CEFFFFFFFFFFFFFFFFFFFFFFF");
	cout << "v1 mod p192 = " << v1.modp192a() << endl;
    
}

//---------------------------------------------------------------
/*
 * Main function to start the application
 */
int main(int argc, char* argv[])
{
	cout << std::hex;
	cout << "Rechnernetze und Organisation - Assignment A1" << endl;

	// Running some "built in" tests
	printConstants();
	testAdd();

	// Running your own test cases
	testModp192();
    getchar();
	return 0;
}
