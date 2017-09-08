
#include "rmfs.h"

void
set_val_truth(config_param_t *p_cp, tri_t v) {
  rmfs_param_t new_val;
  tri_t        tc = UNSET;
  extern char *tri2str[];

  new_val.ue.truth = v;
  tc = typ_check(p_cp->typ, &new_val);
  if (tc != TRUE) {
    ErrExit(ErrExit_CONFIG, "set_val_truth: typ_check failed");
    /*NOTREACHED*/

  } else {
    /* store numeric truth types directly, not with a pointer */
    p_cp->val.ue.truth = v;
    
    /* sizes are the string length of the ascii representation of their value */
    switch (v) {
    case UNSET:
      p_cp->val.size = internal_strlen(tri2str[0]);
      break;
    case FALSE:
      p_cp->val.size = internal_strlen(tri2str[1]);
      break;
    case TRUE:
      p_cp->val.size = internal_strlen(tri2str[2]);
      break;
    default:
      p_cp->val.size = 0;
      break;
    }
  }
  
  return;
}

