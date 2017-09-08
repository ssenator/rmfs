
#include "rmfs.h"

/* complex types may devolve to these typ_check functions conf/typ_*.c */

tri_t
typ_check(ptyp_t ptyp, rmfs_param_t *p_val) {
  tri_t tc;
  
  extern tri_t typ_host(rmfs_param_t *);
  extern tri_t typ_isalphanum(rmfs_param_t *);
  extern tri_t typ_numeric(rmfs_param_t *);
  extern tri_t typ_always_fails(rmfs_param_t *);
  extern tri_t (*param_typecheck_functab[])(rmfs_param_t *);

  tc = UNSET;

  if (!p_val) {
    ErrExit(ErrExit_INTERNAL, "typ_check: null p_val");
  }
  if (!p_val) {
    ErrExit(ErrExit_INTERNAL, "typ_check: null p_val");
  }
  if (!IS_VALID_TYPE(ptyp)) {
    ErrExit(ErrExit_INTERNAL, "typ_check: invalid ptype");
  }
  if (!param_typecheck_functab[ptyp]) {
    ErrExit(ErrExit_ASSERT, "typ_check: null typecheck function");
    return typ_always_fails(p_val);
  }
  tc = (*param_typecheck_functab[ptyp])(p_val);
  return tc;
}
