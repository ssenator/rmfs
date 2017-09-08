
#include "rmfs.h"

#define FILLSTAT_DEFAULT 0
#define FILLSTAT_DOT     1
#define FILLSTAT_DOTDOT  2

void
rmfs_readdir_statfiller(rnode_t *p_rn, struct stat *st, int flag) {

  mode_t     *p_mode;

  memset(st, 0, sizeof(struct stat));
  st->st_uid   = p_rn->uid;
  st->st_ctime = p_rn->ctime;
  st->st_ino   = p_rn->rino;
  st->st_nlink = 1;
  st->st_size  = 0;
  if (p_rn->p_cp && p_rn->p_cp->val.size != CP_UNK_SIZE) {
    st->st_size = p_rn->p_cp->val.size;
  }

  p_mode = get_rnparam_modep(); 
  if (p_rn->is.dir) {
    st->st_mode = S_IFDIR | p_mode[DEFMODE_DIR];
    
  } else if (p_rn->is.file) {
    st->st_mode = S_IFREG | p_mode[DEFMODE_FILE];

  } else if (p_rn->is.link) {
    st->st_mode = S_IFLNK | p_mode[DEFMODE_FILE];
  }

  /*
   * Although we set POSIX mode bits, they are derivative of the rnode
   */
  if (p_rn->maybe.writable) {
    st->st_mode |= S_IWUSR;
  }
  if (p_rn->maybe.controllable) {
    st->st_mode |= S_IXUSR | S_IRUSR;
  }

  if (flag == FILLSTAT_DOT) {
    st->st_nlink += 1 + p_rn->n_children;
    
  } else if (flag == FILLSTAT_DOTDOT) {
    if (!p_rn->parent) {
      ErrExit(ErrExit_WARN, "rmfs_readdir_statfiller: FILLSTAT_DOTDOT: !p_rn->parent");
    } else {
      st->st_nlink += 1 + p_rn->parent->n_children;
    }
  }
  return;
}

int
rmfs_readdir(const char            *path,
	     void                  *buf,
	     fuse_fill_dir_t        fillfn,
	     off_t                  ignored_off,
	     struct fuse_file_info *ignored_fi)
{
  rnode_t          *p_rn, *p_child;
  int               errno;
  int               i;
  struct stat       st;
  
  extern rnode_t *namer(const char *, rnode_t *, int *);
  extern int      rmfs_mayaccess(rnode_t *, int *, struct fuse_file_info *);

  errno = 0;
  (void) ignored_off;
  (void) ignored_fi;
	 
  if ((p_rn = namer(path, /*lookup*/ NULL, &errno)) == NULL) {
    return errno >= 0? -errno: -ENOENT;
  }

  rmfs_readdir_statfiller(p_rn, &st, FILLSTAT_DOT);
  if ((fillfn(buf, ".",  &st, 0)) != 0) {
    return 0;
  }
  
  rmfs_readdir_statfiller(p_rn, &st, FILLSTAT_DOTDOT);
  if ((fillfn(buf, "..", &st, 0)) != 0) {
    return 0;
  }

  for (i = 0, p_child = p_rn->children;
           i < p_rn->n_children && p_child->nm;
               i++, p_child++) {

   rmfs_readdir_statfiller(p_child, &st, FILLSTAT_DEFAULT);
   if (fillfn(buf, p_child->nm, &st, 0) != 0) {
      return 0;
    }
  }

  return 0;
}
