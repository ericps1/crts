#include <cstdio>
#include <cstdlib>
#include "util.h"

using namespace std;

double stirling(int n, int m)
{
  double **s, temp;
  int i, j;
  
  if(n < m)
    return 0.0;
  else if(n == m)
    return 1.0;
  else if(m == 0 && n > 0)
    return 0.0;
  else if(m == 1 && n > 0)
    return 1.0;
  
  s = new double *[m];

  for(i=0; i<m; i++)
    s[i] = new double[n-m+1];
  
  for(j=0; j<=n-m; j++)
    s[0][j] = 1.0;
  
  for(i=1; i<m; i++)
  {
    s[i][0] = 1.0;
    for(j=1; j<=n-m; j++)
      s[i][j] = (i+1)*s[i][j-1] + s[i-1][j];
  }

  temp = s[m-1][n-m];
  
  for(i=0; i<m; i++)
    delete [] s[i];
  
  return temp;
}


#if 0
int main(int argc, char *argv[])
{
  int n, m;
  
  n = atoi(argv[1]);
  m = atoi(argv[2]);
  
  printf("S(%d,%d) = %f\n", n, m, stirling(n,m));

  return 0;
}
#endif

