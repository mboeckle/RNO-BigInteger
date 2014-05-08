#include "BigInteger.h"

#include <iostream>
	using std::cout;
	using std::cin;
	using std::endl;

// Some constants
const char HexDigits[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

//---------------------------------------------------------------
/*
 * Constructor
 * Default constructor zeroes out the value array
 */
BigInteger::BigInteger(void)
{	// Initialize the BigInteger with value to 0
	for (int i=0; i<BIG_INTEGER_MAX_WORDS; i++)
		value[i] = 0;
}

/*
 * Constructor: from (long) Integer
 * zeroes out the value array
 * @param val is set to the first position in the value array
 */
BigInteger::BigInteger(bigIntType val)
{	// Initialize the BigInteger with the given long value
	for (int i=1; i<BIG_INTEGER_MAX_WORDS; i++)
		value[i] = 0;
	value[0] = val;
}

/*
 * Constructor
 * A string is taken to initialize the instance. Every character
 * gets converted to the corresponding hexadecimal value. Then the
 * intermediate value gets shifted and the hexadecimal value is
 * added. After BIG_INTEGER_NUM_NIBBELS characters a word is finished.
 * So every word of the value array is shifted and and the current
 * word is inserted at position zero.
 * @param str the string to initialize the BigInteger instance
 */
BigInteger::BigInteger(const char str[])
{
	int i;
	for (i=0; i<BIG_INTEGER_MAX_WORDS; i++)
		value[i] = 0;

	char c;								// actual character to analyze
	i = 1;								// c is the i'th character
	bigIntType word = 0;				// actual word
	while (*str > 0) {					// process string character-wise
		c = *str & 0x0f;
		if (*str > 0x40) c += 0x09;		// Connvert 'A'-'F' and 'a'-'f' correctly
		word = (word << 4) + c;			// add character (nibble) to aligned word
		if ((i % 8) == 0) {             // whole 32-bit word processed
			for (int j=BIG_INTEGER_MAX_WORDS-1; j>0; j--)
				value[j] = value[j-1];	// move value 32-bits to the left
			value[0] = word;			// insert read-in word at least-significant position
			i = 0;
			word = 0;
		}
		i++;
		str++;
	}
	// Consider the last word that is propably not word aligned
	shiftLeft((i-1)<<2);
	value[0] |= word;
}

/*
 * Default destructor
 */
BigInteger::~BigInteger(void)
{	// Nothing to be done here
}

//---------------------------------------------------------------
/*
 * adds a given BigInteger to *this
 * The value array is processed word by word. Every step a word
 * of the given BigInteger is added to the corresponding word of
 * *this. If the result of the addition is smaller than the word
 * an overflow occured.
 * The carry of the last step is added to *this. If the carry is
 * greater than the result of the addition an overflow occured.
 * If an overflow occured, the carry is stored to be
 * added in the next step.
 * @param addend a BigInteger to be added to *this
 */
BigInteger& BigInteger::addc(const BigInteger &addend)
{	// Multi-precision addition: carry handling is important
	bigIntType carry_next, carry = 0;
	for (int i=0; i<BIG_INTEGER_MAX_WORDS; i++) 
    {
		carry_next = addend.value[i];				// prevent errors if addend = *this
        value[i] += addend.value[i];
		carry_next = value[i] < carry_next;			// Did addition produce an overflow?
	    value[i] += carry;							// Add carry from last iteration
		carry = carry_next || (value[i] < carry);	// Did addition of carry produce an overflow?
 	}
	return *this;
}

/*
 * Adds a given BigInteger to *this.
 * Code is written in Assembler.
 * Registers gets initialized, carry gets cleared. Word by word
 * the value of *this gets loaded to EBX. The corresponding value
 * of the given BigInteger is added as well as the carry.
 * Register usage:
 *	ECX .... loop counter
 *	EDX .... array index
 *	ESI .... Baseaddresses of Array addend.value[]
 *  EDI .... Baseaddresses of Array this->value[]
 */
BigInteger& BigInteger::adda(const BigInteger &addend)
{	// Multi-precision addition: carry handling is important

	__asm__ __volatile__(
		"XOR  %%edx,%%edx\n\t" 				// start with index 0 AND clear carry!
		"addition_loop:\n\t"
		"MOVL (%%esi, %%edx,4), %%eax\n\t"	// EAX = addend.value[i]
		"ADCL (%%edi, %%edx,4), %%eax\n\t"	// EAX += value[i] + carry
		"MOVL %%eax, (%%edi,%%edx,4)\n\t"	// value[i] = EAX
		"INC  %%edx\n\t"
		"LOOP addition_loop\n\t"			// decrement ECX and LOOP
		: // output variables
		: "S"(&addend), "D"(this), "c"(BIG_INTEGER_MAX_WORDS) // input variables: addend -> esi; this -> edi 
		: "%eax", "%edx", "memory" 			// clobber stuff
	);

	return *this;
}

//---------------------------------------------------------------
/*
 * Compares two BigInteger instances
 * returns true if both instances are equal
 */
bool BigInteger::compare(const BigInteger &testme)
{
	for (int c=0; c < BIG_INTEGER_MAX_WORDS; ++c)
	{
		if (value[c] != testme.value[c])
			return false;
	}
	return true;
}

//---------------------------------------------------------------
/*
 * Modular reduction
 */
BigInteger& BigInteger::modp192c()
{	// Modular reduction mod p192 = 2^192 - 2^64 -1
   
   bigIntType carry_next, carry;
   int temp[] = {0,0,0,0,0,0};
   int temp2[] = {0,0,0,0,0,0};
   int field[] = {6,10,7,11,8,6,10,9,7,11,10,8,11,9};
   int x=0, y=0;
   
   for(x=0, y=0; y<14; y=y+2, x++)
   {
        carry_next = value[field[y]];
        value[x] = value[x] + value[field[y]];
        carry_next = value[x] < carry_next;
        carry = carry_next || (value[x] < carry);

        if(carry)
          temp[x] = temp[x] + 1;
        
        
        carry_next = value[field[y+1]];
        value[x] = value[x] + value[field[y+1]];
        carry_next = value[x] < carry_next;
        carry = carry_next || (value[x] < carry);
        
        if(carry)
          temp[x] = temp[x] + 1;
          
        
        if(x==2 || x==3)
        {
            carry_next = value[field[y+2]];
            value[x] = value[x] + value[field[y+2]];
            carry_next = value[x] < carry_next;
            carry = carry_next || (value[x] < carry);
            
            if(carry)
            {
             temp[x] = temp[x] + 1;
            }
            
            y = y + 1;
        }//end.if
        
        
        if(x > 0)
        {
         value[x] = value [x] + temp [x-1];
        }
        cout << "temp " <<x << " = " << temp[x] << endl;
   }//end.for
   
   
   if(temp[5] > 0)                       // wenn nach der ersten Reduktion 
   {                                     // noch 193 Bits übrig bleiben
    for(x=0; x<=5; x++)
    {
     carry_next = value[x];
     
       if(x==0)
         value[0] = value[0] + temp[5];
       
       else if(x==2)
         value[2] = value[2] + temp[5] + temp2[1];
       
       else
       {
         value[x] = value[x] + temp2[x-1];
       }
       
         carry_next = value[x] < carry_next;
         carry = carry_next || (value[x] < carry);
     
         if(carry)
           temp2[x] = temp2[x] + 1;
    
    }//end.for
     
     if(temp2[5] > 0)                    // wenn nach der Übertragsreduktion 
     {                                   // wieder 1 BIT übrig bleibt
       value[0] = value[0] + temp2[5];
       value[2] = value[2] + temp2[5];
     }//end.if
     
       
   }//end.if
       
   for(int i = 6; i < 12; i++)           //setze value[6->11] = 0;
   {
     value[i] = 0;    
   }    
   return *this;

}//end.modpc192

//---------------------------------------------------------------

BigInteger& BigInteger::modp192a()
{	// Modular reduction mod p192 = 2192 - 264 -1

    __asm__ __volatile__(
		"MOV  %0,	%%esi\n\t"                   //Reg. esi mit 0 initialisieren
 	    "MOV  %1,	%%edi\n\t"                   //Reg. edi mit 0 initialisieren

//------------------------------------------------------------------------------
        //value[0]
		"XOR  %%ebx, %%ebx\n\t"                  //beginne b. Ind. 0 
        "XOR  %%edx,%%edx\n\t"			         //beginne b. Ind. 0
		"MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[0] -> eax
        "MOV $0xA, %%edx\n\t"                    //Wert 10(hex) -> edx
		"ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[10] + value[0]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0x6, %%edx\n\t"                    //Wert 6(hex) -> edx
        "ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[6] zu eax
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "XOR  %%edx,%%edx\n\t"                   //rücksetzen des Index auf 0
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[0] = EAX
		
//------------------------------------------------------------------------------
        //value[1]
        "MOV $0x1, %%edx\n\t"                    //Wert 1(hex) -> edx
		"MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[1] -> eax
        "MOVL %%ebx, %%edx\n\t"                  //Anzahl der Carries -> edx
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Übertrag mit 0 init.
        "ADDL %%edx, %%eax\n\t"                  //addiere uebertrag v. value[0]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0x7, %%edx\n\t"                    //Wert 7(hex) -> edx
		"ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[1] + value[7]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0xB, %%edx\n\t"                    //Wert 11(hex) -> edx
        "ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[11] zu eax
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x1, %%edx\n\t"                    //Wert 1(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[1] = EAX

//------------------------------------------------------------------------------
        //value[2]
        "MOV $0x2, %%edx\n\t"                    //Wert 2(hex) -> edx
		"MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[2] -> eax
        "MOVL %%ebx, %%edx\n\t"                  //Anzahl der Carries -> edx
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADDL %%edx, %%eax\n\t"                  //addiere uebertrag v. value[1]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0x8, %%edx\n\t"                    //Wert 8(hex) -> edx
		"ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[2] + value[8]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0x6, %%edx\n\t"                    //Wert 6(hex) -> edx
        "ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[6] zu eax
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0xa, %%edx\n\t"                    //Wert 10(hex) -> edx
        "ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[10] zu eax
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x2, %%edx\n\t"                    //Wert 2(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[2] = EAX        

//------------------------------------------------------------------------------
        //value[3]
        "MOV $0x3, %%edx\n\t"                    //Wert 3(hex) -> edx
		"MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[3] -> eax
        "MOVL %%ebx, %%edx\n\t"                  //Anzahl der Carries -> edx
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADDL %%edx, %%eax\n\t"                  //addiere uebertrag v. value[2]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0x9, %%edx\n\t"                    //Wert 9(hex) -> edx
		"ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[3] + value[9]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0x7, %%edx\n\t"                    //Wert 6(hex) -> edx
        "ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[7] zu eax
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0xb, %%edx\n\t"                    //Wert 11(hex) -> edx
        "ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[11] zu eax
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x3, %%edx\n\t"                    //Wert 3(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[3] = EAX     
        
//------------------------------------------------------------------------------
        //value[4]
        "MOV $0x4, %%edx\n\t"                    //Wert 4(hex) -> edx
		"MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[4] -> eax
        "MOVL %%ebx, %%edx\n\t"                  //Anzahl der Carries -> edx
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADDL %%edx, %%eax\n\t"                  //addiere uebertrag v. value[3]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0xa, %%edx\n\t"                    //Wert 10(hex) -> edx
		"ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[4] + value[10]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0x8, %%edx\n\t"                    //Wert 8(hex) -> edx
        "ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[8] zu eax
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x4, %%edx\n\t"                    //Wert 4(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[4] = EAX
                               
//------------------------------------------------------------------------------
        //value[5]
        "MOV $0x5, %%edx\n\t"                    //Wert 5(hex) -> edx
		"MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[4] -> eax
        "MOVL %%ebx, %%edx\n\t"                  //Anzahl der Carries -> edx
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADDL %%edx, %%eax\n\t"                  //addiere uebertrag v. value[4]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0xb, %%edx\n\t"                    //Wert 11(hex) -> edx
		"ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[5] + value[11]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "MOV $0x9, %%edx\n\t"                    //Wert 9(hex) -> edx
        "ADDL (%%esi, %%edx, 4), %%eax\n\t"      //addiere value[9] zu eax
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x5, %%edx\n\t"                    //Wert 5(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[5] = EAX
                               
//------------------------------------------------------------------------------
        //wenn nach der ersten Reduktion 193 Bits übrig bleiben
     

        "XOR %%edx, %%edx\n\t"                   //rücksetzen des Index auf 0
        "MOV $0x0, %%edx\n\t"                    //Wert 0(hex) -> edx
        "MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[0] -> eax
        "ADDL %%ebx, %%eax\n\t"                  //addiere uebertrag v. value[5]
        "PUSH %%ebx\n\t"                         //Legt ebx (den Übertrag von
                                                 //value[5]) auf denStack
        
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx          
        //move to destination
        "MOV $0x0, %%edx\n\t"                    //Wert 0(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[0] = EAX  
        
        
        "MOV $0x1, %%edx\n\t"                    //Wert 1(hex) -> edx
        "MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[1] -> eax
        "ADDL %%ebx, %%eax\n\t"                  //addiere uebertrag v. value[0]
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x1, %%edx\n\t"                    //Wert 1(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[1] = EAX  
        
        
        "MOV $0x2, %%edx\n\t"                    //Wert 2(hex) -> edx
        "MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[2] -> eax
        "ADDL %%ebx, %%eax\n\t"                  //addiere uebertrag v. value[1]
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "POP %%edx\n\t"                          //Hole Übertrag von value[5] (
                                                 //von Durchgang 1) vom Stack
        "ADDL %%edx, %%eax\n\t"                  //addiere uebertrag v. value[5]
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        "XOR %%edx, %%edx\n\t"                   //rücksetzen des Index auf 0
        //move to destination
        "MOV $0x2, %%edx\n\t"                    //Wert 2(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[2] = EAX
        
        
        "MOV $0x3, %%edx\n\t"                    //Wert 3(hex) -> edx
        "MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[3] -> eax
        "ADDL %%ebx, %%eax\n\t"                  //addiere uebertrag v. value[2]
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x3, %%edx\n\t"                    //Wert 3(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[3] = EAX
        
        
        "MOV $0x4, %%edx\n\t"                    //Wert 4(hex) -> edx
        "MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[4] -> eax
        "ADDL %%ebx, %%eax\n\t"                  //addiere uebertrag v. value[3]
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x4, %%edx\n\t"                    //Wert 4(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[4] = EAX
        
        
        "MOV $0x5, %%edx\n\t"                    //Wert 5(hex) -> edx
        "MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[5] -> eax
        "ADDL %%ebx, %%eax\n\t"                  //addiere uebertrag v. value[4]
        "XOR %%ebx, %%ebx\n\t"                   //Reg. für Carries mit 0 init.
        "ADCL $0x0, %%ebx\n\t"                   //Anzahl der Überträge nach ebx
        //move to destination
        "MOV $0x5, %%edx\n\t"                    //Wert 5(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[5] = EAX
        
//------------------------------------------------------------------------------
        //wenn nach der Uebertragsreduktion noch immer 193 Bits übrig bleiben
        
        "XOR %%edx, %%edx\n\t"                   //Reg. für Carries mit 0 init.
        "MOV $0x0, %%edx\n\t"                    //Wert 0(hex) -> edx
        "MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[0] -> eax
        "ADDL %%ebx, %%eax\n\t"                  //addiere uebertrag v. value[5]
        //move to destination
        "MOV $0x0, %%edx\n\t"                    //Wert 0(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[0] = EAX  
        
        "MOV $0x2, %%edx\n\t"                    //Wert 2(hex) -> edx
        "MOVL (%%esi, %%edx, 4), %%eax\n\t"      //value[2] -> eax
        "ADDL %%ebx, %%eax\n\t"                  //addiere uebertrag v. value[5]
        //move to destination
        "MOV $0x2, %%edx\n\t"                    //Wert 2(hex) -> edx
        "MOVL %%eax,	(%%edi,%%edx,4)\n\t"     //value[2] = EAX  
//------------------------------------------------------------------------------
        //setze value[6->11] = 0;
        "XOR  %%eax,%%eax\n\t"                   //beginne b. Ind. 0 
        "MOV $0x6, %%edx\n\t"                    //Wert 6(hex) -> edx
        "MOVL %%eax, (%%edi,%%edx,4)\n\t"        //value[6] = EAX = 0

        "MOV $0x7, %%edx\n\t"                    //Wert 7(hex) -> edx
        "MOVL %%eax, (%%edi,%%edx,4)\n\t"        //value[7] = EAX = 0

        "MOV $0x8, %%edx\n\t"                    //Wert 8(hex) -> edx
        "MOVL %%eax, (%%edi,%%edx,4)\n\t"        //value[8] = EAX = 0

        "MOV $0x9, %%edx\n\t"                    //Wert 9(hex) -> edx
        "MOVL %%eax, (%%edi,%%edx,4)\n\t"        //value[9] = EAX =0

        "MOV $0xa, %%edx\n\t"                    //Wert 10(hex) -> edx
        "MOVL %%eax, (%%edi,%%edx,4)\n\t"        //value[10] = EAX = 0

        "MOV $0xb, %%edx\n\t"                    //Wert 11(hex) -> edx
        "MOVL %%eax, (%%edi,%%edx,4)\n\t"        //value[11] = EAX = 0
        
        :								             //output variables
		: "a"(value), "c"(this)			             // input variables
		: "%esi", "%edi", "%edx", "%ebx", "memory"	 // clobber stuff
	);
        return *this;
}
//---------------------------------------------------------------
/*
 * shifts *this val times to the left.
 * If val is greater than 32 bits the value array can
 * be shifted word_wise. The lower words are set to zero.
 *
 * A loop walks through all words. The resulting carry of the shift
 * is calculated by shifting 32 bits minus val to the
 * opposite side. The word gets shifted by val positions and the carry
 * of the last step is added by a logical or.
 * @param val defines shifting dimension.
 */
BigInteger& BigInteger::shiftLeft(unsigned int val)
{	// Multi-precision shift-left
	// 1. word-level shifting
	int j;
	int words_to_shift = val / 32;
	words_to_shift = (words_to_shift > BIG_INTEGER_MAX_WORDS) ? BIG_INTEGER_MAX_WORDS : words_to_shift;
	for (j=BIG_INTEGER_MAX_WORDS-1; j>=words_to_shift; j--)
		value[j] = value[j-words_to_shift];	// do the word-wise shift
	for (j=words_to_shift-1; j>=0; j--)
		value[j] = 0;							// fill lower words with 0
	// 2. bit-level shifting
	int number_of_bits_to_shift = val % 32;
	if (number_of_bits_to_shift > 0) {
		int bits_to_shift_carry = 32 - number_of_bits_to_shift;
		bigIntType carry_next, carry = 0;
		for (j=0; j<BIG_INTEGER_MAX_WORDS; j++) {
			carry_next = value[j] >> bits_to_shift_carry;				// highest bits are moved to next word
			value[j] =  (value[j] << number_of_bits_to_shift) | carry;	// do the shift and considered bits are moved out of lower word
			carry = carry_next;
		}
	}
	return *this;
}

//---------------------------------------------------------------
/*
 * controls the output of a BigInteger instance
 * The value array is processed word-wise starting with the most
 * significant one. Every word gets parted into nibbles and set sent
 * to the output.
 */
ostream& operator << (ostream& outstr, const BigInteger &bigint)
{	// Output the BigInteger.value in hexadecimal format to the output stream
	for (int i=BIG_INTEGER_MAX_WORDS-1; i>=0; i--) { // start with highest word and count down
		for (int j=8-1; j>=0; j--) { // count down nibbles of each word
				outstr << HexDigits[(bigint.value[i] >> (j << 2)) & 0x0f];	// output nibble
		}
	}
	return outstr;
}
