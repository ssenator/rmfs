
#include "rmfs.h"

tri_t
typ_host(rmfs_param_t *p_val) {
  int   l;
  char *p_term, *p_start;

  if (!p_val) {
    Usage(ErrExit_INTERNAL, "typ_host: !p_val");
  }
  if (!p_val->ue.hostname) {
    Usage(ErrExit_INTERNAL, "typ_host: !p_val->ue.hostname");
  }
  
  p_start = p_val->ue.hostname;
  p_term  = strchrnul(p_start, '\0');
  l       = p_term - p_start;  /*= strlen(p_cp->val.hostname)*/
    
  if (l <= 0) {
    ErrExit(ErrExit_CONFIG, "typ_host: illegal hostname: negative or zero-length");
  }
  if (l > HOST_NAME_MAX) {
    ErrExit(ErrExit_CONFIG, "typ_host: illegal hostname: length > HOST_NAME_MAX");
  }
  if (*p_start == '-' || *(p_term-1) == '-') {
    ErrExit(ErrExit_CONFIG, "typ_host: illegal hostname: starts or ends with a '-'");
  }
  /*XXX ping check?*/
  
  p_val->size = l+1; /*text + '\0' to terminate*/
  return TRUE;
}
