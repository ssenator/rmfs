
#include "rmfs.h"

int
rmfs_read(const char *path,
	  char *out, size_t size, off_t offset,
	  struct fuse_file_info *fi) {
  
  int             errno, l;
  rnode_t        *p_rn;
  
  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);
  
  l = errno = 0;

  if (!path) {
    return -ENOENT;
  }
  
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  if (!p_rn->p_cp) {
    return -EFAULT;
  }
  if (p_rn->p_cp->val.size == CP_UNK_SIZE) {
    return -ERANGE;
  }

  if (rmfs_mayaccess(p_rn, &errno, fi) < 0) {
    return errno < 0? errno: -errno;
  }

  /*
   * if the user's buffer is too small, tell them how much space is required
   */
  if (p_rn->p_cp->val.size > size) {      /* -ERANGE */
    return p_rn->p_cp->val.size+1;  /* >0 so not interpreted as err by caller */
  }
  l = typ_copyout(p_rn->p_cp, out, size); /* l = # bytes written, possibly 0, or -errno */
  return l;
}
