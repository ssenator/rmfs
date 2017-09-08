
#include "rmfs.h"


/*
 * access()
 *  checks access to path for access methods
 *  note that most mask checks are done generically, before the individual
 *  file ops are called
 */
int
rmfs_access(const char *path, int mask){
  rnode_t *p_rn;
  int      errno;

  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  if (rmfs_mayaccess(p_rn, &errno, NULL) < 0) {
    return errno >= 0? -errno: -EPERM;
  }

  return 0;
}
