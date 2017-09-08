#include "rmfs.h"


/*
 * rn_mkpartd
 *  construct directory with sub-directory entries corresponding to partitions
 *  there are no explicit attributes to the "partitions" (RND_PARTITIONS) directory
 */
rnode_t *
rn_mkpartd(rnode_t *p_cluster, rnode_t *p_partitions) {
  rnode_t         *p_children, *p_subdir, *p_rn, *p_new, *p_partX;
  rnode_t      *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t         rn_pro;
  config_param_t *p_cp, *p_2cp;
  unsigned long   n_children;
  int             i;

  partition_info_msg_t *p_pim;        /* slurm.h */ 
  partition_info_t     *p_part_array; /* slurm.h */
  partition_info_t     *p_pai;        /* "partition array indexer" */

  extern rnode_t       *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);
   
  if (!p_cluster) {
    ErrExit(ErrExit_ASSERT, "rn_mkpartd(NULL parent p_cluster)");
    return NULL;
  }
  
  p_cp = getconfig_fromnm(rnode_buildtab[RND_PARTITIONS].nm);
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "mkpartd: no partitions configuration parameter");
    return NULL;
  }
  if (!p_cp->per_src.slurm.pim) {
    ErrExit(ErrExit_ASSERT, "mkpartd: no slurm partition information msg parameter");
    return NULL;
  }

  /* store the partition array and the partition info msg */
  p_pim                  = p_cp->per_src.slurm.pim;
  p_part_array           = p_pim->partition_array;
  p_cp->val.ue.base_addr = (void *) p_part_array;

  if (p_pim->last_update == 0) {
    ErrExit(ErrExit_ASSERT, "mkpartd: p_pim->last_update == 0");
    return NULL;
  }
  if (!p_part_array) {
    ErrExit(ErrExit_ASSERT, "mkpartd: empty p_cp->slurm.pim->partition_array");
    return NULL;
  }
  if (p_pim->record_count == 0) {
    ErrExit(ErrExit_WARN, "mkpartd: p_cp->slurm.pim->record_count == 0");
  }
  n_children = p_pim->record_count;

  /*
   *  (p_partitions was allocated by the parent's call to the provisioner)
   *   p_new->children = rnode[0], freshly provisioned
   *
   *   rnode[0] = 1st named partition
   *   rnode[1] = 2nd named partition
   *        ...
   *   rnode[n] = n+1 named partition 
   */

  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  rn_pro.n_children = n_children + 1;
  if (!(p_new = (*p_buildfn)(p_partitions, &rn_pro))) {
    ErrExit(ErrExit_ASSERT, "rn_mkpartd(provision failure)- out of memory?");
    return NULL;
  }

  p_children = p_subdir = p_new;
  /* ...  "partitions" [p_parent=RND_CLUSTER, rtype=RND_PARTITIONS]
   *            |
   *            + <partition1> [p_parent=RND_PARTITIONS, rtype=PARTNAME, dyntyp= *partition_info_t]
   *            |
   *            + <partition2> [p_parent=RND_PARTITIONS, rtype=PARTNAME]
   */
  p_partitions = rn_cast(p_partitions, RND_PARTITIONS,
			 CONFPARAM_REQUIRED, p_part_array, p_cp->nm,
			 p_cluster, p_children, n_children,
			 /*attr*/ NULL, p_subdir);
  if (!derefable_cp(p_partitions->p_cp)) {
    ErrExit(ErrExit_ASSERT, "rn_mkpartd: NULL p_partitions->p_cp");
    return NULL;
  }

  for (i = 0, p_rn = p_subdir, p_pai = p_part_array;
           i < p_pim->record_count;
               i++, p_pai++
       ) {

    if (!p_pai->name) {
      ErrExit(ErrExit_ASSERT, "rn_mkparted: !p_pai->name");
      return NULL;
    }
    if (!(p_2cp = dup_cp(p_cp))) {
      ErrExit(ErrExit_ASSERT, "mkpartd: !dup_cp(p_cp) children");
      return NULL;
    }
    p_2cp->val.ue.str = p_2cp->nm = p_pai->name;
    p_partX = rn_cast(p_rn, RND_PARTNAME,
		      p_2cp, p_pai, p_pai->name,
		      p_partitions, /*children*/ NULL, 0,
		      /*attr*/ NULL, /*subdir*/ NULL);
    if (!p_partX) {
      ErrExit(ErrExit_ASSERT, "rn_mkparted: !p_rn");
      return NULL;
    }
    p_rn++;
  }
  return p_partitions;
}

