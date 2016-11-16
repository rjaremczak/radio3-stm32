#ifndef _ACTIVE_CHECK_H
#define _ACTIVE_CHECK_H 1

#define active_check(cond, limit) {     \
  int i;                                \
  for (i = (limit); !(cond); --i)       \
    if (i <= 0)                         \
      return -1;                        \
}

#endif
