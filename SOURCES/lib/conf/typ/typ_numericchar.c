
#include "rmfs.h"

tri_t
typ_numericchar(rmfs_param_t *p_val) {
  char *s;
  int l, i;

  if (p_val->size == CP_UNK_SIZE) {
    Usage(ErrExit_CONFIG, "typecheck(ALPHANUM) has unknown size");
  }
  l = p_val->size;

  for (i = 0, s = p_val->ue.str; i < l && s && *s; i++, s++) {
    if (!isdigit(*s)) {
      return FALSE;
    }
  }
  return TRUE;
}
