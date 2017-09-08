
#include "rmfs.h"

/* is valid alphanumeric()? */ 
tri_t
typ_isalphanum(rmfs_param_t *p_val) {
  char *s;
  int   l, i;

  if (p_val->size == CP_UNK_SIZE) {
    Usage(ErrExit_CONFIG, "typecheck(ALPHANUM) has unknown size");
  }
  
  l = p_val->size;
  for (i = 0, s = p_val->ue.str; i < l && s && *s; i++, s++) {
    if (!isalnum(*s) && *s != '_') {
      return FALSE;
    }
  }
  return TRUE;
}


/* is valid alphanumeric()? */ 
tri_t
typ_isalphanum_ornul(rmfs_param_t *p_val) {
  char *s;
  int   l, i;

  if (CP_UNK_SIZE == p_val->size) {
    return TRUE;
  }
  if (0 == p_val->size) {
    return TRUE;
  }
  if (!p_val->ue.str) {
    return TRUE;
  }
  
  l = p_val->size;
  for (i = 0, s = p_val->ue.str; i < l && s && *s; i++, s++) {
    if (!isalnum(*s) && *s != '_') {
      return FALSE;
    }
  }
  return TRUE;
}
