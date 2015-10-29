#include "bignum.h"

struct BIGNUM_ARRAY_TYPE
{
  long size;
  BigNum *list;
};

/* Values pertain to this particular parameter: 2^61-1 as modulus*/
struct BIGNUM_ARRAY_TYPE init_factors()
{
  struct BIGNUM_ARRAY_TYPE factors;

  factors.size = 12;
  factors.list = new BigNum[12];
	
  factors.list[0] = (char *)"2";
  factors.list[1] = (char *)"3";
  factors.list[2] = (char *)"5";
  factors.list[3] = (char *)"7";
  factors.list[4] = (char *)"B";
  factors.list[5] = (char *)"D";
  factors.list[6] = (char *)"1F";
  factors.list[7] = (char *)"29";
  factors.list[8] = (char *)"3D";
  factors.list[9] = (char *)"97";
  factors.list[10] = (char *)"14B";
  factors.list[11] = (char *)"529";

  return (factors);
}

