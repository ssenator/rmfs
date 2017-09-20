#include "rmfs.h"


/*
 * is_fsroot_attr()
 *  returns TRUE if an attribute is appropriate for fsroot
 */
tri_t
is_fsroot_attr(config_param_t *p_cp) {
  
  int   rootdir_hash;
  char *rootdir_nm;

  if (!p_cp) {
    return FALSE;
  }
  
  rootdir_nm   = rnode_buildtab[RND_ROOT].nm;
  rootdir_hash = djb_strtohash(rootdir_nm);

  /*
   * config parameters are attributes of the fsroot, unless:
   *  - they match the name of the MountPoint config parameter (rootdir_hash)
   *  - they are explicit resource manager parameters (PSRC_SLURM)
   *  - they weren't set from any source
   */

  if (rootdir_hash == p_cp->h) {
    return FALSE;
  }
  if (IS_SLURM_TYPE(p_cp->typ)) {
    return FALSE;
  }
  if (NOBITS(p_cp->src.actual)) {
    return FALSE;
  }
  if (ISSET(p_cp->src.actual, PSRC_MNT) || ISSET(p_cp->src.actual, PSRC_MNT_NONOPT)) {
    return FALSE;
  }
  return TRUE;
}

/*
 * rn_mkfsroot
 *   creates a root node of the resource management fs
 *
 *   root nodes are where the configuration and mount parameters are made visible
 *   as attributes; they are distinct from most other nodes in the tree in that
 *   they have little or no resource-manager-specific data; they are just the
 *   reference from the file system into the resource-manager data
 *
 * the root node has the configuration parameters that are not resource-manager
 * specific as its attributes, in addition to a control node
 *
 * XXXFUTURE import a fs specification or even a DTD?
 *  (a la "mkproto(8)"); but for this implementation, handcraft
 *
 */
