
#include "rmfs.h"

void
set_val_uid(config_param_t *p_cp, uid_t uid) {

  /*XXX typcheck_uid()*/
    
  p_cp->val.ue.uid = uid;
  p_cp->val.size  = sizeof(uid_t);
  return;
}

