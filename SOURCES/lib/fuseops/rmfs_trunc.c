
#include "rmfs.h"

int
rmfs_trunc(const char *path, off_t trunc_size) {
  int                   errno;
  rnode_t              *p_rn;
  struct fuse_file_info fi;

  extern rnode_t       *namer(const char *, rnode_t *, int *);
  extern int            rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  errno = 0;

  if (!path) {
    errno = -ENOENT;
    goto out;
  }
  if (trunc_size < 0) {
    errno = -ENOMEM;
    goto out;
  }
  
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    errno = errno >= 0? -errno: -ENOENT;
    goto out;
  }

  if (rmfs_mayaccess(p_rn, &errno, &fi) < 0) {
    errno = errno >= 0? -errno: -EPERM;
    goto out;
  }
  
  /*FALLTHROUGH*/;

  /*
   * technically, this is mostly a no-op
   *  since almost all file system nodes are
   *   1) not authoritative (the actual resource manager is)
   *   2) not writable
   * but without the trunc() file-op, write() will never be called,
   * instead ENOSYS short-circuited ("Operation not implemented")
   * error, before this layer is entered
   */

 out:  
  return errno != 0? errno: trunc_size;
}