rnode_t *
rn_mkfsroot(rnode_t *p_fsroot, rnode_t *ignored) {
  rnode_t        *p_new, *p_cluster, *p_attrd, *p_control, *p_children;
  rnode_t      *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t         rn_placenta;
  config_param_t *p_cp, *p_2cp;
  int             n_children, n_attr;
  unsigned long   n_alloc;
  char           *rootdir_nm;
/*  int             rootdir_hash; XXX*/
  char           *cname_nm;
  
  extern slurm_ctl_conf_t *p_slcnf;                /*XXX slurm.h use a config_param */

  extern int               attr_cnt_cp(rn_type_t); /*rn_subr.c*/
  extern rnode_t          *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

  ignored = ignored;
  
  rootdir_nm   = rnode_buildtab[RND_ROOT].nm;
/*  rootdir_hash = djb_strtohash(rootdir_nm); XXX*/
  cname_nm     = rnode_buildtab[RND_CLUSTER].nm;
  
  n_children  = n_alloc = 0;
  n_attr      = attr_cnt_cp(RND_ROOT);

  if (n_attr <= 0) {
    ErrExit(ErrExit_INTERNAL, "rn_mkfsroot: attribute count <= 0");
  }

  n_alloc += 1;            /* RND_ROOT       */
  n_alloc += 1;            /* RND_ATTRIBUTES */
  n_alloc += 1;            /* RND_CONTROL    */
  n_alloc += 1;            /* RND_CLUSTER    */

  n_children = n_alloc - 1;

  /*
   * rnode[0]       = fsroot
   * XXX RND_RNPARAMS
   * rnode[1]       = "attributes"
   * rnode[2]       = "control" 
   * rnode[3]       = cluster name = p_fsroot->subdir
   */

  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  if (memset(&rn_placenta, 0, sizeof(rnode_t)) != &rn_placenta) {
    ErrExit(ErrExit_NOMEM, "rn_mkfsroot: memset: cannot allocate placental rnode");
    return NULL;
  }
  rn_placenta.n_children = n_alloc + 1; /* +1 for guard */
  rn_placenta.rtype = RND_ROOT;
  if (!(p_new = (*p_buildfn)(NULL, &rn_placenta))) {
    ErrExit(ErrExit_ASSERT, "rn_mkfsroot: provision failed: out of memory (buildfn)");
    return NULL;
  }
  
  p_attrd   = p_children = p_new + 1;      /* rnode[1] "attributes" */
  p_control = p_children + 1;              /* rnode[2] "control"    */
  p_cluster = p_children + 2;              /* rnode[3] "cluster"    */

  if (rootdir_nm) {
    p_cp = getconfig_fromnm(rootdir_nm);
  }
  /* rnode[0] */
  p_fsroot = rn_cast(p_new, RND_ROOT,
		     p_cp, /*dyntyp*/ NULL, rootdir_nm,
		     /*parent*/ p_new,   /*children*/ p_children, n_children,
		     /*attr*/ p_children+3, /*subdir*/ p_children);
  if (!p_fsroot) {
    ErrExit(ErrExit_ASSERT, "rn_mkfsroot: rn_cast(fsroot) NULL");
    return NULL;
  }

  /*
   * fsroot [rtype=RND_ROOT, parent=p_rootfs] --- <cluster> [RND_CLUSTER, parent=p_rootfs]
   *    |
   *    + "attributes" [RND_ATTRIBUTES, parent=p_rootfs]
   *          |
   *          + attrX...
   */

  /*
   * This will construct the following branch:
   *
   *   fsroot --- ...
   *    |
   *  "attributes" [RND_ATTRIBUTES, parent=p_fsroot]
   *    |
   *    + "BackingStore" [RNF_ATTRIBUTE, parent=p_attrd]
   *    + "Debug"        [RNF_ATTRIBUTE, parent=p_attrd]
   *    + "Hostname"     [RNF_ATTRIBUTE, parent=p_attrd]
   *    + "isctlr"       [RNF_ATTRIBUTE, parent=p_attrd]
   *    + "pid"          [RNF_ATTRIBUTE, parent=p_attrd]
   *    + "version"      [RNF_ATTRIBUTE, parent=p_attrd]
   *    + "unmountmout"  [RNF_ATTRIBUTE, parent=p_attrd]
   *    + ...
   *
   * rn_mkattrd() [RND_ATTRIBUTES] will use the attr_iterator_cp()
   * to get the list of potential attributes. .is_mine()[is_fsroot_attr()]
   * will select those that are fsroot attributes.
   */
  p_2cp = dup_cp(p_cp);
  if (!p_2cp) {
    ErrExit(ErrExit_ASSERT, "rn_mkfsroot: !dup_cp: p_2cp RND_ATTRIBUTES");
    return NULL;
  }
  p_2cp->nm = rnode_buildtab[RND_ATTRIBUTES].nm;
  p_attrd   = rn_cast(p_attrd, RND_ATTRIBUTES,
		      p_2cp, /*dyntyp*/ NULL, p_2cp->nm,
		      /*parent*/ p_fsroot, /*children*/ NULL, /*n_children*/ 0,
		      /*attr*/ NULL, /*subdir*/ NULL);

  if (!p_attrd) {
    ErrExit(ErrExit_ASSERT, "rn_mkfsroot: rn_cast(attr) NULL");
    return NULL;
  }

  set_rnparam_fsroot(p_fsroot);
  /*XXXset_rnparam_rn(offsetof(rn_param_t, p_fsroot), p_fsroot);*/

  if (cname_nm) {
    p_cp = getconfig_fromnm(cname_nm);
  }
  p_cluster = rn_cast(p_cluster, RND_CLUSTER,
		      p_cp, /*dyntyp*/ p_slcnf, cname_nm,
		      /*parent */ p_fsroot, /*children*/ NULL, /*n_children*/ 0,
		      /*attr*/ NULL, /*subdir*/ NULL);
  if (!p_cluster) {
#if defined(PORTING_TO_SLURMv17)	  
    ErrExit(ErrExit_ASSERT, "rn_mkfsroot: rn_cast(cluster) NULL");
#else
    ErrExit(ErrExit_WARN, "rn_mkfsroot: rn_cast(cluster) NULL");
#endif    
    return NULL;
  }

  /*
   * fsroot [rtype=RND_ROOT, parent=p_rootfs] --- <cluster> [RND_CLUSTER, parent=p_rootfs]
   *    |
   *    + "control" [RND_CONTROL, parent=p_rootfs]
   *          |
   *          + cmds known to the parent 
   */

  p_control = rn_cast(p_control, RND_CONTROL,
		      CONFPARAM_MISSINGOK, /*dyntyp*/ NULL,
		      rnode_buildtab[RND_CONTROL].nm,
		      /*parent*/ p_fsroot, /*children*/ NULL, /*n_children*/ 0,
		      /*attr*/ NULL, /*subdir*/ NULL);
  if (!p_control) {
    ErrExit(ErrExit_ASSERT, "rn_mkfsroot: rn_cast(control) NULL");
    return NULL;
  }
  return p_fsroot;
}

