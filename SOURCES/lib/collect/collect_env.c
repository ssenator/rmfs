
#include "rmfs.h"



param_source_t
collect_env(config_param_t *p_cp) {
  char *evarp;
  int   rc;

  rc = PSRC_NONE;
  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "collect_env: !p_cp");
    return PSRC_NONE;
  }

  if ((evarp = getenv(p_cp->nm))) {
    set_val_ptr(p_cp, evarp);
    rc = PSRC_ENV;
  }
  return rc;
}
