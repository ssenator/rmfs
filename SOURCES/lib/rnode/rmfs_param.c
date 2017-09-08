
#include "rmfs.h"

/*
 * routines which manipulate the in-memory superblock-lock structure, rn_paramtab
 *
 * XXX locking is broken
 * XXX associate with root rnode and move locking into the rnode_t directly
 */

static int the_rn_paramtab_lock = 0;
rn_param_t rn_paramtab = { 0 };

void
rn_paramtab_lock(void) {
  /*XXX implement with a semaphore & threadid */
  the_rn_paramtab_lock += 1;
  return;  /*XXXMULTITHREAD */
}

void
rn_paramtab_release(void) {
  /*XXX implement with a semaphore & threadid */
  
  /*
   * if (rn_paramtab_lock == our threadid) {
   *   clear it
   *     return
   * else
   *     wakeup(threadid waiting on the lock
   */
  the_rn_paramtab_lock -= 1; /*XXXatomicity?*/
  return; /*XXXMULTITHREAD */
}

/*
 * usual method to obtain a reference to the rnode parameter table
 * returns with a lock held, if requested to do so
 */
rn_param_t *
get_rn_params(int needlock) {

  if (needlock) {
    rn_paramtab_lock();
  }
  return &rn_paramtab;
}

/*
 * XXX needs generalizing; XXX+ convert bitfields to addressable members
 */
rnode_t *
set_rnparam_rn(off_t offset, rnode_t *p_new) {
  rn_param_t   *p_rn_paramtab;
  rnode_t    **pp_rn;

  if (!p_new) {
    return NULL;
  }

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return NULL;
  }
  pp_rn  = (rnode_t **) p_rn_paramtab + offset;
  *pp_rn = p_new;
  rn_paramtab_release();
  
  return p_new;
}

void
set_rnparam_time(off_t offset, time_t t) {
  rn_param_t *p_rn_paramtab;
  time_t     *pt_off;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return;
  }
  pt_off  = (time_t *) p_rn_paramtab + offset;
  *pt_off = t;
  rn_paramtab_release();
  return; 
}

void
set_rnparam_ul(off_t offset, unsigned long new_val) {
  rn_param_t    *p_rn_paramtab;
  unsigned long *p_off;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_INTERNAL, "cannot obtain ptr->rn_paramtab");
    return;
  }
  p_off  = (unsigned long *) p_rn_paramtab + offset;
  *p_off = new_val;
  rn_paramtab_release();
  return;
}

rnode_t *
get_rnparam_fsroot(void) {
  rn_param_t *p_rn_paramtab;
  rnode_t    *p_rn;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  p_rn = (rnode_t *) p_rn_paramtab + offsetof(struct rnode_params, p_fsroot);

  rn_paramtab_release();
  
  return p_rn;
}


rnode_t *
get_rnparam_rn(off_t offset) {
  rn_param_t  *p_rn_paramtab;
  rnode_t     *p_rn;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  p_rn = (rnode_t *) (p_rn_paramtab + offset);
  rn_paramtab_release();
  
  return p_rn;
}

unsigned long *
get_rnparam_ulp(off_t offset) {
  rn_param_t    *p_rn_paramtab;
  unsigned long *p_ul;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  p_ul = (unsigned long *) p_rn_paramtab + offset;
  rn_paramtab_release();
  
  return p_ul;
}

unsigned long
get_rnparam_ul(off_t offset) {
  rn_param_t   *p_rn_paramtab;
  unsigned long ul, *p_ul;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  p_ul = (unsigned long *) p_rn_paramtab + offset;
  rn_paramtab_release();
  ul = *p_ul;
  
  return ul;
}

uid_t *
get_rnparam_uidp(off_t offset) {
  rn_param_t *p_rn_paramtab;
  uid_t      *p_uid;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  p_uid = (uid_t *) (p_rn_paramtab + offset);
  rn_paramtab_release();
  
  return p_uid;
}

