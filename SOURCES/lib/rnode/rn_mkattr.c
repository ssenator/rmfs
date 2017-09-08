#include "rmfs.h"

/*
 * rn_mkattrd
 *  construct a directory to hold attributes (RND_ATTRIBUTES)
 *  these can appear in multiple places in the file tree,
 *   esp: RND_ROOT, RND_CLUSTER, RND_PARTNAME, RND_JOBID, RND_JOBSTEPID
 */
rnode_t *
rn_mkattrd(rnode_t *p_parent, rnode_t *p_attrd) {
  rnode_t        *p_new, *p_children, *p_rn, *p_attr, *p_rntab;
  rnode_t         rn_pro;
  rnode_t       *(*p_buildfn)(rnode_t *, rnode_t *);
  int             n_attr, n_children, i;
  config_param_t *p_table_cp, *p_cp, *p_2cp;
  rn_type_t       rtype;
  char           *nm;
  config_param_t *collectslurm_attr(rn_type_t, config_param_t *, void *);
  
  extern rnode_t *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);
  extern rnode_t *rn_cast_attr_typconv(rnode_t *, ptyp_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

  if (!p_parent) {
    ErrExit(ErrExit_ASSERT, "rn_mkattrd: !p_parent");
    return NULL;
  }
  if (!p_attrd) {
    ErrExit(ErrExit_ASSERT, "rn_mkattrd: !p_attrd");
    return NULL;
  }
  if (!IS_RTYPE_VALID(p_parent->rtype)) {
    ErrExit(ErrExit_ASSERT, "rn_mkattrd: !valid rtype");
    return NULL;
  }

  rtype   = p_parent->rtype;
  p_rntab = &rnode_buildtab[rtype];
  if (!p_rntab->attr_desc.table) {
    ErrExit(ErrExit_ASSERT, "rn_mkattrd: !table for rtype");
    return NULL;
  }

  p_cp = p_parent->p_cp;
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "rn_mkattrd: !p_parent->p_cp");
    return NULL;
  }
  p_table_cp = p_rntab->attr_desc.table;
  for (n_attr = 0, p_cp = p_table_cp;
           p_cp && p_cp->nm;
               p_cp++) {

    /* no claim function implies always claiming all table entries */
    if (!p_rntab->attr_desc.is_mine) {
      n_attr++;
      
    } else {
      if ((*p_rntab->attr_desc.is_mine)(p_cp)) {
	n_attr++;
      }
    }
  }
  n_children = n_attr;

  if (n_children <= 0) {
    ErrExit(ErrExit_WARN, "rn_mkattrd: attribute count = 0"); /*ErrExit_INTERNAL*/
  }

  /*
   *  (p_attrd was allocated by the parent's call to the provisioner)
   *   p_new->children = rnode[0], freshly provisioned
   *
   *   rnode[0] = 1st named attribute
   *   rnode[1] = 2nd named attribute
   *        ...
   *   rnode[n-1] = n-th named attribute
   *
   */

   p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
   rn_pro.n_children = n_children + 1;
   if (!(p_new = (*p_buildfn)(p_parent, &rn_pro))) {
     ErrExit(ErrExit_ASSERT, "rn_mknode: out of memory? provision failure");
     return NULL;
   }
   p_children = p_attr = p_new;

   /* ...  "attributes" [p_parent=<varies>, rtype=RND_ATTRIBUTES]
    *            |
    *            + <attr1> [p_parent=RND_ATTRIBUTES, rtype=RNF_ATTRIBUTE]
    *            |
    *            + <attr2> [p_parent=RND_ATTRIBUTES, rtype=RNF_ATTRIBUTE]
    *           ...
    */
   p_2cp = dup_cp(p_cp);
   p_2cp->nm = nm = strdup(rnode_buildtab[RND_ATTRIBUTES].nm);
   p_attrd = rn_cast(p_attrd, RND_ATTRIBUTES,
		    p_2cp, p_attrd->p_dyntyp, nm,
		    p_parent, p_children, n_children,
		    /*attr*/ NULL, /*subdir*/ NULL);
   
  for (i = 0, p_cp = p_table_cp, p_rn = p_attr;
           i < n_children && p_cp && p_cp->nm;
               p_cp++) {

    /* there is a claim function, but it does not claim this parameter, skip it */
    if (p_rntab->attr_desc.is_mine && !(*p_rntab->attr_desc.is_mine)(p_cp)) {
      continue;
    }

    p_2cp = dup_cp(p_cp);

    if (p_cp->per_src.slurm.dynamic) {
      if (!(p_2cp = collectslurm_attr(rtype, p_cp, p_attrd->p_dyntyp))) {
	ErrExit(ErrExit_ASSERT, "rn_mkattrd: collectslurm_attr");
	return NULL;
      }
    }

    p_rn = rn_cast_attr_typconv(p_rn, p_2cp->typ,
				p_2cp, p_attrd->p_dyntyp, p_2cp->nm,
				p_attrd, /*children*/ NULL, 0,
				/*attr*/ NULL, /*subdir*/ NULL);
    if (!p_rn) {
      ErrExit(ErrExit_ASSERT, "mk_attrd: rn_cast_attr_typconv() = !p_rn");
      return NULL;
    }
    p_rn++; i++;
  }
  return p_attrd;
}

