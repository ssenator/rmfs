
#include "rmfs.h"

int
rmfs_getxattr(const char *path,
	      const char *xattr_name,
	      char       *xattr_out,
	      size_t      xattr_size) {

  int             xattr_hash;
  int             errno, l, i;
  rnode_t        *p_rn;
  config_param_t *p_cp_match, *p_cp;

  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);
  
  if (!path) {
    return -ENOENT;
  }
  if (!xattr_name) {
    return -ENOENT;
  }
  
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  if (rmfs_mayaccess(p_rn, &errno, NULL) < 0) {
    return errno < 0? errno: -errno;
  }

  xattr_hash = djb_strtohash((char *) xattr_name);
  if (!IS_VALID_HASH(xattr_hash)) {
    return -EINVAL;
  }

  /* requested xattr matches one on the xattr list? */
  for (i = 0, p_cp = p_rn->xattr, p_cp_match = NULL;
           p_cp && p_cp->nm && !p_cp_match && i < p_rn->n_xattr;
               i++, p_cp=p_cp->p_nxt) {
    
    if (strncmp(xattr_name, p_cp->nm, internal_strlen(xattr_name)) == 0) { /* if (p_cp->h == xattr_hash) { */
      p_cp_match = p_cp;
    }
  }
  if (!p_cp_match) {
    return 0;
  }

  /* insufficient buffer space provided, so return size needed */
  if (!xattr_out || xattr_size < p_cp_match->val.size) {

    if (p_cp_match->val.size <= 0) {
      return -ENOMEM;
    }
    return p_cp_match->val.size;
  }
  
  l = typ_copyout(p_cp_match, xattr_out, xattr_size);
  return l;
}

int
rmfs_setxattr(const char *path,
	      const char *xattr_name,
	      const char *xattr_in,
	      size_t      xattr_size,
	      int         flags) {

  rnode_t         *p_rn;
  int             errno, l;
  int             xattr_hash, ctx_def_hash;
  config_param_t *p_cp;
  char           *in2;

  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  if (!path) {
    return -ENOENT;
  }
  if (!xattr_name) {
    return -ENOENT;
  }
  
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  if (rmfs_mayaccess(p_rn, &errno, NULL) < 0) {
    return errno < 0? errno: -errno;
  }

  xattr_hash = djb_strtohash((char *) xattr_name);
  if (!IS_VALID_HASH(xattr_hash)) {
    return -EINVAL;
  }

  ctx_def_hash = djb_strtohash(CTX_XATTR_NM);
  if (!IS_VALID_HASH(ctx_def_hash)) {
    return -EINVAL;
  }
  
  /* XXX for unrecognized extended attributes, say "system.posix_acl_default" */
  if (xattr_hash != ctx_def_hash) {
    return 0;
  }

  if (IS_RTYPE_XATTRIBUTE(p_rn->rtype)) {
    p_cp = p_rn->p_cp;
  }

  /*no buffer space, so just return size needed*/
  if (!xattr_in || xattr_size < p_cp->val.size) {

    if (p_cp->val.size <= 0) {
      return -ENOMEM;
    }
    return p_cp->val.size;
  }
  l = internal_strlen((char *) xattr_in);
  if (l > 0) {
    l += 1;
  }
  in2 = strdup(xattr_in);
  if (!in2) {
    return -EFAULT;
  }
  p_cp->val.ue.str = in2;
  p_cp->val.size   = l;
  /*XXX typ_check(PTYP_CONTEXT, p_cp->val) */
  /*XXX store into BackingStore, replacing any older value */
  /*XXX save reference in the rnode*/
  return l;
}

int
rmfs_listxattr(const char *path,
	       char       *list,
	       size_t      size) {

  rnode_t        *p_rn;
  config_param_t *p_xattr;
  size_t          size_reqd;
  int             bytes_copied, i, l;
  char           *p_list;

  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  if (!path) {
    return -ENOENT;
  }
  
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  if (rmfs_mayaccess(p_rn, &errno, NULL) < 0) {
    return errno < 0? errno: -errno;
  }

  if (!p_rn->xattr || 0 == p_rn->n_xattr) {
    return 0;
  }
  
  for (size_reqd = i = 0, p_xattr = p_rn->xattr;
           i < p_rn->n_xattr;
               p_xattr = p_xattr->p_nxt, i++) {
    
    if (!IS_XATTR_TYPE(p_xattr->typ)) {
      continue;
    }
    if (!p_xattr->nm) {
      continue;
    }

    l = internal_strlen(p_xattr->nm);
    if (l < 0) {
      return -EFAULT;
    }
    size_reqd += l + 1;
  }

  if (size_reqd > size) {
    return size_reqd;
  }
  
  if (!list) {
    return -ENOMEM;
  }
  p_list       = list;
  bytes_copied = 0;
  
  if (memset(list, 0, size) != list) {
    return -EFAULT;
  }
  
  for (i = 0, p_xattr = p_rn->xattr;
           i < p_rn->n_xattr;
               p_xattr = p_xattr->p_nxt, i++) {

    if (!IS_XATTR_TYPE(p_xattr->typ)) {
      continue;
    }
    if (!p_xattr->nm) {
      continue;
    }
    l = internal_strlen(p_xattr->nm);
    if (l < 0) {
      return -EFAULT;
    }
    if (memcpy(p_list, p_xattr->nm, l+1) != p_list) {
      return -EFAULT;
    }
    p_list += l + 1;
  }
  bytes_copied = p_list - list;
  
  if (bytes_copied != size_reqd) {
    ErrExit(ErrExit_ASSERT, "rmfs_listxattr: bytes_copied != size_reqd");
  }

  return bytes_copied;
}


