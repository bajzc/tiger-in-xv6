#include "kernel/types.h"
#include "user/user.h"

#include "set.h"
#include "tiger_gc.h"
#include "util.h"

#define MEMORY_LIMIT 1024

static unsigned char *from = NULL;
static unsigned char *to = NULL;
static unsigned char *next = NULL;
static unsigned char *limit = NULL;

static int isHeadptrMap(uint32 p) { return *(uint32 *) (uint64) p == 0; }
static Set ptrMapSet = NULL;
static void GC(uint32 start_fp);

void GC_info(uint32 map_ptr) {
  uint32 *ptrMap;
  ptrMap = (uint32 *) (uint64) map_ptr;
  uint32 regNum, inStackNum;
  uint32 *frame;
  debug("----------\n");
  debug("fp: 0x%x\n", ptrMap[0]);
  frame = (uint32 *) (uint64) ptrMap[0];
  debug("L_prev: 0x%x\n", ptrMap[1]);
  if (isHeadptrMap(ptrMap[1]))
    debug("L_prev is head ptrMap\n");
  debug("key: 0x%x\n", ptrMap[2]);
  debug("regNum: %d\n", ptrMap[3]);
  regNum = ptrMap[3];
  for (int i = 0; i < ptrMap[3]; i++) {
    debug("reg[%d]: 0x%x\n", i, ptrMap[i + 4]);
  }
  debug("stackPointerNum: %d\n", ptrMap[4 + regNum]);
  inStackNum = ptrMap[4 + regNum];
  for (int i = 0; i < inStackNum; i++) {
    debug("frame[-%d]: 0x%x\n", ptrMap[4 + regNum + 1 + i],
          *(frame - ptrMap[4 + regNum + 1 + i]));
  }
  debug("----------\n");
}

void GC_init(void) {
  debug("MEMORY_LIMIT: %d\n", MEMORY_LIMIT);
  ptrMapSet = SET_empty(SET_default_cmp);
  from = malloc(MEMORY_LIMIT);
  to = malloc(MEMORY_LIMIT);
  next = from;
  limit = from + MEMORY_LIMIT - 1;
  debug("from: 0x%p - limit 0x%p\n", from, limit);
  debug("to: 0x%p\n", to);
}

void *GC_alloc(unsigned long size, uint32 ptrMap) {
  uint32 *n = malloc(sizeof(uint32));
  *n = ptrMap;
  SET_insert(ptrMapSet, n);
  if (next + size > limit) {
    debug("FROM SPACE IS FULL\n");
    GC(((uint32 *) (uint64) ptrMap)[0]);
    exit(1);
  }
  void *p = next;
  next += size;
  return p;
}

enum pointer_type { ARRAY, RECORD };

static enum pointer_type pointerType(struct string *s) {
  if (s->chars[0] == 'a')
    return ARRAY;
  return RECORD;
}

static uint32 forward(uint32 p_) {
  if (p_ >= (uint64) from && p_ <= (uint64) limit) {
    uint32 *p;
    p = (uint32 *) (uint64) p_;
    struct string *descriptor;
    descriptor = (struct string *) (uint64) p[0];
    debug("descriptor for 0x%x: '%s', length: %d\n", p_, descriptor->chars,
          descriptor->length);
    uint32 p_f1 = p[descriptor->length + 1];
    debug("forwarding pointer for 0x%x: 0x%x\n", p_, p_f1);
    if (p_f1 >= (uint64) to && p_f1 <= (uint64) to + MEMORY_LIMIT) {
      return p_f1;
    } else {
      for (int i = 0; i <= descriptor->length; i++) {
        next[i] = p[i];
      }
      next[descriptor->length + 1] = (uint64) next;
      p[descriptor->length + 1] = (uint64) next;
      next += descriptor->length + 2;
      return p[descriptor->length + 1]; // aka p.f_1
    }
  } else
    return p_;
}

