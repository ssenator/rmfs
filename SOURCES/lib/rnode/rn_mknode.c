#include "rmfs.h"

/*
 * rn_mknoded
 *  construct directory with sub-directory entries corresponding to node names
 *  there are no explicit attributes to the the "nodes" (RND_NODES) directory
 */
rnode_t *
rn_mknoded(rnode_t *p_partition, rnode_t *p_noded) {
  rnode_t          *p_new, *p_children, *p_subdir, *p_rn, *p_nodeX;
  rnode_t        *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t           rn_pro;
  config_param_t   *p_cp, *p_2cp;
  unsigned long     n_children;
  int               i;

  node_info_msg_t      *p_nim;              /* slurm.h */ 
  node_info_t          *p_node_array;       /* slurm.h */
  node_info_t          *p_nai;              /* node array indexer */
  extern rnode_t       *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

  if (!p_partition) {
     ErrExit(ErrExit_ASSERT, "rn_mknoded(NULL parent)");
     return NULL;
  }
  p_cp = dup_cp(getconfig_fromnm(rnode_buildtab[RND_NODES].nm));
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "mknoded: !p_cp");
    return NULL;
  }
  if (!p_cp->per_src.slurm.nim) {
    ErrExit(ErrExit_ASSERT, "mknoded: no slurm node information msg parameter");
    return NULL;
  }

  p_nim                  = p_cp->per_src.slurm.nim;
  p_node_array           = p_nim->node_array;
  p_cp->val.ue.base_addr = (void *) p_node_array;

  if (p_nim->last_update == 0) { /* XXX verify that slurm sets this on OUT */
    ErrExit(ErrExit_ASSERT, "mknoded: j_nim->last_update == 0");
    return NULL;
  }
  if (!p_node_array) {
    ErrExit(ErrExit_ASSERT, "mknoded: empty p_cp->nim->node_array");
    return NULL;
  }
  if (p_nim->record_count == 0) {
    ErrExit(ErrExit_WARN, "mknoded: p_cp->nim->record_count == 0");
  }

  n_children = p_nim->record_count;
   /*
    *  (p_jobd was allocated by the parent's call to the provisioner)
    *   p_new->children = rnode[0], freshly provisioned
    *
    *   rnode[0] = 1st named node
    *   rnode[1] = 2nd named node
    *        ...
    *   rnode[n] = n+1 named node
    *
    * There aren't any particular attribute of the nodes directory.
    */

  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  rn_pro.n_children = n_children + 1;
  if (!(p_new = (*p_buildfn)(p_noded, &rn_pro))) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobd(provision failure)- out of memory?");
    return NULL;
  }

  p_children = p_subdir = p_new;
  
   /* ...  "nodes" [p_parent=RND_PARTNAME, rtype=RND_NODES]
    *            |
    *            + <node1> [p_parent=RND_NODES, rtype=NODENAME]
    *            |
    *            + <node2> [p_parent=RND_NODES, rtype=NODENAME]
    */

  p_noded = rn_cast(p_noded, RND_NODES,
		     p_cp, p_node_array, rnode_buildtab[RND_NODES].nm,
		     p_partition, p_children, n_children,
		     /*attr*/ NULL, p_subdir);

  for (i = 0, p_rn = p_subdir, p_nai = p_node_array;
           i < p_nim->record_count;
               i++, p_nai++
      ) {

    /*
     * each node requires its own p_cp, for its own state
     */
    if (!(p_2cp = dup_cp(p_cp))) {
      ErrExit(ErrExit_ASSERT, "rn_mknoded: out of memory? !dup_cp(p_cp) children");
      return NULL;
    }
     
    /*
     * Notice that the p_dyntyp field is being overwritten from the
     * original provisioning value of p_node_array. It is over-written
     * with the specific node_info_t * for this node
     */
    p_2cp->val.ue.str = p_2cp->nm = p_nai->name;
    p_nodeX = rn_cast(p_rn, RND_NODENAME,
		      p_2cp, p_nai, p_nai->name,
		      p_noded, /*children*/ NULL, 0,
		      /*attr*/ NULL, /*subdir*/ NULL);
    if (!p_nodeX) {
      ErrExit(ErrExit_ASSERT, "rn_mknoded: !p_rn");
      return NULL;
    }
    p_rn++;
  }
  return p_noded;
}