mode_t *
get_rnparam_modep(void)
{
  rn_param_t *p_rn_paramtab;
  mode_t     *p_mode;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  p_mode = &p_rn_paramtab->def_mode[0];
  rn_paramtab_release();
  
  return p_mode;
}

unsigned long
get_rnparam_fsid(void)
{
  rn_param_t   *p_rn_paramtab;
  unsigned long fsid;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  fsid = p_rn_paramtab->fsid;
  rn_paramtab_release();
  
  return fsid;
}

unsigned long
get_rnparam_version(void)
{
  rn_param_t   *p_rn_paramtab;
  unsigned long version;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  version = p_rn_paramtab->version;
  rn_paramtab_release();
  
  return version;
}

mode_t *
get_rnparam_modep_offset(off_t offset) {
  rn_param_t *p_rn_paramtab;
  mode_t     *p_mode;

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }
  p_mode = (mode_t *) (p_rn_paramtab + offset);
  rn_paramtab_release();
  
  return p_mode;
}

/*
 * add an rnode to the dirty list
 *  the node being added becomes the list head
 */
bool_t
rn_param_adddirty(rnode_t *p_rn) {
  rn_param_t *p_rn_paramtab;
  rnode_t    *p_dirty;

  if (!p_rn) {
    return FALSE;
  }
  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    return FALSE;
  }

  /*XXX FIXTHIS set_rnparam_rn(RNPARAM_DIRTY_OFF, p_rn);*/
  p_dirty                = p_rn_paramtab->p_dirty; /* may return NULL if none are dirty yet */
  p_rn_paramtab->p_dirty = p_rn;
  p_rn->nxt_dirty        = p_dirty;

  /*XXXFUTURE if/when a poll() interface is added, raise_poll_flag() */
  rn_paramtab_release();
  return TRUE;
}

/*
 * set rnparam fsroot
 * XXX set_rnparam_rn(offset....
 */ 
void
set_rnparam_fsroot(rnode_t *p_new_fsroot) {
  rn_param_t *p_rn_paramtab;

  if (!p_new_fsroot) {
    ErrExit(ErrExit_INTERNAL, "set_rnparam_fsroot: !p_new_fsroot");
  }
  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_INTERNAL, "set_rnparam_fsroot: !p_rn_paramtab");
  }

  if (p_rn_paramtab->p_fsroot) {
    ErrExit(ErrExit_WARN, "set_rnparam_fsroot: resetting fsroot"); /*ASSERT*/
  }
  p_rn_paramtab->p_fsroot = p_new_fsroot;
  rn_paramtab_release();
  return;
}

/*
 * set rnparam cname
 * XXX set_rnparam_rn(offset....
 */ 
void
set_rnparam_cname(rnode_t *p_new_cname) {
  rn_param_t *p_rn_paramtab;

  if (!p_new_cname) {
    ErrExit(ErrExit_INTERNAL, "set_rnparam_cname: !p_new_cname");
  }
  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_INTERNAL, "set_rnparam_cname: !p_rn_paramtab");
  }

  if (p_rn_paramtab->p_cname) {
    ErrExit(ErrExit_WARN, "set_rnparam_cname: resetting cname"); /*ASSERT*/
  }
  p_rn_paramtab->p_fsroot = p_new_cname;
  rn_paramtab_release();
  return;
}

/*
 * set rnparam jobd
 * XXX set_rnparam_rn(offset....
 */
void
set_rnparam_jobd(rnode_t *p_new_jobd) {
  rn_param_t *p_rn_paramtab;

  if (!p_new_jobd) {
    ErrExit(ErrExit_INTERNAL, "set_rnparam_fsroot: !p_new_jobd");
  }
  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_INTERNAL, "set_rnparam_jobd: !p_rn_paramtab");
  }

  if (p_rn_paramtab->p_jobd) {
    ErrExit(ErrExit_WARN, "set_rnparam_jobd: resetting jobd"); /*ASSERT*/
  }
  p_rn_paramtab->p_jobd = p_new_jobd;
  rn_paramtab_release();
  return;
}

