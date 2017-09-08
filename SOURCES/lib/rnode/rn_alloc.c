
#include "rmfs.h"

/*
 * bsalloc()
 *  "Backing Store allocator"
 *  is a drop-in replacement for calloc() that leaves breadcrumbs in backingstore
 */
void *
bsalloc(int n, size_t size) {
  ErrExit(ErrExit_INTERNAL, "bsalloc()");
  return NULL;
}


tri_t
bsalloc_init(config_param_t *p_backingstore_cp) {
  ErrExit(ErrExit_INTERNAL, "bsalloc_init()");
  return FALSE;
}

tri_t
backingstore_valid(config_param_t *p_backingstore_cp) {

  ErrExit(ErrExit_INTERNAL, "backingstore_valid()");
  /*
  ...does backingstore pathname containing dir exist? NO => FALSE
    ...does previous backingstore exist? NO => skip subsequent tests
      ...does it have a size of non-zero? YES => fallthrough

 If does not exist, or is zero length
      reinitialize backingstore => return TRUE

 If it does exist, and has non-zero length
    examine header, confirming fields
    walk allocated pool(s), validating by type
      for type = rnode, validate known type, probably RN_GUARD
	if known & allocated type (!RN_GUARD) - call rnode-type-specific validator()
	  if per-rnode-type validator fails() - mark as unusable
  */
  return FALSE;
 }

/*
 * rn_poolinit()
 *  initializes the pool of memory from which rnodes are allocated
 *  initializes connection to BackingStore, if possible
 *  BackingStore is used to leave breadcrumbs behind for future instances
 *   Does not read from BackingStore at this point if it exists and may be valid
 *   See: collect_predstate() [collect/collect_predecessor.c]
 */

void
rn_poolinit(void) {
  unsigned long  n;
  rnode_t       *p_rn;
  unsigned long *p_allocmap; 
  rn_param_t     *p_rn_paramtab;
  config_param_t *slurmuser_cp, *slurmduser_cp, *rnpool_cp;
  config_param_t *backingstore_cp;
  void           (*allocator)(int, size_t);

  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE);
  allocator = &calloc();

  if (!p_rn_paramtab) {
    ErrExit(ErrExit_ENOMEM, "rn_poolinit: !rn_paramtab");    
  }
  if (p_rn_paramtab->rn_pool) {
    rn_paramtab_release();
    ErrExit(ErrExit_WARN, "rn_poolinit(): recalled");
    return;
  }

  slurmuser_cp = getconfig_fromnm("SlurmUser");
  slurmduser_cp = getconfig_fromnm("SlurmdUser");
  backingstore_cp = getconfig_fromnm("BackingStore");
  if (!slurmuser_cp || !slurmduser_cp || !backingstore_cp) {
    ErrExit(ErrExit_INTERNAL, "rn_poolinit: !slurmuser | !slurmduser | !backingstore_cp");
  }

  /* see calcul_rnodepool() [calcul.c]  */
  n = p_rn_paramtab->rn_minpoolsize;
  if (p_rn_paramtab->rn_minpoolsize <= RNODEPOOL_DEFAULT_MINSIZE) { /*rmfs_param.h*/
    p_rn_paramtab->rn_minpoolsize = n = RNODEPOOL_DEFAULT_MINSIZE;
    if ((rnpool_cp = getconfig_fromnm("rnodepool"))) {
      rnpool_cp->val.ue.l = n * sizeof(rnode_t);
    }
  }
  if (backingstore_valid(backingstore_cp)) {
    allocator = &bsalloc();
  }
  
  if (!(p_rn = (*allocator)(n, sizeof(rnode_t) * n))) {
    ErrExit(ErrExit_ENOMEM, "rn_poolinit: calloc (rnode_pool) failed");
  }

  /*
   * there's a small pad between the end of the build rnode prototypes
   * this is reserved for builder functions and flexibility in object type creation
   * (RN_FIRST --- RN_LAST) ... RN_BASEALLOC ...pool... (RN_FIRST --- RN_LAST) ... poolsize
   * This is useful when the rnode pool is snapshotted to BackingStore.
   */
  p_rn_paramtab->rn_basealloc = RN_BASEALLOC;
  p_rn_paramtab->rn_maxalloc  =
                            p_rn_paramtab->rn_minpoolsize - p_rn_paramtab->rn_basealloc;
  p_rn_paramtab->rn_curalloc = 0;

  p_rn_paramtab->rm_uid[RM_UID_CTL]      = slurmuser_cp->val.ue.uid;
  p_rn_paramtab->rm_uid[RM_UID_DISPATCH] = slurmduser_cp->val.ue.uid;
  
  p_rn_paramtab->def_mode[DEFMODE_FILE] = S_IRUSR | S_IRGRP;
  p_rn_paramtab->def_mode[DEFMODE_DIR]  = S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP;
  
  p_rn_paramtab->rn_pool     = p_rn;
  p_rn_paramtab->rn_allocmap = p_allocmap;
  
  p_rn_paramtab->p_nxt_avail_rn =                          /* ptr to next avail rnode */
                             p_rn_paramtab->rn_pool + p_rn_paramtab->rn_basealloc;
  p_rn_paramtab->nxt_avail = p_rn_paramtab->rn_basealloc; /* nxt avail rnode index num */

  rn_paramtab_release();
  return;
}

