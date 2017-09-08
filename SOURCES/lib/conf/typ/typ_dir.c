
#include "rmfs.h"

tri_t
typ_dir(rmfs_param_t *p_val) {
  tri_t        rc;
  struct stat  st;
  int          e;
  extern tri_t typ_file(rmfs_param_t *);

  e = 0;
  rc = typ_file(p_val);
  
  /* dir status generates an error? if not ENOENT, typ check fails */
  if (-1 == stat(p_val->ue.pathnm, &st)) {
    e = errno;
    if (ENOENT != e) {
      rc = FALSE;
    }
  } else {
    /* entry status is fine, but it is not a dir? => FALSE */ 
    rc = !S_ISDIR(st.st_mode)? FALSE: TRUE;
  }
  return rc;
}

tri_t
typ_direxist(rmfs_param_t *p_val) {
  tri_t        rc;
  struct stat  st;
  extern tri_t typ_file(rmfs_param_t *);

  rc = typ_file(p_val);
  if (rc == FALSE || -1 == stat(p_val->ue.pathnm, &st)) {
    return rc;
  }
  if (!S_ISDIR(st.st_mode)) {
    return FALSE;
  }
  return rc;
}