/*
 * rn_mkattr
 *  construct leaf-node (ISFILE()) with contents according to the p_cp
 */
rnode_t *
mkattr_knob(rnode_t *p_knob) {

  if (!p_knob) {
    return NULL;
  }
  if (!p_knob->p_cp) {
    return NULL;
  }
  p_knob->p_cp->typ = PTYP_OPAQUE;
  return p_knob;
}

/*
 * accessor function to retrieve the signer function associated with the "RNF_SIGNATURE" ctl command
 *
 * given an rnode *p_watched,
 * returns ptr to a function that implements the "sign" function
 * the function referenced & returned takes as its args an rnode_t * and a ctl_arg_t and returns a tri_t
 */
tri_t 
(*get_rn_signfn(rnode_t *p_watched))(rnode_t *, config_param_t *) {
  rn_ctl_t *p_cmd;
  tri_t   (*p_signfn)(rnode_t *, config_param_t *);
  int       i, h;

  if (!p_watched) {
    ErrExit(ErrExit_ASSERT, "get_rn_signfn: !p_watched");
    return NULL;
  }

  h = djb_strtohash("sign");
  if (!IS_VALID_HASH(h)) {
    ErrExit(ErrExit_ASSERT, "get_rn_signfn: !IS_VALID_HASH(\"sign\")");
    return NULL;
  }

  for (p_signfn = NULL, p_cmd = p_watched->ctl, i = 0;
           !p_signfn && p_cmd && p_cmd->nm && i < MAX_RN_CMDS;
               p_cmd++, i++) {
    
    if (p_cmd->rtype != RNF_SIGNATURE) {
      continue;
    }
    if (djb_strtohash(p_cmd->nm) != h) {
      ErrExit(ErrExit_ASSERT, "get_rn_signfn: sig func hash mismatch");
      return NULL;
    }
    p_signfn = p_cmd->fn;
  }
  
  if (!p_signfn) {
    ErrExit(ErrExit_ASSERT, "get_rn_signfn: !p_signfn found");
    return NULL;
  }
  return p_signfn;
}

rnode_t *
mkattr_sig(rnode_t *p_sig) {
  rnode_t        *p_watched;
  tri_t         (*p_sign_fn)(rnode_t *, config_param_t *);

  if (!p_sig) {
    ErrExit(ErrExit_ASSERT, "mkattr_sig: !p_sig");
    return NULL;
  }
  /*
   * signature nodes are actually the signature of the grandparent and its attributes
   * p_watched is the grandparent; p_sig is this signature attribute node
   * and is made visible as an extended attribute
   */

  /*
   * notify grandparent that this signature rnode requests notification when
   * the grandparent is modified (to regenerate the signature)
   */
  if (!p_sig->parent || !p_sig->parent->parent) {
    ErrExit(ErrExit_ASSERT, "mkattr_sig: cannot find parent to notify");
    return NULL;
  }

  p_watched = p_sig->parent->parent;
  p_sign_fn = get_rn_signfn(p_watched);
  if (!p_sign_fn) {
    ErrExit(ErrExit_ASSERT, "mkattr_sig: no sign control function");
    return NULL;
  }

  if (!p_sig->p_cp) {
    p_sig->p_cp = calloc(1, sizeof(config_param_t));
    if (!p_sig->p_cp) {
      ErrExit(ErrExit_ASSERT, "mkattr_sig: p_cp_sig alloc failed");
      return NULL;
    }
  }

  if (TRUE != (*p_sign_fn)(p_sig, p_sig->p_cp)) {
    ErrExit(ErrExit_ASSERT, "mkattr_sig: !p_sign_fn(p_sig, p_sig->p_cp)");
    return NULL;
  }

  p_watched->signature   = p_sig;
  p_watched->maybe.dirty = TRUE;

  /* p_sig becomes the new list head for the p_watched notify list */
  p_sig->maybe.notify     = p_watched->maybe.notify; /* => rn_poll */
  p_watched->maybe.notify = p_sig;
    
  return p_sig;
}

/*
 * mkattr_xattr_ctx()
 *  create extended attribute config_param_t in this rnode
 *  usually called from rn_cast(), also by the RNF_CONTEXT attribute node
 */
