
#include "rmfs.h"

tri_t
typ_xattr(rmfs_param_t *p_val) {
  char *p_c;
  int   i, l;

  if (!p_val->ue.str) {
    return FALSE;
  }
  for (i = 0, l = p_val->size, p_c = p_val->ue.str; i < l && p_c && *p_c; i++, p_c++) { 
    if (!isprint(*p_c)) {
      return FALSE;
    }
  }
  return TRUE;
}