/*
 * rn_mknode
 *  construct per-node directory for an individual named node
 */
rnode_t *
rn_mknode(rnode_t *p_noded, rnode_t *p_nodeX) {
  rnode_t        *p_new, *p_children, *p_subdir, *p_attr, *p_attrd;
   rnode_t       *(*p_buildfn)(rnode_t *, rnode_t *);
   rnode_t         rn_pro;
   unsigned long   n_children;
   int             n_attr;
   config_param_t *p_cp, *p_2cp;
   
   node_info_t    *p_ni;   /* slurm.h */

  extern int       attr_cnt_cp(rn_type_t); /*rn_subr.c*/
   extern rnode_t *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

   if (!p_noded) {
     ErrExit(ErrExit_ASSERT, "rn_mknode(NULL parent)");
     return NULL;
   }

   if (!p_nodeX->p_cp) {
       ErrExit(ErrExit_ASSERT, "rn_mknode: !p_nodeX->p_cp");
       return NULL;
   }
   p_cp = p_nodeX->p_cp;

   /*
    * slurm info node array index ptr was squirreled away by buildfn(RND_NODENAME)
    */
   p_ni = (node_info_t *) p_nodeX->p_dyntyp;
   if (!p_ni) {
     ErrExit(ErrExit_ASSERT, "rn_mknode: !p_ni");
     return NULL;
   }

   n_children = 0;
   n_attr     = attr_cnt_cp(RND_NODENAME);
   if (n_attr > 0) {
     n_children++;
   }
  
   /*
    *  (p_nodeX was allocated by the parent's call to the provisioner)
    *   p_new->children = rnode[0], freshly provisioned
    *
    *   rnode[0] = "attributes" sub-directory, if there are any
    */

   p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
   rn_pro.n_children = n_children + 1;
   if (!(p_new = (*p_buildfn)(p_nodeX, &rn_pro))) {
     ErrExit(ErrExit_ASSERT, "rn_mknode: out of memory? provision failure");
     return NULL;
   }

   p_children = p_attr = p_subdir = p_new;
   p_nodeX    = rn_cast(p_nodeX, RND_NODENAME,
			p_cp, p_ni, p_nodeX->nm,
			p_noded, p_children, n_children,
			/*attr*/ NULL, p_subdir);
   if (!p_nodeX) {
     ErrExit(ErrExit_ASSERT, "rn_mknode: rn_cast(p_nodeX) return NULL");
     return NULL;
   }

   if (n_attr > 0) {
   /* ...  <nodeX> [p_parent=RND_NODES, rtype=RND_NODENAME]
    *            |
    *            + "attributes" [p_parent=p_nodeX, rtype=RND_ATTRIBUTES]
    */
     p_2cp = dup_cp(p_noded->p_cp);
     p_2cp->nm = rnode_buildtab[RND_ATTRIBUTES].nm;
     p_attrd = rn_cast(p_children, RND_ATTRIBUTES,
		       p_2cp, /*dyntyp*/ p_ni, p_2cp->nm,
		       p_nodeX, /*children*/ NULL, 0,
		       /*attr*/ NULL, /*subdir*/ NULL);
     if (!p_attrd) {
       ErrExit(ErrExit_ASSERT, "rn_mknode: rn_cast_attr_typconv() = !p_attrd");
       return NULL;
     }
   }
   return p_nodeX;
}


/*
 * the nodestate attribute corresponds to the resource manager's
 * node_info.node_state field and is presented as a directory
 * to show its sub-attributes
 */
#define MAX_STATE_STR_LEN (7)

