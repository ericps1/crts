#include <cstdio>
#include <cstdlib>
#include <cstddef>

using namespace std;

void *_mymalloc(size_t size, int line, char *message)
{
  char *temp;

  if(size == 0)
    return NULL;

  temp = (char *) malloc(size);
  
  
  if(temp == NULL)
    {
      printf("\nmemory allocation failure in file: %s at line number: %d\n", message, line);
      exit(-1);
    }

  return (void *) temp;
}
