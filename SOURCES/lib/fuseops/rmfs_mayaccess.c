
#include "rmfs.h"
extern rnode_t *namer(const char *, rnode_t *, int *);

/*
 * rmfs_mayaccess() 
 *  note that this is only the posix/DAC check
 */
int
rmfs_mayaccess(rnode_t *p_rn, int *p_errno, struct fuse_file_info *fi) {
  uid_t                uid_dispatch;
  struct fuse_context *p_fcntxt; /*fuse.h*/

  if (!p_rn) {
    *p_errno = EINVAL;
    return -1;
  }

  if (!(p_fcntxt = fuse_get_context())) {
    *p_errno = ENXIO;
    return -1;
  }
  
  uid_dispatch = rn_paramtab.rm_uid[RM_UID_DISPATCH];

  if (!Debug() &&
	(p_fcntxt->uid != p_rn->uid    &&  /*not owner*/
	 p_fcntxt->uid != uid_dispatch &&  /*not dispatcher*/
	 p_fcntxt->uid != 0                /*not root*/
	)
       ){
      /*WOULDMATCH, but access is denied*/
    *p_errno = EPERM;
    return -1;
  }

  /*
   * if our caller wants us to perform these checks, it is responsible
   * for passing us a valid fi
   */
  if (fi) {
    switch (fi->flags & O_ACCMODE) {
    case O_WRONLY:
    case O_RDWR:
    case (O_WRONLY|O_RDWR):
      if (!(p_rn->maybe.writable || p_rn->maybe.controllable)) {
	*p_errno = EPERM;
	return -1;
      }
      break;
      
    case O_RDONLY:
      if (!(p_rn->maybe.readable)) {
	*p_errno = EPERM;
	return -1;
      }
      break;
    }
  }
  return 0;
}