rnode_t *
rn_mknstate(rnode_t *p_parent, rnode_t *p_nodeX) {
  rnode_t            *p_children, *p_attr, *p_new, *p_nstate, *p_allocjob;
  rnode_t          *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t             rn_pro;
  config_param_t     *p_cp, *p_2cp;
  unsigned long       n_children;
  int                 node_allocated;
  uint16_t            node_state, *p_nodestate; /*as in slurm.h*/
  char                node_state_str[MAX_STATE_STR_LEN+1];

  extern rnode_t *rn_cast_attr_typconv(rnode_t *, ptyp_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

  if (!p_parent) {
    ErrExit(ErrExit_ASSERT, "rn_mknstate(NULL parent)");
    return NULL;
  }
  if (!p_nodeX) {
    ErrExit(ErrExit_ASSERT, "rn_mknstate(NULL p_nodeX)");
    return NULL;
  }
  p_cp = p_nodeX->p_cp;
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "mknstate: !p_cp");
    return NULL;
  }
  if (!p_nodeX->p_cp->per_src.slurm.nim && !p_nodeX->p_dyntyp) {
    ErrExit(ErrExit_ASSERT, "mknstate: !p_nodeX->p_cp->per_src.slurm.nim && !p_dyntyp");
    return NULL;
  }
  /*
   * parent node's dyntyp ptr = node_info struct
   * the node_info contains the node status
   */
  node_allocated = 0;

  /* XXX ASSERT(slurm API #) */
  p_nodestate =  (uint16_t *) p_nodeX->p_dyntyp;
  node_state  = *p_nodestate;
  if (node_state & NODE_STATE_ALLOCATED) {
    node_allocated++;
  }

  /*
   *  (p_nodeX was allocated by the parent's call to the provisioner)
   *   p_new->children = rnode[0], state attribute
   *
   *   rnode[1] = allocjob symlink
   *
   * XXX ASSERT resource mgr allocates nodes exclusively
   */
  n_children = 1;
  n_children += node_allocated;
  
  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  rn_pro.n_children = n_children + 1;
  if (!(p_new = (*p_buildfn)(p_parent, &rn_pro))) {
    ErrExit(ErrExit_ASSERT, "rn_mknstate(provision failure)- out of memory?");
    return NULL;
  }
  p_children = p_attr = p_new;

  /*
   *  ...  "state" [p_parent=<variable> rtype=RND_NSTATE ]
   *            |
   *            + named by their state (provided by the resource mgr)
   *            |  {down|idle|alloc|error} [p_parent=p_nstate, rtype=RNF_ATTR]
   *            |
   *    [if state == alloc]
   *            |
   *            + allocjob [p_parent=nstate, rtype=RNF_ALLOCJOB]
   *                          XXXFUTURE: symlink pointing to jobidX
   *                          XXXFUTURE: bind mount to jobidX
   *
   * nstate nodes have no sub-directories
   *
   */

  /*
   * XXX ASSERT specific resource manager api, as a dependency
   * XXXFUTURE: use resource manager select & format functions
   */
  (void) snprintf(node_state_str, /*size_t*/ MAX_STATE_STR_LEN, "%s",
		  (node_allocated?                      "alloc" :
		   (node_state & NODE_STATE_DOWN?       "down" :
		    (node_state & NODE_STATE_IDLE?      "idle" :
		     (node_state & NODE_STATE_ERROR?    "error" :
		      (node_state & NODE_STATE_MIXED?   "mixed" :
		       (node_state & NODE_STATE_FUTURE? "future" :
			                                "unknown")))))
		   )
		  );
  p_2cp = dup_cp(p_cp);
  if (!derefable_cp(p_2cp)) {
    ErrExit(ErrExit_ASSERT, "rn_mknstate: no memory - !derefable_cp(p_cp)");
    return NULL;
  }
  p_2cp->nm = strdup(node_state_str);
  p_nstate = rn_cast_attr_typconv(p_children, PTYP_NODESTATE,
				  p_2cp, /*dyntyp*/ p_nodestate, p_2cp->nm,
				  p_nodeX, /*children*/ NULL, 0,
				  /*attr*/ NULL, /*subdir*/ NULL);

  if (!p_nstate) {
    ErrExit(ErrExit_ASSERT, "rn_mknstate(cast attr) node_state");
    return NULL;
  }

  if (node_allocated) {
    p_allocjob = rn_cast_attr_typconv(p_children+1, PTYP_ALLOCJOB,
				      p_2cp, /*dyntyp*/ p_nodestate,
				      rnode_buildtab[RNF_ALLOCJOB].nm,
				      p_nodeX, /*children*/ NULL, 0,
				      /*attr*/ NULL, /*subdir*/ NULL);
      if (!p_allocjob) {
	ErrExit(ErrExit_ASSERT, "rn_mknstate: !p_allocjob");
	return NULL;
      }
  }

  return p_nstate;
}

/*
 * the allocjob attribute ought to be a symbolic link (bind mount?)
 * to the jobid node that this node has been allocated to
 *
 * XXXfuture no symlinks in a fuse implementation
 *
 * its child node is a pre-existing jobid node, unlike others
 */
