
#include "rmfs.h"


/*
 * XXX per-node control functions
 */

void
cleanup_file(config_param_t *p_cp) {

  if (!p_cp) {
    return;
  }

  if (p_cp->val.pd.fstr) {
    fflush(p_cp->val.pd.fstr);
    fsync(p_cp->val.pd.fd);
    fclose(p_cp->val.pd.fstr);
    
    p_cp->val.pd.fstr = NULL;
    p_cp->val.pd.fd   = -1;
  }

  if (p_cp->val.pd.fullpath) {
    unlink(p_cp->val.pd.fullpath);
    free(p_cp->val.pd.fullpath);
    
    p_cp->val.pd.fullpath = NULL;
  }
  return;
}

void
cleanup() {
  cleanup_file(getconfig_fromnm("mountpointfile"));
  cleanup_file(getconfig_fromnm("pidfile"));
  return;
}

tri_t
rn_ctlroot_umount(rnode_t *p_rn, config_param_t *p_cp) {
  
  if (!p_rn) {
    ErrExit(ErrExit_ASSERT, "rn_ctlroot_umount: !p_rn");
    return FALSE;
  }
  /*XXX fuse_umount(mountpoint, channel)*/

  cleanup();
  return TRUE;
}

/*
 * dump asci representation of fstree
 *  p_root is the starting node, if NULL, start at p_fsroot
 *  levels_down is the number of subdirectories to descend
 *   0 = dump only this node's contents
 *  <0 = do not show children
 */

void
effluviate_one_rn(rnode_t *p_rn){
  rnode_t *p_child;
  int      i;

  /*XXX tablify, use log*/
  if (!p_rn) {
    return;
  }

  fprintf(stderr, "rnode: %ld\n\tgen:%d ctime:%ld uid:%d h:%ud\n",
	  p_rn->rino,
	  p_rn->gen, p_rn->ctime, p_rn->uid, p_rn->h);

  fprintf(stderr, "\tnm:\"%s\" rtyp:\"%s\" (%d)\tftype:\"%s\"\n",
	  p_rn->nm? p_rn->nm: "<?>",
	  rnode_buildtab[p_rn->rtype].nm, p_rn->rtype,
	  p_rn->is.dir? "DIR": p_rn->is.file? "FILE": "<unset>");	  

  fprintf(stderr, "\tn_children:%d", p_rn->n_children);
  if (p_rn->n_children > 0) {
    fprintf(stderr, "\n\tchildren rino:[");
    for (i = 0, p_child = p_rn->children; i < p_rn->n_children; i++, p_child++) {
      fprintf(stderr, "%ld%s", p_child->rino, i < p_rn->n_children-1? ",": "");
    }
    fprintf(stderr, "]\n\tchildren: ");
    for (i=0, p_child = p_rn->children; i < p_rn->n_children; i++, p_child++) {
      fprintf(stderr, "%s%s", p_child->nm? p_child->nm: "",
	                      i < p_rn->n_children-1? ",": "");
    }
   }
  fprintf(stderr, "\n");

  if (!derefable_cp(p_rn->p_cp)) {
    fprintf(stderr, "\tp_cp: unreferencable: (%ld)\n", (long) p_rn->p_cp);
  } else {
    effluviate_one_cp(p_rn->p_cp);
  }

  fprintf(stderr, "\tparent->rino:%ld", p_rn->parent? p_rn->parent->rino: ~0);
  if (p_rn->parent) {
    fprintf(stderr, " \"%s\"", p_rn->parent->nm? p_rn->parent->nm: "<null>");
    if (p_rn->rino == p_rn->parent->rino) {
      fprintf(stderr, " (self/fsroot)");
    }
  }
  fprintf(stderr, "\n");
}

void
effluviate_rnode(rnode_t *p_start, int levels_down) {
  rnode_t *p_child;
  int      i;
  
  if (!p_start) {
    return;
  }
  effluviate_one_rn(p_start);
  if (levels_down < 0) {
    return;
  }
  for (i = 0, p_child = p_start->children; i < p_start->n_children; i++, p_child++) {
    effluviate_one_rn(p_child);
  }
  for (i = 0, p_child = p_start->children; i < p_start->n_children; i++, p_child++) {
      effluviate_rnode(p_child, levels_down-1);
  }
  return;
}


void
effluviate_fstree(rnode_t *p_root, int levels_down) {
  rn_param_t *p_rn_paramtab;

  if (!p_root) {
    p_rn_paramtab = get_rn_params(/*needlock*/ FALSE);

    if (!p_rn_paramtab) {
      ErrExit(ErrExit_INTERNAL, "effluviate_fstree: !rn_paramtab");    
    }
    if (!p_rn_paramtab->p_fsroot) {
      ErrExit(ErrExit_INTERNAL, "effluviate_fstree: !rn_paramtab->p_fsroot");    
    }
    rn_paramtab_release();
    p_root = p_rn_paramtab->p_fsroot;
  }
  effluviate_rnode(p_root, levels_down);
  fprintf(stderr, "---end of fstree.\n");
  
  return;
}

tri_t
rn_ctlroot_effluviate(rnode_t *p_rn, config_param_t *p_cp) {
  
  if (!p_rn) {
    ErrExit(ErrExit_ASSERT, "rn_ctlroot_effluviate: !p_rn");
    return FALSE;
  }
  effluviate_fstree(NULL, INT_MAX); /*XXX record tree depth in rn_param*/
  return TRUE;
}

tri_t
rn_ctlroot_write(rnode_t *p_rn, config_param_t *p_cp) {
  
  if (!p_rn) {
    ErrExit(ErrExit_ASSERT, "rn_ctlroot_write: !p_rn");
    return FALSE;
  }

  if (!isCntrl()) { /* only valid in controller mode */
    return FALSE;
  }

  /*
   * walk dirty list
   *  count # nodes
   *   if > max in backing store
   *    resize backing store
   *   else
   *    seek BackingStore to infinite_expiration
   *     dump them
   *    seek BackingStore to node_delta
   *     dump them
   */
  write_modifiedrnode_toBackingStore();
  return TRUE;
}



tri_t
rn_ctlroot_check(rnode_t *p_rn, config_param_t *p_cp) {
  
  if (!p_rn) {
    ErrExit(ErrExit_ASSERT, "rn_ctlroot_check: !p_rn");
    return FALSE;
  }

  /*XXX ck_fs(p_rn);*/
  
  cleanup();
  return TRUE;
}
