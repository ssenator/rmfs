
#include "rmfs.h"

/*
 * store ptr to char
 */
void
set_val_charptr(config_param_t *p_cp, char *p_str) {
  rmfs_param_t new_val;
  tri_t        tc = UNSET;
  int          len;
  
  if (!p_cp) {
    ErrExit(ErrExit_CONFIG, "set_val_charptr: null p_cp");
    return;
  }
  if (!p_str) {
    ErrExit(ErrExit_CONFIG, "set_val_charptr: null p_str");
  }
  
  if ((len = internal_strlen(p_str)) < 0) {  /* len = 0 is OK */
    ErrExit(ErrExit_CONFIG, "set_val_charptr: strlen < 0");
  };
  new_val.ue.str = p_str;
  new_val.size   = len + 1;

  tc = typ_check(p_cp->typ, &new_val);
  if (tc != TRUE) {
    ErrExit(ErrExit_CONFIG, "set_val_charptr: typ_check failed");
    /*NOTREACHED*/
  } else {
    p_cp->val.ue.str = new_val.ue.str;
    p_cp->h          = djb_strtohash(p_str); /* make the ptr into a str */
    p_cp->val.size   = len+1;                /* by adding a hash and a size */
  }
  return;
}