rnode_t *
rn_mkallocjob(rnode_t *p_nstate, rnode_t *p_allocjob) {
  rnode_t        *p_jobidX, *p_jobd;
  int             h_pos, n_jobs;
  char           *h_nm, *nodenm, *nodenm2;
  config_param_t *p_cp;
  rn_param_t     *p_rn_paramtab;

  /*slurm.h*/
  slurm_job_info_t      *p_ji;
  hostlist_t       hl;

  extern rnode_t *rn_cast_attr_typconv(rnode_t *, ptyp_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

  if (!p_nstate) {
    ErrExit(ErrExit_ASSERT, "rn_mkallocjob(NULL parent nstate)");
    return NULL;
  }

  p_cp = p_nstate->p_cp;
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "mkallocjob: !p_cp");
    return NULL;
  }
  /*
   * that is, p_allocjob's parent's parent
   */
  if (!p_nstate->parent) {
    ErrExit(ErrExit_ASSERT, "mkallocjob: !p_nstate->p_parent");
    return NULL;
  }

  nodenm = (char *) p_nstate->parent->p_dyntyp;
  if (!nodenm) {
    ErrExit(ErrExit_ASSERT, "mkallocjob: p_nstate->p_parent->nodenm");
    return NULL;
  }
  /*
   * walk the jobs, is this host's name present in this job's hostlist?
   * XXXFUTURE: a lot of room for optimization improvement
   */
#ifdef FIX_RNPARM_LOCKING
  p_jobd = get_rnparam_rn(offsetof(rn_param_t, p_jobd));
#else
  {
    p_rn_paramtab = get_rn_params(/*needlock*/ FALSE);
    if (!p_rn_paramtab) {
      return FALSE;
    }
    p_jobd = p_rn_paramtab->p_jobd;
  };
#endif  

  if (!p_jobd) {
    ErrExit(ErrExit_ASSERT, "mkallocjob: p_jobd");
    return NULL;
  }
  
  for (n_jobs = 0, p_jobidX = p_jobd->children, h_nm = NULL;
	     n_jobs < p_jobd->n_children && p_jobd->nm && p_jobd && !h_nm;
	         n_jobs++, p_jobidX++) {
    int i;

    p_ji = (slurm_job_info_t *) p_jobidX->p_dyntyp;
    if (!p_ji) {
      ErrExit(ErrExit_ASSERT, "rn_mkallocjob NULL p_ji ptr");
      continue;
    }
    if (!(hl = slurm_hostlist_create(p_ji->nodes))) {
      ErrExit(ErrExit_ASSERT, "rn_mkallocjob NULL p_ji ptr");
      continue;
    }
    
    /* ASSERT SLURM API version */
    /* hostlist contains nm, position is h_pos */
    if ((h_pos = slurm_hostlist_find(hl, nodenm)) < 0) {
      /*notfound*/
      slurm_hostlist_destroy(hl);
      continue;
    }
    for (i = 0; i < h_pos-1; i++) {
      if (!slurm_hostlist_shift(hl)) {
	ErrExit(ErrExit_ASSERT, "rn_mkallocjob !slurm_hostlist_shift(h_pos-1)");
      }
    }
    if (!(h_nm = slurm_hostlist_shift(hl))) {
      ErrExit(ErrExit_ASSERT, "rn_mkallocjob !slurm_hostlist_shift (h_pos)");
      slurm_hostlist_destroy(hl);
      continue;
    }
    nodenm2 = strdup(h_nm);
    if (!nodenm) {
      ErrExit(ErrExit_ASSERT, "rn_mkallocjob !strdup(h_nm)");
      break;
    }
    p_allocjob = rn_cast_attr_typconv(p_allocjob, RNF_ALLOCJOB,
				      CONFPARAM_MISSINGOK, nodenm2, nodenm2,
				      p_nstate, /*children*/ NULL, 0,
				      /*attr*/ NULL, /*subdir*/ NULL);

    if (!p_allocjob) {
      ErrExit(ErrExit_ASSERT, "rn_mkallocjob: !p_allocjob");
      continue;
    }
  }

  if (hl) {
    slurm_hostlist_destroy(hl);
  }
  if (h_nm) {
    free(h_nm);
  }

  return p_allocjob;
}
