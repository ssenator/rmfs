
#include "rmfs.h"

/*
 * resource manager file system - fuse file ops linkage
 */

extern int   rmfs_access(const char *, int);
extern int   rmfs_getattr(const char *, struct stat *);
extern int   rmfs_trunc(const char *, off_t);
extern void *rmfs_init(struct fuse_conn_info *);
extern int   rmfs_open(const char *, struct fuse_file_info *);
extern int   rmfs_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
extern int   rmfs_readdir(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *);
extern int   rmfs_statfs(const char *, struct statvfs *);
extern int   rmfs_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
extern int   rmfs_getxattr(const char *, const char *, char *, size_t);
extern int   rmfs_setxattr(const char *, const char *, const char *, size_t, int);
extern int   rmfs_listxattr(const char *, char *, size_t);
extern int   rmfs_poll(const char *, struct fuse_file_info *, struct fuse_pollhandle *, unsigned *);

struct fuse_operations rmfs_file_ops = {
  .access	= rmfs_access,
  .getattr	= rmfs_getattr,
  .truncate     = rmfs_trunc, /* required in order for rmfs_write to be called */

  .init		= rmfs_init,

  .open		= rmfs_open,
  .read		= rmfs_read,
  .readdir	= rmfs_readdir,
  
  .statfs	= rmfs_statfs,
  .write        = rmfs_write,

  .getxattr	= rmfs_getxattr,
  .setxattr	= rmfs_setxattr,
  .listxattr	= rmfs_listxattr,

  .poll         = rmfs_poll,
};
