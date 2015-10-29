/*************************************************************************/
/*************************************************************************/
/* Implementation file for arithmetic on large integers                  */
/*                                                                       */ 
/* Author: J. Ren,                                                       */
/*            Florida State Unversity                                    */
/* E-Mail: ren@csit.fsu.edu                                              */
/*                                                                       */ 
/*************************************************************************/
/*************************************************************************/


#if __GNUC__ > 3
 #include <string.h>
 #include <iostream>
#else
 #include <iostream.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#if __GNUC__ > 3
 #include <iomanip>
#else
 #include <iomanip.h>
#endif
#include <ctype.h>
#include "bignum.h"


#ifdef LONG64
#define NUMBITS 64

#else
#define NUMBITS 32

#endif

using namespace std;

/* default constructor */
BigNum::BigNum() 
{
  size = 1;
  v = new unsigned long int[size];
  *v = 0;
  sign = '+';
}                

/* constructor by size */                
BigNum::BigNum(unsigned long int s) 
{
  size = s;
  sign = '+';
  v = new unsigned long int[s];

  for (unsigned long int i = 0; i < s; i++)
    v[i] = 0;
}

BigNum::BigNum(char * c, char s)
{
  unsigned long int hexdigits = strlen(c);
  unsigned long int index;
  int n = NUMBITS / 4;
  int mod = hexdigits % n;
  int i, temp;

  sign = s;

  if (mod == 0)
    size = hexdigits / n;
  else
    size = hexdigits / n + 1;

  v = new unsigned long int[size];

  if (mod > 0) {
    index = size - 1;
    v[index] = 0;

    temp = mod * 4 - 4;

    for (i = temp; i >= 0; i-=4) 
      v[index] = v[index] ^ (static_cast<unsigned long int>(C2I(*c++)) << static_cast<unsigned long int>(i));

    index = size - 1;
  }
  else
    index = size;

  for (unsigned long int j = index; j > 0; j--) {
    temp = j - 1;

    v[temp] = 0;

    for (i = NUMBITS - 4; i >= 0; i-= 4) 
      v[temp] = v[temp] ^ (static_cast<unsigned long int>(C2I(*c++)) << static_cast<unsigned long int>(i));
  }

}

/* Copy constructor */
BigNum::BigNum(const BigNum &bn)
{
  size = bn.size;
  sign = bn.sign;
  v = new unsigned long int[size];

  if (v == 0) {
    cerr << "\nbn memory allocation failure\n";
    exit(EXIT_FAILURE);
  }

  for (unsigned long int i = 0; i < size; i++)
    v[i] = bn.v[i];
}

/* Assignment operator */
BigNum& BigNum::operator= (const BigNum &bn)
{
  if (this != &bn) {
    if (size != bn.size) {
      if (v != NULL)
      	delete [] v;
      	
      size = bn.size;
      sign = bn.sign;
      v = new unsigned long int[size];
      
      if (v == 0) {
	    cerr << "\nbn memory allocation failure\n";
	    exit(EXIT_FAILURE);
      }
    }

    for (unsigned long int i = 0; i < size; i++)
      v[i] = bn.v[i];
  }

  return *this;
}

BigNum Set_ui(unsigned long int n)
{
  BigNum bn;
   
  *(bn.v) = n;
  
  return bn;
}

BigNum Set_si(signed long int n)
{
  BigNum bn;

  bn.v = new unsigned long int[1];
  
  if (n < 0) {
    bn.sign = '-';
    *(bn.v) = 0 - n;
  }
  else 
    *(bn.v) = n;
    
  return bn;
}

BigNum::~BigNum()
{
  if  (v)
    delete [] v;
}

void BigNum::b_clear()
{
  if (v) {
    delete [] v;
    v = 0;
    size = 0;
    sign = 0;
  }  
}

