#include <iostream>
#include <cstdio>
#include <cstdlib>

using namespace std;

void *_mymalloc(long size, int line, const char *message)
{
  char *temp;

  if(size == 0)
    return NULL;

  temp = (char *) malloc(size);
  
  if(temp == NULL)
    {
      fprintf(stderr,"\nmemory allocation failure in file: %s at line number: %d\n", message, line);
      return NULL;
    }

  return (void *) temp;
}
