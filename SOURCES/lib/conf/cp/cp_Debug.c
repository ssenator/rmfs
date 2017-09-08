
#include "rmfs.h"


tri_t
Debug(void) {
  config_param_t *p_Debug_cp;
  
  p_Debug_cp = getconfig_fromnm("Debug");/*@-mustfreefresh@*/

  if (!derefable_cp(p_Debug_cp)) {
    return FALSE;
  }

  return (tri_t) p_Debug_cp->val.ue.btruth;
}

tri_t
isCtlr(void) {
  config_param_t *p_isController_cp;

  p_isController_cp = getconfig_fromnm("isController");

  if (!p_isController_cp) {
    ErrExit(ErrExit_ASSERT, "isCtlr: !p_isController_cp");
    return FALSE;
  }

  return p_isController_cp->val.ue.truth;
}