BigNum operator + (const BigNum & x, const BigNum & y)
{
  unsigned long int * xptr, * yptr, * zptr;
  unsigned long int zsize;
  unsigned long int temp, andtemp, ztemp;
  unsigned long int carry = 0;
  unsigned long int smallersize = SmallerSize(x, y);
  unsigned long int biggersize = BiggerSize(x, y);
  unsigned long int i;
  BigNum zero;

  if (x.sign == '-') {
    if (y.sign == '-')
      return b_neg(b_abs(x) + b_abs(y));
    else if (y.sign == '+') {
      if (b_cmp(b_abs(x), y) < 0)
	return y - b_abs(x);
      else if (b_cmp(b_abs(x), y) > 0)
	return b_neg(b_abs(x) - y);
      else if (b_cmp(b_abs(x), y) == 0)
	return zero;
    }  
  }
  else if (x.sign == '+' && y.sign == '-') {
    if (b_cmp(x, b_abs(y)) < 0)
      return b_neg(b_abs(y) - x);
    else if (b_cmp(x, b_abs(y)) > 0)
      return x - b_abs(y);
    else if (b_cmp(x, b_abs(y)) == 0)
      return zero; 
  }

  if (x.size > y.size)
    zsize = x.size + 1;
  else
    zsize = y.size + 1;

  BigNum z(zsize);
  xptr = x.v;
  yptr = y.v;
  zptr = z.v;

  for (i = 0; i < smallersize; i++) {
    ztemp = *zptr;
    *zptr ^= *xptr ^ *yptr;
    andtemp = *xptr & *yptr | *xptr & ztemp | *yptr & ztemp;

    if ((Test(*xptr, NUMBITS - 1) == 1) && (Test(*yptr, NUMBITS - 1) == 1)) {
      *zptr = Set(*(++zptr), 0);
      *(zptr--);
    }

    carry = andtemp << 1;

    do {
      andtemp = carry & *zptr;

      if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 1)) {
	*zptr = Set(*(++zptr), 0);
	*(zptr--);
      }

      temp = andtemp << 1;
      *zptr ^= carry;
      carry = temp;
    } while (temp != 0);

    *(++xptr);
    *(++yptr);
    *(++zptr);
  }

  if (x.size != y.size) {
    if (*zptr == 0) {
      if (smallersize == x.size) {
	while (i < z.size - 1) {
	  *(zptr++) = *(yptr++);
	  i++;
	}
      }
      else {
	while (i < z.size - 1) {
	  *(zptr++) = *(xptr++);
	  i++;
	}
      }
    }
    else {
      if (smallersize == x.size) {
	for (i = smallersize; i < biggersize; i++) {
	  ztemp = *zptr;
	  *zptr ^= *yptr;
	  andtemp = *yptr & ztemp;
	  carry = andtemp << 1;

	  do {
	    andtemp = carry & *zptr;

	    if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 1)) {
	      *zptr = Set(*(++zptr), 0);
	      *(zptr--);
	    }

	    temp = andtemp << 1;
	    *zptr ^= carry;
	    carry = temp;
	  } while (temp != 0);

	  *(++yptr);
	  *(++zptr);
	}
      }
      else {
	for (i = smallersize; i < biggersize; i++) {
	  ztemp = *zptr;
	  *zptr ^= *xptr;
	  andtemp = *xptr & ztemp;
	  carry = andtemp << 1;

	  do {
	    andtemp = carry & *zptr;

	    if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 1)) {
	      *zptr = Set(*(++zptr), 0);
	      *(zptr--);
	    }

	    temp = andtemp << 1;
	    *zptr ^= carry;
	    carry = temp;
	  } while (temp != 0);

	  *(++xptr);
	  *(++zptr);
	}
      }
    }
  }

  if (*zptr == 0) 
    z.size--;

  zero.b_clear();

  return z;
}

BigNum operator + (const BigNum & x, const unsigned long int y)
{
  BigNum bn_y = Set_ui(y);

  return x + bn_y;
}

