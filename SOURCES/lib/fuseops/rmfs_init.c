
#include "rmfs.h"

/*
 * init()
 *  verifies that the rnode file tree is accessible, reasonably sane
 *
 *  if in debug mode, emits chatty messages about configuration
 *
 */
void *
rmfs_init(struct fuse_conn_info *ignored_fuse_conn_info){
  config_param_t *p_fsid_cp, *p_version_cp;
  rn_param_t     *p_rn_paramtab;
  
  (void) ignored_fuse_conn_info;
  
  /*XXXFUTURE: mk_fs(0) instead of in main(), iff !Debug */
  /*XXXFUTURE: ck_fs()?  */

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_ASSERT, "rmfs_init: !p_rn_paramtab");
    return NULL;
  }

  p_fsid_cp = getconfig_fromnm("fsid");
  if (!p_fsid_cp) {
    ErrExit(ErrExit_ASSERT, "rmfs_init: !fsid");
    return NULL;
  }
  if (p_rn_paramtab->fsid) {
    ErrExit(ErrExit_ASSERT, "rmfs_init: fsid already set");
    return NULL;
  }
  if (!IS_VALID_HASH(p_fsid_cp->h)) {
    ErrExit(ErrExit_ASSERT, "rmfs_init: invalid fsid hash");
    return NULL;
  }
  p_rn_paramtab->fsid = p_fsid_cp->h;

  p_version_cp = getconfig_fromnm("version");
  if (!p_version_cp) {
    ErrExit(ErrExit_ASSERT, "rmfs_init: !p_version_cp");
    return NULL;
  }
  if (p_version_cp->val.ue.ul == 0) {
    ErrExit(ErrExit_ASSERT, "rmfs_init: p_version_cp->val.ue.ul = 0");
    return NULL;
  }
  if (0 == p_rn_paramtab->version) {
    p_rn_paramtab->version = p_version_cp->val.ue.ul;
  }
  rn_paramtab_release();
  
  return p_rn_paramtab;
}

