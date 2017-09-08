
#include "rmfs.h"

int
rmfs_poll(const char             *path,
	  struct fuse_file_info  *fi,
	  struct fuse_pollhandle *ph,
	  unsigned               *reventsp) {

  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  return 0;
}

