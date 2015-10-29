#if __GNUC__ > 3
 #include <iostream>
#else
 #include <iostream.h>
#endif

#ifndef BIGNUM_H
#define BIGNUM_H


class BigNum
{
  friend BigNum Set_ui (unsigned long int);
  friend BigNum Set_si (signed long int);
  
  friend BigNum operator + (const BigNum &, const BigNum &);
  friend BigNum operator + (const BigNum &, const unsigned long int);
  friend BigNum operator - (const BigNum &, const BigNum &);
  friend BigNum Sub4Div (const BigNum &, const BigNum &);
  friend BigNum operator - (const BigNum &, const unsigned long int);
  friend BigNum operator * (const BigNum &, const BigNum &);
  friend BigNum operator * (const BigNum &, const unsigned long int);
  friend BigNum operator / (const BigNum &, const BigNum &);
  friend BigNum operator / (const BigNum &, const unsigned long int);
  friend BigNum operator % (const BigNum &, const BigNum &);
  friend BigNum operator % (const BigNum &, const unsigned long int);
  friend BigNum operator ^ (const BigNum &, const BigNum &);
  friend BigNum operator ^ (const BigNum &, const unsigned long int);
  friend unsigned long int operator & (const BigNum &, const unsigned long int);
  friend BigNum operator >> (BigNum &, unsigned long int);
  friend BigNum operator << (BigNum &, unsigned long int);
  friend BigNum b_div_2exp (const BigNum &, const unsigned long int);
  friend BigNum b_pow (const BigNum &, const unsigned long int);
  friend BigNum b_powm (const BigNum &, const BigNum &, const BigNum &);
  friend BigNum b_powm (const BigNum &, const unsigned int, const BigNum &);
  friend BigNum b_abs(const BigNum &);
  friend BigNum b_abs2(const BigNum &);
  friend BigNum b_neg(const BigNum &);
  friend std::ostream& operator << (std::ostream &, const BigNum &); 
 
  friend bool operator == (const BigNum &, const BigNum &);
  friend bool operator == (const BigNum &, const unsigned long int);
  friend int b_cmp (const BigNum &, const BigNum &);
  friend int b_cmp (const BigNum &, const unsigned long int);
  friend int b_cmp (const BigNum &, const signed long int);
  
  friend unsigned long int SmallerSize (const BigNum &, const BigNum &);
  friend unsigned long int BiggerSize (const BigNum &, const BigNum &);
  friend int Test (const unsigned long int, int);
  friend unsigned long int Set (const unsigned long int, int);
  friend unsigned long int Unset (const unsigned long int, int);
  friend unsigned long int CeilDiv (unsigned long int, unsigned long int);
  friend int GetNumBits (unsigned long int);

  friend int C2I(char c);

public:
  BigNum();
  BigNum(unsigned long int);
  BigNum(char *, char s = '+');
  BigNum(const BigNum &);
  BigNum& operator= (const BigNum &);
  ~BigNum();
       
  unsigned long int b_get_ui() const;
  unsigned long int Size() const;
  unsigned long int * V() const;
  char Sign() const;
  BigNum EraseLeadingBits(unsigned long int);
  unsigned long int GetTotalBits() const;
  void b_clear();
  void b_print_w_sign();
  
  //private:
  unsigned long int * v;
  unsigned long int size;    
  char sign;
};

#endif

#if (__GNUC__ > 3)

BigNum Set_ui (unsigned long int);
BigNum Set_si (signed long int);

BigNum operator + (const BigNum &, const BigNum &);
BigNum operator + (const BigNum &, const unsigned long int);
BigNum operator - (const BigNum &, const BigNum &);
BigNum Sub4Div (const BigNum &, const BigNum &);
BigNum operator - (const BigNum &, const unsigned long int);
BigNum operator * (const BigNum &, const BigNum &);
BigNum operator * (const BigNum &, const unsigned long int);
BigNum operator / (const BigNum &, const BigNum &);
BigNum operator / (const BigNum &, const unsigned long int);
BigNum operator % (const BigNum &, const BigNum &);
BigNum operator % (const BigNum &, const unsigned long int);
BigNum operator ^ (const BigNum &, const BigNum &);
BigNum operator ^ (const BigNum &, const unsigned long int);
unsigned long int operator & (const BigNum &,
			      const unsigned long int);
BigNum operator >> (BigNum &, unsigned long int);
BigNum operator << (BigNum &, unsigned long int);
BigNum b_div_2exp (const BigNum &, const unsigned long int);
BigNum b_pow (const BigNum &, const unsigned long int);
BigNum b_powm (const BigNum &, const BigNum &,
	       const BigNum &);
BigNum b_powm (const BigNum &, const unsigned int,
	       const BigNum &);
BigNum b_abs (const BigNum &);
BigNum b_abs2 (const BigNum &);
BigNum b_neg (const BigNum &);
std::ostream & operator << (std::ostream &, const BigNum &);

bool operator == (const BigNum &, const BigNum &);
bool operator == (const BigNum &, const unsigned long int);
int b_cmp (const BigNum &, const BigNum &);
int b_cmp (const BigNum &, const unsigned long int);
int b_cmp (const BigNum &, const signed long int);

unsigned long int SmallerSize (const BigNum &,
			       const BigNum &);
unsigned long int BiggerSize (const BigNum &,
			      const BigNum &);
int Test (const unsigned long int, int);
unsigned long int Set (const unsigned long int, int);
unsigned long int Unset (const unsigned long int, int);
unsigned long int CeilDiv (unsigned long int,
			   unsigned long int);
int GetNumBits (unsigned long int);

int C2I (char c);

#endif
