#include <cstdio>

using namespace std;

int main()
{
  int c;
  
  while( (c=getchar())!=EOF )
    printf("%x ",c);
  
  putchar('\n');
  
  return 0;
}