uint32 getStaticLink(uint32 fp) {
  uint32 *sl;
  sl = (uint32 *) (uint64) fp;
  debug("0x%x links to 0x%x\n", fp, *sl);
  return *sl;
}

void forwardPtrMap(uint32 *ptrMap) {
  uint32 regNum, inStackNum;
  uint32 *frame;
  frame = (uint32 *) (uint64) ptrMap[0];
  regNum = ptrMap[3];
  inStackNum = ptrMap[4 + regNum];
  debug("ptrMap: %p, regNum: %d, inStackNum: %d, fp: 0x%p\n", ptrMap, regNum,
        inStackNum, frame);
  for (int i = 0; i < regNum; i++) {
    debug("forward: 0x%x\n", ptrMap[i + 4]);
    forward(ptrMap[i + 4]);
  }
  for (int i = 0; i < inStackNum; i++) {
    debug("forward: 0x%x\n", *(frame - ptrMap[4 + regNum + 1 + i]));
    forward(*(frame - ptrMap[4 + regNum + 1 + i]));
  }
}

void GC(uint32 start_fp) {
  debug("start_fp: 0x%x\n", start_fp);
  Set reachable_fp = SET_empty(SET_default_cmp);
  uint32 fp, static_link;
  fp = start_fp;
  uint32 *n = malloc(sizeof(uint32));
  *n = fp;
  SET_insert(reachable_fp, n);
  static_link = getStaticLink(fp);
  while (static_link) {
    n = malloc(sizeof(uint32));
    *n = fp;
    SET_insert(reachable_fp, n);
    fp = static_link;
    static_link = getStaticLink(fp);
  }

#if DEBUG
  SET_FOREACH(reachable_fp, nptr) {
    uint32 fp = **(uint32 **) nptr;
    debug("reachable fp: 0x%x\n", fp);
  }
  SET_FOREACH(ptrMapSet, nptr) {
    uint32 ptrMap;
    ptrMap = **(uint32 **) nptr;
    debug("ptrMapSet: 0x%x\n", ptrMap);
  }
#endif

  uint32 *scan;
  next = to;
  scan = (uint32 *) to;

  SET_FOREACH(ptrMapSet, mptr) {
    uint32 *ptrMap;
    ptrMap = **(uint32 ***) mptr;
    debug("ptrMap: %p\n", ptrMap);
    uint32 fp = ptrMap[0];
    debug("fp: 0x%x\n", fp);
    SET_FOREACH(reachable_fp, nptr) {
      uint32 n = **(uint32 **) nptr;
      debug("n: 0x%x\n", n);
      if (n == fp || fp == 0) {
        forwardPtrMap(ptrMap);
      }
    }
  }
  while (scan < (uint32 *) next) {
    uint32 *p = (uint32 *) (uint64) scan;
    struct string *descriptor;
    descriptor = (struct string *) (uint64) p[0];
    debug("descriptor at 0x%x: %s\n", p[0], descriptor->chars);
    switch (pointerType(descriptor)) {
      case ARRAY: {
        if (descriptor->chars[1] == 'n') {
          for (int i = 0; i <= descriptor->length;
               i++) // copy integers and descriptor
            scan[i] = p[i];
        } else {
          // array of pointers
          scan[0] = p[0]; // descriptor
          for (int i = 0; i < descriptor->length; i++) {
            scan[i + 1] = forward(p[i + 1]);
          }
        }
        scan[descriptor->length + 1] = (uint64) scan; // special descriptor
        scan += descriptor->length + 2;
        break;
      }
      case RECORD: {
        unsigned char *type = descriptor->chars;
        for (int i = 0; i < descriptor->length; i++, type++) {
          if (*type == 'p')
            scan[i + 1] = forward(p[i + 1]);
          else
            scan[i + 1] = p[i + 1];
        }
        scan[0] = p[0]; // descriptor;
        scan[descriptor->length + 1] = (uint64) scan; // special descriptor
        scan += descriptor->length + 2;
        break;
      }
    }
  }
}