BigNum operator - (const BigNum & x, const BigNum & y)
{
  unsigned long int *xptr, *yptr, *zptr;
  unsigned long int i;
  unsigned long int temp, andtemp, ztemp;
  unsigned long int carry = 0;
  unsigned long int biggersize = BiggerSize(x, y);
  unsigned long int smallersize = SmallerSize(x, y);
  BigNum zero;

  if (x.sign == '+' && y.sign == '+' && b_cmp(x, y) == -1)
    return b_neg(y - x);
  else if ((x.sign == '-' && y.sign == '-') || (x.sign == '+' && y.sign == '-'))
    return x + b_neg(y);
  else if (x.sign == '-' && y.sign == '+')
    return b_neg(b_abs(x) + y);

  BigNum z(biggersize + 1);
  xptr = x.v;
  yptr = y.v;
  zptr = z.v;

  for (i = 0; i < smallersize; i++) {
    ztemp = *zptr;
    *zptr ^= *xptr ^ *yptr;
    andtemp = (*yptr & *zptr) | (ztemp == 1 && Test(*xptr, 0) == 0 &&
				 (Test(*yptr, 0) == 0 | Test(*yptr, 0) == 1));

    if ((Test(*xptr, NUMBITS - 1) == 0) && (Test(*yptr, NUMBITS - 1) == 1)) {
      *zptr = Set(*(++zptr), 0);
      *(zptr--);
    }

    carry = andtemp << 1;

    do {
      andtemp = (carry ^ *zptr) & carry;

      if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 0)) {
	*zptr = Set(*(++zptr), 0);
	*(zptr--);
      }

      temp = andtemp << 1;
      *zptr ^= carry;
      carry = temp;
    } while (temp != 0);

    *(++xptr);
    *(++yptr);
    *(++zptr);
  }

  if (x.size != y.size) {
    if (*zptr == 0 && smallersize == y.size) {
      while (i < biggersize) {
	i++;
	*(zptr++) = *(xptr++);
      }
    }
    else {
      if (smallersize == x.size) {
	for (i = smallersize; i < biggersize; i++) {
	  ztemp = *zptr;
	  *zptr ^= *yptr;
	  andtemp = (*yptr & *zptr) | (ztemp == 1 &&
				       (Test(*yptr, 0) == 0 | Test(*yptr, 0) == 1));

	  if (Test(*yptr, NUMBITS - 1) == 1) {
	    *zptr = Set(*(++zptr), 0);
	    *(zptr--);
	  }

	  carry = andtemp << 1;

	  do {
	    andtemp = (carry ^ *zptr) & carry;

	    if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 0)) {
	      *zptr = Set(*(++zptr), 0);
	      *(zptr--);
	    }

	    temp = andtemp << 1;
	    *zptr ^= carry;
	    carry = temp;
	  } while (temp != 0);

	  *(++yptr);
	  *(++zptr);
	}
      }
      else {
	for (i = smallersize; i < biggersize; i++) {
	  ztemp = *zptr;
	  *zptr ^= *xptr;
	  andtemp = (ztemp == 1 && Test(*xptr, 0) == 0);
	  carry = andtemp << 1;

	  do {
	    andtemp = (carry ^ *zptr) & carry;

	    if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 0)) {
	      *zptr = Set(*(++zptr), 0);
	      *(zptr--);
	    }

	    temp = andtemp << 1;
	    *zptr ^= carry;
	    carry = temp;
	  } while (temp != 0);

	  *(++xptr);
	  *(++zptr);
	}
      }
    }
  }

  if (z.size != 1) {
    while (*zptr == 0) {
      *(zptr--);
      z.size--;
    }
  }

  if (z.size == 0) {
    z.size++;
    z.v[0] = 0;
  }

  if ((x.size < y.size) || (x.size == y.size && z.size > x.size))
    z.sign = '-';

  zero.b_clear();

  return z;
}

