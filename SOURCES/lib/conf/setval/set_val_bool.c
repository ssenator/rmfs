
#include "rmfs.h"

void
set_val_bool(config_param_t *p_cp, bool_t v) {
  switch (v) {
  case TRUE:
    set_val_truth(p_cp, TRUE);
    break;
  case FALSE:
    set_val_truth(p_cp, FALSE);
    break;
  }
}
