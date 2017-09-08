
#include "rmfs.h"

void
set_val_pid(config_param_t *p_cp, pid_t pid) {

  /*XXX typcheck_pid()*/
  
  p_cp->val.ue.pid = pid;
  p_cp->val.size   = sizeof(pid_t);
  return;
}