BigNum Sub4Div (const BigNum & x, const BigNum & y)
{
  unsigned long int *xptr, *yptr, *zptr;
  unsigned long int i;
  unsigned long int temp, andtemp, ztemp;
  unsigned long int carry = 0;
  unsigned long int biggersize = BiggerSize(x, y);
  unsigned long int smallersize = SmallerSize(x, y);
  BigNum zero;
  BigNum z(biggersize + 1);
  xptr = x.v;
  yptr = y.v;
  zptr = z.v;

  for (i = 0; i < smallersize; i++) {
    ztemp = *zptr;
    *zptr ^= *xptr ^ *yptr;
    andtemp = (*yptr & *zptr) | (ztemp == 1 && Test(*xptr, 0) == 0 &&
				 (Test(*yptr, 0) == 0 | Test(*yptr, 0) == 1));

    if ((Test(*xptr, NUMBITS - 1) == 0) && (Test(*yptr, NUMBITS - 1) == 1)) {
      *zptr = Set(*(++zptr), 0);
      *(zptr--);
    }

    carry = andtemp << 1;

    do {
      andtemp = (carry ^ *zptr) & carry;

      if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 0)) {
	*zptr = Set(*(++zptr), 0);
	*(zptr--);
      }

      temp = andtemp << 1;
      *zptr ^= carry;
      carry = temp;
    } while (temp != 0);

    *(++xptr);
    *(++yptr);
    *(++zptr);
  }

  if (x.size != y.size) {
    if (*zptr == 0 && smallersize == y.size) {
      while (i < biggersize) {
	i++;
	*(zptr++) = *(xptr++);
      }
    }
    else {
      if (smallersize == x.size) {
	for (i = smallersize; i < biggersize; i++) {
	  ztemp = *zptr;
	  *zptr ^= *yptr;
	  andtemp = (*yptr & *zptr) | (ztemp == 1 &&
				       (Test(*yptr, 0) == 0 | Test(*yptr, 0) == 1));

	  if (Test(*yptr, NUMBITS - 1) == 1) {
	    *zptr = Set(*(++zptr), 0);
	    *(zptr--);
	  }

	  carry = andtemp << 1;

	  do {
	    andtemp = (carry ^ *zptr) & carry;

	    if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 0)) {
	      *zptr = Set(*(++zptr), 0);
	      *(zptr--);
	    }

	    temp = andtemp << 1;
	    *zptr ^= carry;
	    carry = temp;
	  } while (temp != 0);

	  *(++yptr);
	  *(++zptr);
	}
      }
      else {
	for (i = smallersize; i < biggersize; i++) {
	  ztemp = *zptr;
	  *zptr ^= *xptr;
	  andtemp = (ztemp == 1 && Test(*xptr, 0) == 0);
	  carry = andtemp << 1;

	  do {
	    andtemp = (carry ^ *zptr) & carry;

	    if ((Test(carry, NUMBITS - 1) == 1) && (Test(*zptr, NUMBITS - 1) == 0)) {
	      *zptr = Set(*(++zptr), 0);
	      *(zptr--);
	    }

	    temp = andtemp << 1;
	    *zptr ^= carry;
	    carry = temp;
	  } while (temp != 0);

	  *(++xptr);
	  *(++zptr);
	}
      }
    }
  }

  if (z.size != 1) {
    while (*zptr == 0) {
      *(zptr--);
      z.size--;
    }
  }

  if (z.size == 0) {
    z.size++;
    z.v[0] = 0;
  }

  if ((x.size < y.size) || (x.size == y.size && z.size > x.size))
    z.sign = '-';

  zero.b_clear();

  return z;
}

BigNum operator - (const BigNum & x, const unsigned long int y)
{
  BigNum bn_y = Set_ui(y);

  return x - bn_y;
}

BigNum operator * (const BigNum & x, const BigNum & y)
{
  unsigned long int j, k;
  unsigned long int ysize;
  int i;
  unsigned long int *yptr;
  BigNum z(1);
  BigNum xcopy, ycopy;

  if (x.sign == '-' && y.sign == '-')
    return b_abs(x) * b_abs(y);
  else if (x.sign != y.sign)
    return b_neg(b_abs(x) * b_abs(y)); 

  if (x.size < y.size) {
    xcopy = y;
    ycopy = x;
    yptr = ycopy.v;
    ysize = ycopy.size;
  }
  else {
    xcopy = x;
    yptr = y.v;
    ysize = y.size;
  }

  k = 0;

  for (j = 0; j < ysize; j++) {
    for (i = 0; i < NUMBITS; i++) {
      if (Test(*yptr, i) == 1) {
	if (k != 0) {
	  xcopy = xcopy << k;
	  k = 0;
	}

	z = z + xcopy;
	xcopy = xcopy << 1;
      }
      else
	k++;
    }

    *(yptr++);
  }

  xcopy.b_clear();
  ycopy.b_clear();

  return z;
}

