#define mymalloc(a) (_mymalloc((a), __LINE__, __FILE__))

void *_mymalloc (long size, int line, const char *message);

