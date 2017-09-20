
#include "rmfs.h"

/*
 * typ_context()
 *  is called from the generic typ_check() function but is in this file rather than rmfs_conf.c
 *  to handle the selinux dependency
 * XXXrefactor
 *
 * validate a security context
 */

tri_t
typ_context(rmfs_param_t *p_val) {
  pid_t                u_pid;
  struct fuse_context *p_fcntxt;
  security_context_t   secontxt = NULL;
  char                *p_c;

  extern int getpidcon(pid_t, security_context_t *);     /*selinux.h*/
  extern int security_check_context(const char * con); /*selinux.h*/

  if (!p_val) {
    ErrExit(ErrExit_ASSERT, "typ_context: !p_val");
    goto charcheck;
  }
  
  p_fcntxt = fuse_get_context();
  if (!p_fcntxt) {
    ErrExit(ErrExit_WARN, "typ_context: no fuse context (yet?), fall back to xattr-only check");
    goto charcheck;
  }

  u_pid = p_fcntxt->pid;
  if (0 == u_pid || 1 == u_pid || -1 == u_pid) {
    ErrExit(ErrExit_WARN, "typ_context: implausible pid (0, 1, -1)");
    goto charcheck;
  }
  if (kill(u_pid, 0) < 0) {
    goto charcheck;
  }

  if (!is_selinux_enabled()) {
    ErrExit(ErrExit_WARN, "typ_context: !is_selinux_enabled(), skipping context checks");
  } else {
    if (getpidcon(u_pid, &secontxt) < 0) {
      ErrExit(ErrExit_ASSERT, "typ_context: !getpidcon()");
      goto charcheck;
    }
    if (security_check_context(secontxt) < 0) {
      return FALSE;
    }
#ifdef XXXSEPOLICY_JOBID_CTX_DEFINED
    if (selinux_file_context_cmp(secontxt, p_val->ue.str) < 0) {
      return FALSE;
    }
#endif
  }

charcheck:
  if (!p_val->ue.str) {
    return FALSE;
  }

  for (p_c = p_val->ue.str; p_c && *p_c; p_c++) {
    if ('\n' == *p_c && *(p_c+1) == '\0') {
      break;
    }
    if (!isprint(*p_c)) {
      return FALSE;
    }
  }
  return TRUE;
}