BigNum operator * (const BigNum & x, const unsigned long int y)
{
  BigNum bn_y = Set_ui(y);

  return x * bn_y;
}

BigNum operator / (const BigNum & x, const BigNum & y)
{
  BigNum z;
  BigNum xcopy = x;
  BigNum xtemp, t;
  BigNum difference;
  BigNum zero(1);
  unsigned long int diffbits = 0;
  unsigned long int lostbits;
  unsigned long int tempbits;
  unsigned long int xtempbits;
  unsigned long int shiftbits;
  unsigned long int xbits = x.GetTotalBits();
  unsigned long int ybits = y.GetTotalBits();
  bool prevneg = false;

  if (x.sign == '-' && y.sign == '-')
    return b_abs(x) / b_abs(y);
  else if (x.sign != y.sign)
    return b_neg(b_abs(x) / b_abs(y)); 

  if (y == zero) {
    cerr << "Division by 0 error";
    exit(EXIT_FAILURE);
  }

  if (ybits > xbits)
    return zero;

  z = zero;

  while (true) {
    xbits = xcopy.GetTotalBits();

    if (prevneg == false)
      xtemp = xcopy >> (xbits - ybits);
    else
      xtemp = xcopy >> (xbits - ybits - 1);

    if (xbits < ybits) {
      z = z << (xbits - diffbits);
      break;
    }

    difference = Sub4Div(xtemp, y);
    diffbits = difference.GetTotalBits();
    xtempbits = xtemp.GetTotalBits();
    z = z << shiftbits;

    if (difference == zero) {
      z = z << 1;
      z = z ^ 1;
      shiftbits = ybits - 1;
      tempbits = xbits - ybits;

      if (tempbits == 0)
	break;

      xcopy = xcopy.EraseLeadingBits(ybits);
      lostbits = tempbits - xcopy.GetTotalBits();
      z = z << lostbits;
      prevneg = false;
    }
    else if (difference.sign == '-') {
      z = z << 1;
      tempbits = xbits - ybits;
      shiftbits = 0;

      if (tempbits == 0)
	break;

      prevneg = true;
    }
    else { /* difference is non-zero positive */
      z = z << 1;
      z = z ^ 1;
      tempbits = xbits - ybits;

      if (prevneg == false) {
	if (tempbits == 0)
	  break;

	xcopy = xcopy.EraseLeadingBits(ybits);
	lostbits = tempbits - xcopy.GetTotalBits();
      }
      else {
	if (tempbits - 1 == 0)
	  break;

	xcopy = xcopy.EraseLeadingBits(ybits + 1);
	lostbits = tempbits - xcopy.GetTotalBits() - 1;
      }

      xbits = xcopy.GetTotalBits();

      if (lostbits == 0) {
	t = difference << xbits;
	xcopy = t ^ xcopy;
      }
      else {
	t = difference << (xbits + lostbits);
	xcopy = t ^ xcopy;
      }

      if (ybits == diffbits) {
	shiftbits =  0;
	prevneg = true;
      }
      else {
	shiftbits = ybits - diffbits - 1;
	prevneg = false;
      }
    }
  }

  xtemp.b_clear();
  t.b_clear();
  difference.b_clear();
  zero.b_clear();

  return z;
}

BigNum operator / (const BigNum & x, const unsigned long int y)
{
  BigNum bn_y = Set_ui(y);

  return x / bn_y;
}

BigNum operator % (const BigNum & x, const BigNum & y)
{
  if (x.GetTotalBits() < y.GetTotalBits())
    return x;

  if (x.sign == '+')
    return x - x / y * y;
  else 
    return y - b_abs(x) % y;
}

