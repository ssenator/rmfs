
#include "rmfs.h"

void
set_val_ui(config_param_t *p_cp, unsigned int uiv) {
  rmfs_param_t new_val;
  tri_t        tc = UNSET;

  new_val.ue.ui = uiv;
  
  tc = typ_check(p_cp->typ, &new_val);
  if (tc != TRUE) {
    ErrExit(ErrExit_CONFIG, "set_val_ul: typ_check failed");
    /*NOTREACHED*/
  } else {
    
    /* store numeric types directly, not with a pointer */
    p_cp->val.ue.ul = uiv;
    p_cp->val.size  = sizeof(unsigned long);
  }
  
  return;
}