/*
 * rn_mkpart
 *  construct per-partition directory with
 *      "nodes" and "attributes subdirectories
 */
rnode_t *
rn_mkpart(rnode_t *p_partitions, rnode_t *p_partX) {
  rnode_t          *p_new, *p_nodes, *p_children, *p_subdir, *p_attrd;
  rnode_t        *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t           rn_pro;
  unsigned long     n_children, n_attr;
  config_param_t   *p_cp, *p_2cp;

  partition_info_t *p_pi;   /* slurm.h */
  extern int        attr_cnt_cp(rn_type_t); /*rn_subr.c*/
  extern rnode_t   *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

  if (!p_partitions) {
    ErrExit(ErrExit_ASSERT, "rn_mkpart(NULL parent)");
    return NULL;
  }
  p_cp = p_partX->p_cp;
  p_pi = (partition_info_t *) p_partX->p_dyntyp;
  if (!p_pi) {
    ErrExit(ErrExit_ASSERT, "mkpart: !partition info ptr");
    return NULL;
  }
  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "mkpart: !p_partX->p_cp");
    return NULL;
  }

  n_children = 1; /* RND_NODES */
  n_attr     = attr_cnt_cp(RND_PARTNAME);
  
  if (n_attr > 0) { /* RND_ATTRIBUTES */
    n_children++;
  }
  
  /*
   *  (p_part was allocated by the parent's call to the provisioner)
   *   p_new->children = rnode[0], freshly provisioned
   *
   *   rnode[0]   = "nodes" subdirectory
   *   rnode[1]   = "attributes" subdirectory, if needed
   */

  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  rn_pro.n_children = n_children + 1;
  if (!(p_new = (*p_buildfn)(p_partX, &rn_pro))) {
    ErrExit(ErrExit_ASSERT, "rn_mkpart(provision failure)- out of memory?");
    return NULL;
  }

  p_subdir = p_children = p_new;
  p_partX = rn_cast(p_partX, RND_PARTNAME,
		    p_cp, /*dyntyp*/ p_pi, p_partX->nm,
		    p_partitions, p_children, n_children,
		    /*attr*/NULL, p_subdir);
  
  /* ...  <partitionX> [p_parent=RND_PARTITIONS, rtype=RND_PARTNAME]
   *            |
   *            + "nodes" [p_parent=p_partition, rtype=RND_NODES]
   *            |
   *            + "attributes" [p_parent=p_partition, rtype=RND_ATTRIBUTES]
   */

  p_nodes = rn_cast(p_children, RND_NODES,
		    CONFPARAM_REQUIRED, /*dyntyp*/ NULL,
		    rnode_buildtab[RND_NODES].nm,
		    p_partX, /*children*/ NULL, 0,
		    /*attr*/ NULL, /*subdir*/ NULL);
  if (!p_nodes) {
    ErrExit(ErrExit_ASSERT, "rn_mkpart: !p_nodes");
    return NULL;
  }

  if (n_attr > 0) {
    p_2cp = dup_cp(p_cp);
    if (!derefable_cp(p_cp)) {
      ErrExit(ErrExit_ASSERT, "rn_mkpart: no memory - dup_cp(p_cp)");
      return NULL;
    }
    p_2cp->nm = rnode_buildtab[RND_ATTRIBUTES].nm;
    p_attrd = rn_cast(p_nodes+1, RND_ATTRIBUTES,
		      p_2cp, /*dyntyp*/ p_pi, p_2cp->nm,
		      p_partX, /*children*/ NULL, 0,
		      /*attr*/ NULL, /*subdir*/ NULL);
    if (!p_attrd) {
      ErrExit(ErrExit_ASSERT, "rn_mkpart: !p_nodes");
      return NULL;
    }
  }
  return p_partX;
}
