
#include "rmfs.h"

int
rmfs_write(const char *path,
	   const char *in, size_t size, off_t offset,
	   struct fuse_file_info *fi) {
  int               errno;
  bool_t          (*p_ctlfn)(rnode_t *, void *);
  rnode_t          *p_rn; 
  tri_t             tc;
  config_param_t   *p_cp_backingstore;
  config_param_t   *p_cp_varrun;
  config_param_t    cp_write;
  
  extern int        typ_copyin(config_param_t *, char *, size_t);         
  extern rnode_t   *namer(const char *, rnode_t *, int *);
  extern int        rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  errno = 0;

  if (!in) {
    errno = -EIO;
    goto out;
  }
  
  if (!path) {
    errno = -ENOENT;
    goto out;
  }
  if (size <= 0) {
    errno = -ENOMEM;
    goto out;
  }

  /*
   * persistent state is stored in BackingStore or in /var/run
   */
  p_cp_backingstore = getconfig_fromnm("BackingStore");
  if (!derefable_cp(p_cp_backingstore)) {
    errno = -EFAULT;
    goto out;
  }
  p_cp_varrun = getconfig_fromnm("varrun");
  if (!derefable_cp(p_cp_varrun)) {
    errno = -EFAULT;
    goto out;
  }
  
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    errno = errno >= 0? -errno: -ENOENT;
    goto out;
  }

  if (rmfs_mayaccess(p_rn, &errno, fi) < 0) {
    return errno < 0? errno: -errno;
  }

  /*
   * controllable semantics are a special flavor of writable:
   *  a controllable node allows writes but only of data that
   *  matches the specific control
   */
  tc = UNSET;
  
  memset(&cp_write, 0, sizeof(config_param_t));
  cp_write.val.size   = size;
  cp_write.val.ue.str = (char *) in;

  if (RNF_KNOB == p_rn->rtype) {
    p_ctlfn = (bool_t (*)(rnode_t *, void *)) (p_rn->p_dyntyp);
    if (!p_ctlfn) {
      errno = -ENXIO;
      goto out;
    }
    tc = (*p_ctlfn)(p_rn, &cp_write);
  } else if (RNF_ATTRIBUTE == p_rn->rtype ||
	     RNF_CONTEXT   == p_rn->rtype
	     ) {
    if (typ_copyin(p_rn->p_cp, (char *) in, size) < 0) {
      errno = -EINVAL;
      goto out;
    }
    rn_param_adddirty(p_rn);
  } else {
    if (p_rn->maybe.writable) {
      ErrExit(ErrExit_ASSERT, "rmfs_write: unhandled writable node");
      errno = -EFAULT;
      goto out;
    }
  }
  
 out:
  return errno < 0? errno: size;
}