BigNum operator % (BigNum & x, const unsigned long int y)
{
  BigNum bn_y = Set_ui(y);

  return x % bn_y;
}

BigNum operator ^ (const BigNum & x, const BigNum & y)
{
  unsigned long int zsize;
  unsigned long int biggersize, smallersize;
  unsigned long int *xptr, *yptr, *zptr;
  unsigned long int i;

  biggersize = BiggerSize(x, y);
  smallersize = SmallerSize(x, y);
  zsize = biggersize;
  BigNum z(zsize);
  xptr = x.v;
  yptr = y.v;
  zptr = z.v;

  for (i = 0; i < smallersize; i++)
    *(zptr++) = *(xptr++) ^ *(yptr++);

  if (biggersize == smallersize)
    return z;

  if (biggersize == x.size) {
    for (i = smallersize; i < biggersize; i++)
      *(zptr++) = *(xptr++);

    while (*(zptr--) == 0 && zsize > 1)
      zsize--;
  }
  else
    for (i = smallersize; i < biggersize; i++)
      *(zptr++) = *(yptr++);

  return z;
}

BigNum operator ^ (const BigNum & x, const unsigned long int y)
{
  BigNum z = x;
  *(z.v) ^= y;

  return z;
}

unsigned long int operator & (const BigNum & x, const unsigned long int y)
{
  return *(x.v) & y;
}

BigNum operator >> (BigNum & x, unsigned long int index)
{
  unsigned long int indexmod;
  unsigned long int zsize;
  unsigned long int lastbits;
  unsigned long int bitscopy = 0;
  unsigned long int *xptr, *zptr;
  unsigned long int xbits = x.GetTotalBits();
  unsigned long int shiftamount;
  unsigned long int zfirstbits;
  unsigned long int i;
  BigNum zero(1);

  if (index >= xbits)
    return zero;

  zsize = CeilDiv(xbits - index, NUMBITS);
  indexmod = index % NUMBITS;
  shiftamount = NUMBITS - indexmod;
  BigNum z(zsize);
  zfirstbits = (xbits - index) % NUMBITS;
  xptr = &(x.v[x.size-1]);
  zptr = &(z.v[z.size-1]);

  if (index == 0 || x == zero)
    return x;

  if (indexmod == 0) {
    for (i = 0; i < zsize; i++)
      *(zptr--) = *(xptr--);
  }
  else {
    if (GetNumBits(*xptr) < static_cast<int>(zfirstbits % NUMBITS) || ((xbits - index) % NUMBITS == 0)) {
      bitscopy = *xptr << shiftamount;
      *(xptr--);
    }

    for (i = 0; i < zsize; i++) {
      *zptr = *xptr;
      lastbits = *zptr << shiftamount;
      *zptr >>= indexmod;
      *zptr ^= bitscopy;
      bitscopy = lastbits;
      *(xptr--);
      *(zptr--);
    }
  }

  zero.b_clear();

  return z;
}

BigNum operator << (BigNum & x, unsigned long int index)
{
  BigNum zero(1);

  if (index == 0 || x == zero) {
    return x;
  }

  unsigned long int indexmod;
  unsigned long int zsize;
  unsigned long int amount;
  unsigned long int shiftamount;
  unsigned long int firstbits;
  unsigned long int bitscopy = 0;
  unsigned long int *xptr, *zptr;
  unsigned long int i;
  zsize = CeilDiv(index + x.GetTotalBits(), NUMBITS);
  indexmod = index % NUMBITS;
  amount = (index - indexmod) / NUMBITS;
  shiftamount = NUMBITS - indexmod;
  BigNum z(zsize);

  xptr = x.v;
  zptr = z.v;

  for (i = 0; i < amount; i++)
    *(zptr++) = 0;

  if (indexmod == 0) {
    for (i = 0; i < x.size; i++)
      *(zptr++) = *(xptr++);
  }
  else {
    for (i = 0; i < x.size; i++) {
      *zptr = *xptr;
      firstbits = *zptr >> shiftamount;
      *zptr <<= indexmod;
      *zptr ^= bitscopy;
      bitscopy = firstbits;
      *(xptr++);
      *(zptr++);
    }
  }

  if (x.size + amount < zsize)
    *zptr = bitscopy;

  zero.b_clear();

  return z;
}
  