config_param_t *
mkattr_xattr_ctx(rnode_t *p_rn, char *ctx_str) {
  config_param_t *p_cp_defcontext, *p_cp_context, *p_2cp;
  int             l, hash_jobidctx, hash_ctx;

  if (!p_rn) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !p_rn");
    return NULL;
  }
  if (!p_rn->parent) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !p_rn->parent");
    return NULL;
  }
  if (!p_rn->parent->parent) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !p_rn->parent->parent");
    return NULL;
  }
  if (!IS_RTYPE_VALID(p_rn->parent->parent->rtype)) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !IS_VALID_RTYPE(p_rn->parent->parent)");
    return NULL;
  }
  
  /*
   * initialized with:
   *  value passed in as context (ctx_str)
   *  p_cp_context    - if available, else
   *  p_cp_defcontext - else
   *  dummy entry
   *
   * also link this entry to the extended attribute of
   *  1. this node
   *  2. this node's parent's parent, presumably the jobid entry
   */
  p_cp_defcontext = getconfig_fromnm("DefContext");
  p_cp_context    = getconfig_fromnm("context");

  if (!p_cp_defcontext) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: RNF_CONTEXT !defcontext");
    return NULL;
  }

  if (ctx_str) {
    p_2cp = dup_cp(p_cp_defcontext);
    if (p_2cp) {
      p_2cp->nm = strdup(CTX_XATTR_NM); /* ie. "security.selinux" */
      p_2cp->h  = 0;
    }
  
  } else if (p_cp_context) {
    p_2cp = dup_cp(p_cp_context);

  } else if (p_cp_defcontext) {
    p_2cp = dup_cp(p_cp_defcontext);
  }
  if (!p_2cp) {
    ErrExit(ErrExit_ASSERT, "mkattr: !p_2cp");
    return NULL;
  }

  init_hash_cp(p_2cp);
  if (!IS_VALID_HASH(p_2cp->h)) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !invalid hash(CTX_XATTR_NM)");
    return NULL;
  }

  hash_jobidctx = djb_strtohash(CTX_JOBID_DEFAULT);
  if (!IS_VALID_HASH(hash_jobidctx)) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !invalid hash(CTX_JOBID)");
    return NULL;
  }
  hash_ctx = djb_strtohash(ctx_str);
  if (!IS_VALID_HASH(hash_ctx)) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !invalid hash(ctx_str)");
    return NULL;
  }
  
  if (hash_ctx == hash_jobidctx) {
    if (p_rn->rtype == RNF_ATTRIBUTE &&
	(p_rn->parent->parent->rtype != RND_JOBID && p_rn->parent->parent->rtype != RND_JOBSTEPID)) {
      ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: jobid context but grandparent !RND_JOBID | RND_JOBSTEPID");
      return NULL;
    }
  }

  p_2cp->val.ue.str = strdup(ctx_str); /*ex. "system_u:object_r:slurm_job_t" */
  if (!p_2cp->val.ue.str) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !p_2cp->str");
    return NULL;
  }
  l = internal_strlen(p_2cp->val.ue.str);
  if (l <= 0) {
    ErrExit(ErrExit_ASSERT, "mkattr_xattr_ctx: !strlen(p_2cp->val.ue.str)");
    return NULL;
  }

  p_2cp->val.size    = l + 1;
  /* link rnode's xattr to this p_cp, trying not to clobber any previous xattrs (unlikely) */
  if (p_rn->xattr) {
    p_2cp->p_nxt = p_rn->xattr;
  }
  p_rn->xattr    = p_2cp;
  p_rn->n_xattr += 1;
  
  /* return p_cp, in case caller needs to further process it */
  return p_2cp;
}

