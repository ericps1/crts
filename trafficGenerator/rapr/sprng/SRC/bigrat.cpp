/*************************************************************************/
/*************************************************************************/
/* Implementation file for arithmetic on large rationals                 */
/*                                                                       */
/* Author: J. Ren,                                                       */
/*            Florida State Unversity                                    */
/* E-Mail: ren@csit.fsu.edu                                              */
/*                                                                       */
/*************************************************************************/
/*************************************************************************/

#include "bigrat.h"

BigRat::BigRat()
{
  numerator = 0ul;
  denominator = 1ul;
}

BigNum BigRat::br_get_num()
{
  return numerator;
}

BigNum BigRat::br_get_den()
{
  return denominator;
}

void BigRat::br_set_num(const BigNum & num)
{
  numerator = num;
}

void BigRat::br_set_den(const BigNum & den)
{
  denominator = den;
}
