#include "rmfs.h"

/*
 * rn_provision()
 *   allocate an rnode, setting the common fields
 *   the per-type build function will usually call this as the 1st step in
 *   rnode construction
 *
 * this routine (and the deallocator) are the only ones that manage rnode de/allocation
 *
 */    
rnode_t *
rn_provision(rnode_t *p_parent, rnode_t *rn_pro) {
  rnode_t          *p_new, *p_i;
  time_t            t;
  rn_param_t       *p_rn_paramtab;
  unsigned long     i, prev_curalloc, basealloc, n_alloc, n;

  extern void     rn_poolinit(void);      /*rn_alloc.c*/
  extern rnode_t *rn_alloc(unsigned int); /*rn_alloc.c*/
  
  p_rn_paramtab = get_rn_params(/*needlock*/TRUE);
  if (!p_rn_paramtab) {
    return NULL;
  }
  if (!p_rn_paramtab->rn_pool) {
    rn_poolinit();
  }
  if (!rn_pro) {
    return NULL;
  }
  if (rn_pro->n_children <= 0) {
    return NULL;
  }
  
  n_alloc       = rn_pro->n_children;
  prev_curalloc = p_rn_paramtab->rn_curalloc;
  basealloc     = p_rn_paramtab->rn_basealloc;

  if (!(p_new = rn_alloc(n_alloc))) {
    goto out;
  }
  if (memset(p_new, 0, sizeof(rnode_t) * n_alloc) != p_new) {
    ErrExit(ErrExit_NOMEM, "rn_provistion: memset");
    p_new = NULL;
    goto out;
  }
  for (i = 0, n = n_alloc, p_i = p_new; i < n; i++, p_i++) {
    time(&t);
    p_i->rtype   = RN_GUARD;
    p_i->ctime   = t;
    p_i->buildfn = NULL;
    p_i->parent  = p_parent;
    p_i->rino    = basealloc + prev_curalloc + i;
    p_i->uid     = p_rn_paramtab->rm_uid[RM_UID_CTL];
    p_i->gen     = 0;
  }

 out:  
  rn_paramtab_release();
  return p_new;
}
