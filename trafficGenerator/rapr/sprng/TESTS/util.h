#ifndef _util_h
#define _util_h

#include <stddef.h>

#define mymalloc(a) (_mymalloc((a), __LINE__, __FILE__))

void *_mymalloc (size_t size, int line, char *message);

#endif
