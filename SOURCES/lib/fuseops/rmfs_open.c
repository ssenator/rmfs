
#include "rmfs.h"

int
rmfs_open(const char *path, struct fuse_file_info *fi) {

  rnode_t *p_rn;
  int      errno;

  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  errno   = 0;

  if (!path) {
    return -ENOENT;
  }
  if (!fi) {
    return -ENXIO;
  }
  fi->direct_io   = FALSE;
  fi->keep_cache  = FALSE;
  fi->nonseekable = TRUE;
  fi->fh          = -1;
  fi->lock_owner  = -1;
	 
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  if (rmfs_mayaccess(p_rn, &errno, fi) < 0) {
    return errno < 0? errno: -errno;
  }
  return -errno;
}