/*
 * allocate requested rnodes out of the pool
 */

rnode_t *
rn_alloc(unsigned int requested) {
  unsigned long n, prev_nxt_avail;
  rnode_t      *p_rn;
  rn_param_t   *p_rn_paramtab;

  p_rn_paramtab = get_rn_params(/*needlock*/ FALSE);

  if (!p_rn_paramtab) {
    ErrExit(ErrExit_ENOMEM, "rn_alloc: !rn_paramtab");    
  }

  if (requested <= 0) {
    return NULL;
  }
  if (!p_rn_paramtab->rn_pool) {
    rn_poolinit();
  }
  
  /* XXXMULTITHREAD: get_rn_params(readlock) */
  if (p_rn_paramtab->rn_curalloc + requested > p_rn_paramtab->rn_maxalloc) {
    ErrExit(ErrExit_ENOMEM, "rn_alloc(pool size exceeded)");
  }

  /*
   *   XXXPROTOTYPE rnode pool allocator needs work, esp. for efficient re-use
   *   XXXPROTOTYPE this does accommodate a fragmented pool
   */
  p_rn_paramtab = get_rn_params(/*needlock*/ TRUE); /*get_rn_params(writelock)*/
  p_rn = p_rn_paramtab->p_nxt_avail_rn;
  prev_nxt_avail = p_rn_paramtab->nxt_avail;

  if (memset(p_rn, RN_INVALID, sizeof(rnode_t) * requested) != p_rn) {
    ErrExit(ErrExit_NOMEM, "rn_alloc: memset");
    p_rn = NULL;
    goto out;
  }
  for (n = 0; n < requested; n++) {
    int a = prev_nxt_avail + n;

    if (p_rn_paramtab->rn_allocmap[a] != 0) {
      ErrExit(ErrExit_ENOMEM, "rn_alloc: duplicate allocation");
    }
    p_rn_paramtab->rn_allocmap[a] = 1;

    /*
     * it is less efficient do this in loop, incrementally,
     * but this keeps allocmap consistent with these counters
     * XXXfuture guard these two and p_next_avail_rn change
     * XXXfuture with a write paramtab lock
     */
    p_rn_paramtab->nxt_avail++;
    p_rn_paramtab->rn_curalloc++;
    p_rn_paramtab->p_nxt_avail_rn++;
  }

 out:
  rn_paramtab_release();
  return p_rn;
}


/*
 * rn_raze()
 *  destructor of rnodes
 *
 * XXXFUTURE: intelligently put space back into the pool (quantity in <ignored>)
 */
rnode_t *
rn_raze(rnode_t *p_rn, rnode_t *ignored) {
  ignored = ignored;

  /*NOTIMPL*/
  ErrExit(ErrExit_ASSERT, "rn_raze: NOTIMPL");
  return NULL;
}
