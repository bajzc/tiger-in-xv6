#include "kernel/types.h"
#include "user/user.h"
#define UINT_MAX 4294967295L
#define stderr 2
extern void _start();

struct string {
  int length;
  unsigned char chars[1];
};

void GC_info(uint64 map_ptr) {
  uint32 *ptrMap;
  ptrMap = (uint32 *) map_ptr;
  uint32 regNum, inStackNum;
  uint32 *frame;
  fprintf(stderr, "----------\n");
  fprintf(stderr, ">fp: 0x%x\n", ptrMap[0]);
  frame = (uint32 *) (uint64) ptrMap[0];
  fprintf(stderr, ">L_prev: 0x%x\n", ptrMap[1]);
  fprintf(stderr, ">key: %d\n", ptrMap[2]);
  fprintf(stderr, ">regNum: %d\n", ptrMap[3]);
  regNum = ptrMap[3];
  for (int i = 0; i < ptrMap[3]; i++) {
    fprintf(stderr, ">reg[%d]: 0x%x\n", i, ptrMap[i + 4]);
  }
  fprintf(stderr, ">stackPointerNum: %d\n", ptrMap[4 + regNum]);
  inStackNum = ptrMap[4 + regNum];
  for (int i = 0; i < inStackNum; i++) {
    fprintf(stderr, ">frame[-%d]: 0x%x\n", ptrMap[4 + regNum + 1 + i],
            *(frame - ptrMap[4 + regNum + 1 + i]));
  }
  fprintf(stderr, "----------\n");
}

int *initArray(int size, int init, struct string *s, uint32 ptrMap) {
  int i;
  GC_info(ptrMap);
  fprintf(stderr, ">initArray: size: %d, init: 0x%x, descriptor: %s\n", size,
          init, s->chars);
  int *a = (int *) malloc((size + 1) * sizeof(int));
  s->length = size;
  a[0] = (uint64) s;
  fprintf(stderr, ">initArray: a[0]: 0x%x\n", a[0]);
  if ((uint64) a >= UINT_MAX) {
    fprintf(stderr, ">initArray: UINT_MAX exceeded\n");
    exit(1);
  }
  for (i = 1; i <= size; i++)
    a[i] = init;
  fprintf(stderr, ">initArray: alloced memory from %p to %p\n", a, &a[size]);
  return a;
}

int *initRecord(struct string *s, uint32 ptrMap) {
  int i;
  int *p, *a;
  GC_info(ptrMap);
  p = a = (int *) malloc(sizeof(int) * (s->length + 1));
  // int is 32-bit
  if ((uint64) s->chars >= UINT_MAX || (uint64) p >= UINT_MAX) {
    fprintf(stderr, ">initRecord: UINT_MAX exceeded\n");
    exit(1);
  }
  *p++ = (uint64) s->chars;
  fprintf(stderr, ">InitRecord: record descriptor: %s\n", s->chars);
  for (i = 1; i <= s->length; i += sizeof(int))
    *p++ = 0;
  fprintf(stderr, ">initRecord: allocated memory from %p to %p\n", a, p);
  return a;
}

int stringEqual(struct string *s, struct string *t) {
  int i;
  if (s == t)
    return 1;
  if (s->length != t->length)
    return 0;
  for (i = 0; i < s->length; i++)
    if (s->chars[i] != t->chars[i])
      return 0;
  return 1;
}

void printInt(int a) { printf("%d", a); }

void print(struct string *s) { printf("%s", s->chars); }

void flush() {}

struct string consts[256];
struct string empty = {0, ""};

/* int main() { */
/*   int i; */
/*   for (i = 0; i < 256; i++) { */
/*     consts[i].length = 1; */
/*     consts[i].chars[0] = i; */
/*   } */
/*   return (0 /1* static link *1/); */
/* } */

int ord(struct string *s) {
  if (s->length == 0)
    return -1;
  else
    return s->chars[0];
}

struct string *chr(int i) {
  if (i < 0 || i >= 256) {
    printf("chr(%d) out of range\n", i);
    exit(1);
  }
  return consts + i;
}

int size(struct string *s) { return s->length; }

/* conflict definition with the book reference:
  @first: the starting character
 */
struct string *substring(struct string *s, int first, int n) {
  unsigned char *p = s->chars;
  while (p - s->chars < s->length) {
    if (*p == first)
      break;
    p++;
  }
  if (n < 0 || p - s->chars + n > s->length) {
    fprintf(stderr, ">substring([%d],%d,%d) out of range\n", s->length, first,
            n);
    exit(1);
  }
  if (n == 1)
    return consts + first;
  {
    struct string *t = (struct string *) malloc(sizeof(int) + n);
    int i;
    t->length = n;
    for (i = 0; i < n; i++)
      t->chars[i] = *p++;
    return t;
  }
}

struct string *concat(struct string *a, struct string *b) {
  if (a->length == 0)
    return b;
  else if (b->length == 0)
    return a;
  else {
    int i, n = a->length + b->length;
    struct string *t = (struct string *) malloc(sizeof(int) + n);
    t->length = n;
    for (i = 0; i < a->length; i++)
      t->chars[i] = a->chars[i];
    for (i = 0; i < b->length; i++)
      t->chars[i + a->length] = b->chars[i];
    return t;
  }
}

int not(int i) { return !i; }

#undef getchar

struct string *getchar() {
  char c;
  read(0, &c, 1);
  if (c == -1)
    return &empty;
  else
    return consts + c;
}

int main() {
  int i;
  for (i = 0; i < 256; i++) {
    consts[i].length = 1;
    consts[i].chars[0] = i;
  }
  fprintf(stderr, ">call tiger here\n");
  uint64 fp, sp;
  asm volatile("mv %0, fp" : "=r"(fp));
  asm volatile("mv %0, sp" : "=r"(sp));
  fprintf(stderr, ">before fp: %lx sp: %lx\n", fp, sp);
  _start();
  fprintf(stderr, ">after fp: %lx sp: %lx\n", fp, sp);
  return 3;
}