BigNum b_div_2exp (const BigNum & x, const unsigned long int ex_of_2)
{
  BigNum two = 2ul;
  BigNum z = x / (two << (ex_of_2 - 1ul));

  two.b_clear();

  return z;
}

BigNum b_pow (const BigNum & base, const unsigned long int exp)
{
  BigNum product = 1ul;

  for (unsigned long int i = 0; i < exp; i++) 
    product = product * base;

  return product;
}

BigNum b_powm (const BigNum & base, const BigNum & exp, const BigNum & mod)
{
  BigNum product = 1ul;
  BigNum ex = exp;
  BigNum y = base;
  unsigned long int zero = 0;

  if (exp.sign == '-') {
    cerr << "exp is negative\n";
    exit(EXIT_FAILURE);
  }

  while (b_cmp(ex, zero) > 0) {
    if ((ex & 1) == 1)
      product = (product * y) % mod;

    y = (y * y) % mod;
    ex = ex >> 1;
  }

  ex.b_clear();
  y.b_clear();

  return product;
}

BigNum b_powm (BigNum & base, const unsigned long int exp, BigNum & mod)
{
  BigNum product = 1ul;
  BigNum y = base;
  unsigned long int ex = exp;

  while (ex > 0) {
    if (ex & 1 == 1)
      product = (product * y) % mod;

    y = (y * y) % mod;
    ex = ex >> 1;
  }

  y.b_clear();

  return product;
}

unsigned long int BigNum::Size() const
{
  return size;
}

unsigned long int * BigNum::V() const
{
  return v;
}

char BigNum::Sign() const
{
  return sign;
}

unsigned long int BigNum::b_get_ui() const
{
  return v[0];
}

BigNum b_abs(const BigNum & x)
{
  BigNum bn = x;
  bn.sign = '+';

  return bn;
}
BigNum b_abs2(const BigNum & x)
{
  BigNum bn = x;
  bn.sign = '+';

  return bn;
}

BigNum b_neg(const BigNum & x)
{
  BigNum bn = x;

  if (bn.sign == '+')
    bn.sign = '-';
  else
    bn.sign = '+';

  return bn;
}

int b_cmp (const BigNum & x, const BigNum & y)
{
  unsigned long int * xptr, * yptr;  
  xptr = x.v + x.size - 1;
  yptr = y.v + y.size - 1;

  if (x == y)
    return 0;
  else if (x.sign == '+' && y.sign == '-')
    return 1;
  else if (x.sign == '-' && y.sign == '+')
    return -1; 
  else if (x.sign == y.sign) {
    if (x.size > y.size) {
      if (x.sign == '+')
        return 1;
      else
        return -1;
    }
    else if (x.size < y.size) {
      if (x.sign == '+')
        return -1;
      else
        return 1;
    }
    
    for (unsigned long int i = 0; i < x.size; i++) {
      if (*xptr > *yptr) {
        if (x.sign == '+')
          return 1;
        else
          return -1;
      }      
      else if (*xptr < *yptr) {
        if (x.sign == '+')
          return -1;
        else
          return 1; 
      }

      *xptr--;
      *yptr--;
    }
  }

  //  xptr = 0;
  //  yptr = 0;  

  return 0; /* junk */
}

int b_cmp (const BigNum & x, const unsigned long int y)
{
  unsigned long int x_ui = x.b_get_ui();
  BigNum bn;
  bn = Set_ui(y);
  int result;
  
  if (x == bn)
    result = 0;
  else {
    if (x.size > 1) {
      if (x.sign == '+')
	result = 1;
      else
	result = -1;
    }
    else { /* size == 1 */
      if (x_ui > y) {
	if (x.sign == '+')
	  result = 1;
	else
	  result = -1;
      }
      else {
	if (x.sign == '-')
	  result = -1;
	else
	  result = 1;
      }
    }
  }

  bn.b_clear();

  return result;
} 