rnode_t *
rn_mkattr(rnode_t *p_parent, rnode_t *p_attr) {
  config_param_t *p_cp;
  rnode_t        *p_rn;

  extern rnode_t *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);
  extern rnode_t *rn_cast_attr_typconv(rnode_t *, ptyp_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);
   
  if (!p_parent) {
    ErrExit(ErrExit_ASSERT, "rn_mkattr(NULL parent)");
    return NULL;
  }
  if (!p_attr) {
    ErrExit(ErrExit_ASSERT, "rn_mkattr(NULL p_attr)");
    return NULL;
  }
  if (!IS_RTYPE_ATTRIBUTE(p_attr->rtype)) {
    ErrExit(ErrExit_ASSERT, "rn_mkattr: p_attr->rtype != IS_RTYPE_ATTRIBUTE");
    return NULL;
  }
  p_cp = p_attr->p_cp;
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "mkattr: !p_cp");
    return NULL;
  }

  /*
   * p_attr was allocated by the parent's call to the provisioner
   * generic attribute nodes have are leaf nodes without children;
   * so, no children to provision
   *
   * but, post-allocation processing is needed to link these to their
   * source config_param_t *
   */

  /*
   * "p_attr" [p_parent=<VARIABLE>, rtype=RNF_ATTRIBUTE |
   *                                      RNF_KNOB      |
   *					  RNF_CONTEXT   |
   *					  RNF_SIGNATURE ]
   */

   p_rn = rn_cast_attr_typconv(p_attr, p_attr->p_cp->typ,
			       p_attr->p_cp, p_attr->p_dyntyp, p_attr->nm,
			       p_parent, p_attr->children, p_attr->n_children,
			       p_attr->attr, p_attr->subdir);
   if (!p_rn) {
     ErrExit(ErrExit_ASSERT, "rn_mkattr: rn_cast_attr_typconv() = !p_rn");
     return NULL;
   }

   if (RNF_KNOB == p_attr->rtype) {
     if (!mkattr_knob(p_attr)) {
       ErrExit(ErrExit_ASSERT, "rn_mkattr: !mkattr_knob()");
       return NULL;
     }
   } else if (RNF_SIGNATURE == p_attr->rtype) {
     if (!mkattr_sig(p_attr)) {
       ErrExit(ErrExit_ASSERT, "rn_mkattr: !mkattr_sig()");
       return NULL;
     }
   } else if (RNF_CONTEXT == p_attr->rtype) {
     char           *p_ctx_str;
     config_param_t *p_cp;
     
     /*
      * jobids have special context to allow the transition to the user's context
      */
     p_ctx_str = (RND_JOBID == p_attr->parent->parent->rtype)? CTX_JOBID_DEFAULT: CTX_DEFAULT;

     if (!(p_cp = mkattr_xattr_ctx(p_attr, p_ctx_str))) {
       ErrExit(ErrExit_ASSERT, "rn_mkattr: !mkattr_xattr_ctx(RNF_CONTEXT)");
       return NULL;
     }
     p_attr->p_cp = p_cp;

   } else if (RNF_ATTRIBUTE == p_attr->rtype) {
     int h_jobid = djb_strtohash("job_id");
     
     if (!IS_VALID_HASH(h_jobid)) {
       ErrExit(ErrExit_ASSERT, "rn_mkattr: RNF_ATTRIBUTE: job_id");
       return NULL;
     }
     if (h_jobid == p_attr->h) {
       if (!mkattr_xattr_ctx(p_attr, CTX_JOBID_DEFAULT)) {
	 ErrExit(ErrExit_ASSERT, "rn_mkattr: !mkattr_xattr_ctx(RNF_ATTRIBUTE job_id)");
	 return NULL;
       }
     } else {
       ; /*no special xattr processing*/
     }

   } else {
     ErrExit(ErrExit_ASSERT, "mkattr: unknown attribute type");
     return NULL;
   }
   return p_attr;
}


/*
 * getconfig_from_myattrnm()
 *  walks an attribute directory, searching for a match on one of the attributes
 *  with the name supplied
 *
 * support routine which needs to know rnode_t and config_param_t
 * falls squarely between rnode-specific (rmfs_rnode.c) and conf-specific (rmfs_conf.c)
 */

config_param_t *
getconfig_from_myattrnm(rnode_t *p_attrd, char *attr_nm) {
  int             attr_hash, i;
  rnode_t        *p_rn;
  config_param_t *p_cp_match;
  
  if (!p_attrd) {
    ErrExit(ErrExit_INTERNAL, "getconfig_from_myattrnm: !p_attrd");
    return NULL;
  }
  if (!attr_nm) {
    ErrExit(ErrExit_INTERNAL, "getconfig_from_myattrnm: !attr_nm");
    return NULL;
  }
  attr_hash = djb_strtohash(attr_nm);
  if (!IS_VALID_HASH(attr_hash)) {
    ErrExit(ErrExit_INTERNAL, "getconfig_from_myattrnm: !attr_hash");
    return NULL;
  }
  p_cp_match = NULL;
  for (i = 0, p_rn = p_attrd->children;
           i < p_attrd->n_children && !p_cp_match;
               i++, p_rn++) {

    if (!IS_RTYPE_VALID(p_rn->rtype)) {
      ErrExit(ErrExit_ASSERT, "getconfig_from_myattrnm: p_attrd contains malformed attr rn");
      continue;
    }
    if (!p_rn->p_cp) {
       ErrExit(ErrExit_ASSERT, "getconfig_from_myattrnm: p_attrd attr with missing p_cp");
       continue;
   }
    if (!(IS_VALID_HASH(p_rn->p_cp->h))) {
      ErrExit(ErrExit_ASSERT, "getconfig_from_myattrnm: p_attrd attr hash invalid");
      continue;
    }
    if (p_rn->p_cp->h == attr_hash) {
      p_cp_match = p_rn->p_cp;
    }
  }
  return p_cp_match; 
}

