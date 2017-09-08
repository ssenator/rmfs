
#include "rmfs.h"

/*
 * store ptr to datum
 */
void
set_val_ptr(config_param_t *p_cp, void *p_void) {
  tri_t tc = UNSET;
  
  if (!p_cp) {
    ErrExit(ErrExit_INTERNAL, "set_val_ptr: null p_cp");
  }
  if (!p_void) {
    ErrExit(ErrExit_INTERNAL, "set_val_ptr: null p_void");
  }
  tc = typ_check(p_cp->typ, p_void);
  if (TRUE == tc) {
    p_cp->val.ue.ptr = p_void;
    p_cp->val.size   = CP_UNK_SIZE;
      
  } else if (UNSET == tc) {
    ErrExit(ErrExit_INTERNAL, "set_val_ptr: unknown typecheck failure");
  }
  /*
   * if truly an opaque type, we expect and allow tc = FALSE
   */
  return;
}
