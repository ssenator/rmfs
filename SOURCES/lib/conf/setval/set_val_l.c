
#include "rmfs.h"

void
set_val_l(config_param_t *p_cp, long v) {
  rmfs_param_t new_val;
  tri_t        tc = UNSET;

  new_val.ue.l = v;
  
  tc = typ_check(p_cp->typ, &new_val);
  if (tc != TRUE) {
    ErrExit(ErrExit_CONFIG, "set_val_l: typ_check failed");
    /*NOTREACHED*/
  } else {
    /* store numeric int types directly, not with a pointer */
    p_cp->val.ue.l = v;
    p_cp->val.size  = sizeof(long);
  }
  
  return;
}
