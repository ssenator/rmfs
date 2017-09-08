
#include "rmfs.h"

/*
 * XXX set_val() analogous to typ_check() to hide type switch table
 * XXX what should be the functions signature for the value parameter? 
 */

/* http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightLinear */
extern unsigned int
psrc_msb(unsigned int tst) {
  unsigned int _r = 0;
  unsigned int _v;
  
  _v = tst;
  while (_v >>= 1) {
    _r++;
  }
  return _r;
}

extern int
internal_strlen(const char *p_str) {
  char *p_term;
  int   len;

  if (!p_str) {
    return -1;
  }
  p_term = strchrnul(p_str, '\0');
  len    = (int) (p_term - p_str);
  
  return len;
}
