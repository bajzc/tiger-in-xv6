#ifndef TIGER_GC_H
#define TIGER_GC_H
void GC_info(uint32);
void *GC_alloc(unsigned long size, uint32 ptrMap);
void GC_init(void);
#endif