int b_cmp (const BigNum & x, const signed long int y)
{
  int result;
  unsigned long int x_ui;
  BigNum bn;
  bn = Set_si(y);
  
  if (x == bn)
    result = 0;
  else {
    if (x.size > 1) {
      if (x.sign == '+')
	result = 1;
      else  
	result = -1;
    }
    else {
      x_ui = x.b_get_ui();
      
      if (x_ui > 0)
	result = 1;
      else
	result = -1;
    }
  }

  bn.b_clear();

  return result;
}

bool operator == (const BigNum & x, const BigNum & y)
{
  if (x.GetTotalBits() != y.GetTotalBits() || x.sign != y.sign)
    return false;

  return memcmp(x.v, y.v, sizeof(unsigned long int) * x.size) == 0;
}

bool operator == (const BigNum & x, const unsigned long int y)
{
  return (x.size == 1 && *(x.v) == y && x.sign == '+');
}

unsigned long int SmallerSize (const BigNum &x, const BigNum &y)
{
  if (x.Size() < y.Size())
    return x.Size();
  
  return y.Size();
}
  
unsigned long int BiggerSize (const BigNum & x, const BigNum & y)
{
  if (x.Size() > y.Size())
    return x.Size();
  
  return y.Size();
}

/* returns 0 bit for 0 */
int GetNumBits (unsigned long int x)
{
  if (x == 0)
    return 0;

  int bits = NUMBITS - 1;

  for (int i = bits; i > 0; i--)
    if (Test(x, i) == 1)
      return i + 1;

  return 1;
}

unsigned long int BigNum::GetTotalBits() const
{
  return (size - 1) * NUMBITS + GetNumBits(v[size - 1]);
}
       
BigNum BigNum::EraseLeadingBits(unsigned long int numbits)
{
  unsigned long int zsize;
  unsigned long int modulus;
  unsigned long int *zptr, *xptr;
  unsigned long int xbits;
  unsigned long int shiftamount;

  xbits = GetTotalBits();
  zsize = CeilDiv(GetTotalBits() - numbits, NUMBITS);
  modulus = numbits % NUMBITS;
  shiftamount = NUMBITS - (xbits - modulus) % NUMBITS;
  BigNum z(zsize);

  zptr = z.v;
  xptr = v;

  for (unsigned int i = 0; i < z.size; i++)
    *(zptr++) = *(xptr++);

  *(zptr--);
  *zptr = (*zptr << shiftamount) >> shiftamount;

  if (z.size != 1)
    while (*zptr == 0 && z.size != 1) {
      *(zptr--);
      z.size--;
    }

  return z;
}

ostream& operator << (ostream& os, const BigNum& bn)
{
  unsigned long int * ptr;
  unsigned long int i = 1;

  if (!bn.v)
    return os;

  ptr = bn.v + bn.Size() - 1;

  os.setf(ios::uppercase);
  os << hex << *ptr--;

  while (i < bn.size) {
    i++;

    if (NUMBITS == 32)
      os << setw(8) << setfill('0') << hex << *ptr--;
    else
      os << setw(16) << setfill('0') << hex << *ptr--;
  }

  os << endl;
  //  ptr = 0;

  return os;
}

void BigNum::b_print_w_sign()
{
  cout << sign << *this;
}
 
/* Test bit value */
int Test (const unsigned long int num, int index)
{
  return 0 != (num & (static_cast<unsigned long int>(1) << static_cast<unsigned long int>(index)));
}
 
unsigned long int Set (const unsigned long int num, int index)
{
  return (num | (static_cast<unsigned long int>(1) << static_cast<unsigned long int>(index)));
}

unsigned long int Unset (const unsigned long int num, int index)  
{ 
  return (num & ~(static_cast<unsigned long int>(1) << static_cast<unsigned long int>(index)));
}

unsigned long int CeilDiv (unsigned long int x, unsigned long int y)
{
  if (x % y > 0)
    return x / y + 1;

  return x / y;
}

int C2I(char c)
{
  if (c < 65)
    return c - '0';
  else
    return toupper(c) - 55;  
}


