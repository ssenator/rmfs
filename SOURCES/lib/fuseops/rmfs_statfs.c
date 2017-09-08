
#include "rmfs.h"

int
rmfs_statfs(const char *path, struct statvfs *stv) {

  rnode_t       *p_rn;
  int            errno;
  unsigned long *p_maxalloc, *p_basealloc, *p_curalloc, *p_fsid;
  rn_param_t    *p_rn_paramtab;

  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);


  memset(stv, 0, sizeof(struct statvfs));
  errno = 0;
	 
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  stv->f_bsize   = sizeof(rnode_t);
  stv->f_frsize  = 0;
  {
    p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
    if (!p_rn_paramtab) {
      return FALSE;
    }
    p_maxalloc  = &p_rn_paramtab->rn_maxalloc;
    p_basealloc = &p_rn_paramtab->rn_basealloc;
    p_curalloc  = &p_rn_paramtab->rn_curalloc;
    p_fsid      = &p_rn_paramtab->fsid;
    rn_paramtab_release();
  };
  
  stv->f_blocks  = *p_maxalloc;
  stv->f_bfree   = *p_maxalloc - *p_curalloc;
  stv->f_bavail  = *p_maxalloc - *p_curalloc - *p_basealloc;
  stv->f_files   = *p_curalloc;
  stv->f_ffree   = *p_maxalloc - *p_curalloc;
  stv->f_favail  = *p_maxalloc - *p_curalloc - *p_basealloc;
  stv->f_fsid    = *p_fsid;
  stv->f_namemax = _POSIX_NAME_MAX-1;

  return -errno;
}
