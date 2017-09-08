
#include "rmfs.h"

tri_t
typ_file(rmfs_param_t *p_val) {
  tri_t        rc;
  struct stat  st;
  extern tri_t typ_path(rmfs_param_t *);

  rc = typ_path(p_val);
  if (rc != TRUE) {
    goto out;
  }
  
  if (-1 == stat(p_val->ue.pathnm, &st)) {
    if (ENOENT == errno) {
      goto out;
    }
    if (!S_ISREG(st.st_mode)) {
      goto out;
    }
  }
  
 out:  
  return rc;
}

tri_t
typ_filexist(rmfs_param_t *p_val) {
  tri_t rc;
  struct stat st;

  rc = typ_file(p_val);
  if (rc == TRUE) {
    
    if (-1 == stat(p_val->ue.pathnm, &st)) {
      return rc;
    }
    if (!S_ISREG(st.st_mode)) {
      return FALSE;
    }
  }
  return rc;
}
