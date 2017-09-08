
#include "rmfs.h"

int
rmfs_getattr(const char *path, struct stat *st){
  rnode_t          *p_rn;
  int               errno;
  mode_t           *p_mode;
  config_param_t   *p_cp;

  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  memset(st, 0, sizeof(struct stat));
  errno = 0;
	 
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  st->st_dev     = get_rnparam_fsid();
  st->st_ino     = p_rn->rino;
  st->st_uid     = p_rn->uid;
  st->st_gid     = p_rn->uid; /*not all resource managers provide a group id*/
  st->st_nlink   = 1;
  st->st_rdev    = 0;
  st->st_atime   = p_rn->ctime;
  st->st_ctime   = p_rn->ctime;
  st->st_mtime   = p_rn->ctime;
  st->st_blksize = sizeof(rnode_t);
  st->st_blocks  = 1 + p_rn->n_children;
  st->st_size    = 0;
  p_cp           = p_rn->p_cp;
  
  
  p_mode = get_rnparam_modep();
  if (p_rn->is.dir) {
    st->st_mode   = S_IFDIR | p_mode[DEFMODE_DIR];
    st->st_size   = st->st_nlink * sizeof(rnode_t);
    st->st_nlink += 1 + p_rn->n_children;  /* parent + children */    

    /*
     * directories that are controllable use the sticky bit S_ISVTX
     * to indicate controllability since S_IXUSR indicates searchability
     */
    if (p_rn->maybe.controllable) {
      st->st_mode |= S_ISVTX;
    }

  } else if (p_rn->is.file) {
    st->st_mode = S_IFREG | p_mode[DEFMODE_FILE];
    
    if (p_cp && p_cp->val.size > 0 && p_cp->val.size != CP_UNK_SIZE) {
      st->st_size = p_rn->p_cp->val.size;
    }

    if (p_rn->maybe.writable) {
      st->st_mode |= S_IWUSR;
    }

    /*
     * prefer to use the 'S_IXUSR' bit for execute permission
     * but there isn't a FUSE exec() file op in this API
     * so allow writes to the control file instead
     */
    if (p_rn->maybe.controllable) {
      st->st_mode |= S_IRUSR | S_IWUSR | S_IXUSR;
    }
  } else if (p_rn->is.link) {

    st->st_size = 0;
    if (p_rn->p_cp && p_rn->p_cp->val.ue.str) {
      st->st_size = internal_strlen(p_rn->p_cp->val.ue.str)+1;
    }
    st->st_mode = S_IFLNK | S_IRUSR;

  } else {
    /* !dir, !file, !link */
    errno = EINVAL;
  }
  return -errno;
}
