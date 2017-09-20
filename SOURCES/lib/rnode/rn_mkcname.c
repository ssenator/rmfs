#include "rmfs.h"

/*
 * rn_mkcname(): make cluster name directory node
 *
 * the "ClusterName" rnode has attributes which are collected from the resource mgr
 * it has two explicitly named sub-directories:
 *    RND_JOBS ("jobs") and
 *    RND_PARTITIONS ("partitions")
 *
 */

/*
 * is_cname_attr()
 *  returns TRUE if an attribute is appropriate for RND_CLUSTER
 */
tri_t
is_cname_attr(config_param_t *p_cp) {

  if (!p_cp) {
    return FALSE;
  }

  /*
   * config parameters are attributes of the clustername,
   * provided that they are actually resource mgr parameters
   */
  if (!IS_SLURM_TYPE(p_cp->typ)) {
    return FALSE;
  }
  if (NOBITS(p_cp->src.actual)) {
    return FALSE;
  }
  if (!ISSET(p_cp->src.actual, PSRC_SLURM)) {
    return FALSE;
  }
  /*
   * dynamic attributes are used to derive the structure of the fs tree
   * so aren't presented as attributes of the top-most clustername node
   */
  if (p_cp->per_src.slurm.dynamic) {
    return FALSE;
  }
  return TRUE;
}

rnode_t *
rn_mkcname(rnode_t *p_fsroot, rnode_t *p_cluster) {
  rnode_t          *p_new, *p_partitions, *p_jobs, *p_attr;
  rnode_t          *p_children, *p_attrd, *p_subdir;
  rnode_t        *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t           rn_pro;
  config_param_t   *p_cp;
  int               n_children, n_attr;
  unsigned long     n_alloc;
  
  extern slurm_ctl_conf_t *p_slcnf; /*XXX slurm.h use a config_param */
  extern int               attr_cnt_cp(rn_type_t); /*rn_subr.c*/
  extern rnode_t          *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

  if (!p_fsroot) {
    ErrExit(ErrExit_ASSERT, "rn_mkcname(NULL parent p_fsroot)");
    return NULL;
  }
  
  n_children = 0;
  n_attr = attr_cnt_cp(RND_CLUSTER);
  if (n_attr <= 0) {
    ErrExit(ErrExit_INTERNAL, "rn_mkcname: attribute count <= 0");
  }
  
  n_children += 1;          /* RND_ATTRIBUTES */
  n_children += 1;          /* RND_JOBS */
  n_children += 1;          /* RNF_PARTS */
  n_alloc     = n_children;

  if (!p_cluster) {
    if (p_fsroot->children) {
      ErrExit(ErrExit_ASSERT, "rn_mkcname: fsroot has children already, but NULL p_cluster)");
      return NULL;
    }
    ErrExit(ErrExit_ASSERT, "rn_mkcname(NULL p_new)");
    return NULL;
  }

  /*
   *  (p_cluster was allocated by the parent's call to the provisioner)
   *   p_new->children = rnode[0], freshly provisioned
   *
   *   rnode[0]   = "attributes", from config parameters
   *   rnode[1]   = "jobs" sub-directory
   *   rnode[2]   = "partitions" sub-directory
   */
  
  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  rn_pro.n_children = n_alloc + 1; /* +1 for guard */
  if (!(p_new = (*p_buildfn)(p_fsroot, &rn_pro))) {
    ErrExit(ErrExit_ASSERT, "rn_mkcname(provision failure)- out of memory?");
    return NULL;
  }
  p_attrd = p_subdir = p_children = p_attr = p_new;

  p_cp = getconfig_fromnm(rnode_buildtab[RND_CLUSTER].nm);
  /*
   * recast ourself with the allocated children, attr, subdir pointers
   */
  p_cluster = rn_cast(p_cluster, RND_CLUSTER,
		      p_cp, /*dyntyp*/ p_slcnf, /*nm*/ p_cp->val.ue.str,
		      p_fsroot, p_children, n_children,
		      p_attrd, p_subdir);
  if (!p_cluster) {
    ErrExit(ErrExit_ASSERT, "rn_mkcname: !p_cluster");
    return NULL;
  }

  /*
   * <cluster> [rtype=RND_CLUSTER, parent=p_rootfs]
   *    |
   *    + "attributes" [RND_ATTRIBUTES, parent=p_rootfs]
   *          |
   *          + attrX...
   */

  /*
   * root - <cluster-name> [RND_CLUSTER] + "jobs" [RND_JOBS]
   *                |                    | 
   *                |                    +-- "partitions" [RND_PARTITIONS]
   *           "attributes"
   *                |
   *                + "SlurmCtldTimeout" [RNF_ATTRIBUTE]
   *                + "SlurmTimeout"     [RNF_ATTRIBUTE]
   *               ...
   */

  p_attrd = rn_cast(p_attrd, RND_ATTRIBUTES,
		    CONFPARAM_MISSINGOK, p_cluster->p_dyntyp,
		    rnode_buildtab[RND_ATTRIBUTES].nm,
		    /*parent*/ p_cluster, /*children*/ NULL, /*n_children*/ 0,
		    /*attr*/ NULL, /*subdir*/ NULL);
  if (!p_attrd) {
    ErrExit(ErrExit_ASSERT, "rn_mkfsroot: rn_cast(attrd) NULL");
    return NULL;
  }

  set_rnparam_cname(p_cluster);
  /*XXXset_rnparam_rn(offsetof(rn_param_t, p_cname), p_cluster);*/
 
  /*
   * insert the sub-directories for partitions and jobs
   */
  p_cp = getconfig_fromnm(rnode_buildtab[RND_PARTITIONS].nm);
  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "rn_mkcname: !p_cp RND_PARTITIONS");
    return NULL;
  }
  p_partitions = rn_cast(p_attrd+1, RND_PARTITIONS,
			 p_cp, /*dyntyp*/ NULL, p_cp->nm,
			 p_cluster, /*children*/ NULL, /*n_children*/ 0,
			 /*attr*/ NULL, /*subdir*/ NULL);
  if (!p_partitions) {
    ErrExit(ErrExit_ASSERT, "rn_mkcname: !p_partitions");
    return NULL;
  }

  p_cp = getconfig_fromnm(rnode_buildtab[RND_JOBS].nm);
  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "rn_mkcname: !p_cp RND_JOBS");
    return NULL;
  }
  p_jobs = rn_cast(p_partitions+1, RND_JOBS,
		   p_cp, /*dyntyp*/ NULL, p_cp->nm,
		   p_cluster, /*children*/ NULL, 0,
		   /*attr*/ NULL, /*subdir*/ NULL);
  if (!p_jobs) {
#if defined(PORTING_TO_SLURMv17)	  
    ErrExit(ErrExit_ASSERT, "rn_mkcname: !p_jobs");
#else    
    ErrExit(ErrExit_WARN, "rn_mkcname: !p_jobs");
#endif
    return NULL;
  }
  return p_cluster;
}
