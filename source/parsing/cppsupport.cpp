#include <stdlib.h>

extern "C" void __cxa_pure_virtual()
{
}

void* operator new(size_t size) { return malloc(size); }
void operator delete(void *p) { free(p); }
void* operator new[](size_t size) { return malloc(size); }
void operator delete[](void *p) { free(p); }
