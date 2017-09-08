
#include "rmfs.h"

tri_t
typ_path(rmfs_param_t *p_val) {
  char       *pathnm;
  extern int  errno; 
  
  if (!p_val->ue.pathnm) {
    ErrExit(ErrExit_ASSERT, "typ_path()");
    return FALSE;
  }
  pathnm = p_val->ue.pathnm;
  if (-1 == access(pathnm, F_OK)) {
    if (ENOENT == errno) {    /* it's OK not to exist yet */
      return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}
