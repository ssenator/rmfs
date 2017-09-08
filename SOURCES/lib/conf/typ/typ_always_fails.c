
#include "rmfs.h"
  
/* the NULL type is never satisfied and is always FALSE */
tri_t
typ_always_fails(rmfs_param_t *p_val) {
  return FALSE;
}
