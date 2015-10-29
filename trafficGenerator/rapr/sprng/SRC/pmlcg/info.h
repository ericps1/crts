#define MAXVAL "152D02C7E14AF6800000" /*maximum # of independent streams*/

struct BIGNUM_ARRAY_TYPE 
{
  long size;
  BigNum *list;
};

#define  OP_SIZE    2
#define  RNGBITS       3
#define  SHIFT      29
#define  MASK       0X1FFFFFFF
#define  MAGIC_NUM  "222222222222222"
#define  MAGIC_DEN  "60454F554C0000"
#define  PRIM       37
#define  POWER_N    61

/* Values pertain to this particular parameter: 2^61-1 as modulus*/
struct BIGNUM_ARRAY_TYPE init_factors()
{
  struct BIGNUM_ARRAY_TYPE factors;

  factors.size = 12;
  factors.list = new BigNum[12];
	
  factors.list[0] = 2ul;
  factors.list[1] = 3ul;
  factors.list[2] = 5ul;
  factors.list[3] = 7ul;
  factors.list[4] = 11ul;
  factors.list[5] = 13ul;
  factors.list[6] = 31ul;
  factors.list[7] = 41ul;
  factors.list[8] = 61ul;
  factors.list[9] = 151ul;
  factors.list[10] = 331ul;
  factors.list[11] = 1321ul;

  return (factors);
}

