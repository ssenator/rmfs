#include "rmfs.h"


/*
 * cast a provisioned node into an RN_type
 *  and call the builder function for that type
 */
rnode_t *
rn_cast(rnode_t *p_new,	rn_type_t rtype,
	config_param_t *p_cp, void *p_dyntyp, char *nm,
	rnode_t *p_parent, rnode_t *p_children, int n_children,
	rnode_t *p_attr, rnode_t *p_subdir) {
  
  rnode_t        *p_built, *p_buildtab_rn, *p_child;
  config_param_t *p_2cp;
  int             n_flags, h;
  tri_t           someAssemblyRequired, not_fsroot;
  
  extern config_param_t *mkattr_xattr_ctx(rnode_t *, char *);

  someAssemblyRequired = p_new->gen == 0;   /* called on 1st cast, not subsequent */
  not_fsroot           = rtype != RND_ROOT; /* except for fsroot */
  
  p_buildtab_rn = &rnode_buildtab[rtype];

  if (!p_buildtab_rn->buildfn) {
    ErrExit(ErrExit_ASSERT, "rn_cast: !buildfn");
    return NULL;
  }
  p_new->gen++;

  if (p_new->rtype != rtype) {
    if (someAssemblyRequired) {
      if (RN_GUARD == p_new->rtype) {
	p_new->rtype = rtype;
      } else {
	ErrExit(ErrExit_ASSERT, "rn_cast: p_new->rtype != rtype && p_new->rtyp != RN_GUARD)");
      }
    } else if (IS_RTYPE_ATTRIBUTE(rtype) && IS_RTYPE_ATTR_SUBTYPE(p_new->rtype)) {
      p_new->rtype = rtype;
      
    } else {
      ErrExit(ErrExit_ASSERT, "rn_cast: p_new->rtype overwrite");
      return NULL;
    }
  }

  if (p_cp) {
    p_new->p_cp = dup_cp(p_cp);
    
  } else if (p_cp == CONFPARAM_MISSINGOK) {
    p_new->p_cp = CONFPARAM_MISSINGOK;
    
  } else if (p_cp == CONFPARAM_REQUIRED) {

    /*
     * Attempt to use config_param based on the specific name,
     * if none found, use the type name
     */

    if (derefable_cp(p_cp)) {
      p_2cp = p_cp;
      
    } else if (nm) {
      p_2cp = getconfig_fromnm(nm);
      if (!p_2cp) {
	p_2cp = getconfig_fromnm(p_buildtab_rn->nm);
      }
    }
    if (!p_2cp) {
      ErrExit(ErrExit_ASSERT, "rn_cast: p_2cp");
      return NULL;
    }
    p_new->p_cp = dup_cp(p_2cp);
    if (!p_new->p_cp) {
      ErrExit(ErrExit_ASSERT, "rn_cast(p_new->p_cp)");
      return NULL;
    }
  }
  nm = nm? nm: p_buildtab_rn->nm;
  h  = djb_strtohash(nm);
  
  if (p_parent && p_parent != p_new && someAssemblyRequired) {
    for (p_child = p_parent->children;
	     p_child && p_child->nm && IS_VALID_HASH(p_child->h);
	         p_child++) {
      if (h == p_child->h) {
	ErrExit(ErrExit_ASSERT, "rn_cast: !twin"); /*no duplicate children*/
	return NULL;
      }
    }
  }
  
  p_new->nm         = nm;
  p_new->h          = h;
  p_new->p_dyntyp   = p_dyntyp;

  p_new->parent     = p_parent;
  p_new->children   = p_children;
  p_new->n_children = n_children;
  p_new->attr       = p_attr;
  p_new->subdir     = p_subdir;
  p_new->buildfn    = p_buildtab_rn->buildfn;

  /* but not uid, gid to allow user visibility into their own job */

  p_new->xattr      = NULL;
  p_new->n_xattr    = 0;
  p_new->nxt_dirty  = NULL;
  
  p_new->maybe.dirty        = FALSE;
  p_new->maybe.controllable = p_buildtab_rn->maybe.controllable;
  p_new->maybe.readable     = p_buildtab_rn->maybe.readable;
  p_new->maybe.writable     = p_buildtab_rn->maybe.writable;
  p_new->maybe.execable     = p_buildtab_rn->maybe.execable;
  p_new->maybe.notify       = NULL;
  
  p_new->is.dir  = p_buildtab_rn->is.dir;
  p_new->is.file = p_buildtab_rn->is.file;
  p_new->is.link = p_buildtab_rn->is.link;

  n_flags = 0;
  if (p_new->is.dir) {
    n_flags++;
  }
  if (p_new->is.file) {
    n_flags++;
  }
  if (p_new->is.link) {
    ErrExit(ErrExit_ASSERT, "rn_cast: symlinks not fully implemented yet");
    return NULL;
    n_flags++;
  }
  if (n_flags != 1) {
    if (n_flags == 0) {
      ErrExit(ErrExit_ASSERT, "rn_cast: unknown file types");
    } else {
      ErrExit(ErrExit_ASSERT, "rn_cast: conflicting file types");
    }
    return NULL;
  }

  if (IS_RTYPE_CONTROL(p_buildtab_rn->ctl[0].rtype)) {
    p_new->maybe.controllable = TRUE;
    
    if (memcpy (p_new->ctl, p_buildtab_rn->ctl, (sizeof(rn_ctl_t)*MAX_RN_CMDS)) != p_new->ctl) {
      ErrExit(ErrExit_ASSERT, "rn_cast: memcpy(ctl) failed");
      return NULL;
    }
    
  } else {
    if (memset(p_new->ctl, 0, (sizeof(rn_ctl_t)*MAX_RN_CMDS)) != p_new->ctl) {
      ErrExit(ErrExit_ASSERT, "rn_cast: memcpy(ctl) failed");
      return NULL;
    }
  }

  p_built = p_new;
  
  if (someAssemblyRequired) {
    if (not_fsroot) {
      p_built = (*p_new->buildfn)(p_parent, p_new);
    } /* else { root has built, does (will?) build itself } */

  } else {
    if (!mkattr_xattr_ctx(p_new, rtype != RND_JOBID? CTX_DEFAULT: CTX_JOBID_DEFAULT)) {
      ErrExit(ErrExit_ASSERT, "rn_cast: !mkattr_xattr_ctx(CTX_DEFAULT)");
      return NULL;
    }
  }
  return p_built;
}

/*
 * rn_cast_attr_typconv() is a wrapper around rn_cast which converts from
 * config param types to rnode types
 */
rnode_t *
rn_cast_attr_typconv(rnode_t *p_new, ptyp_t ptyp,
	config_param_t *p_cp, void *p_dyntyp, char *nm,
	rnode_t *p_parent, rnode_t *p_children, int n_children,
	rnode_t *p_attr, rnode_t *p_subdir) {
  
  rn_type_t rtyp;
  int       i;
  rnode_t  *p_rn;

  /* ensure the conversion tables are laid out correctly */
  if (IS_SLURM_TYPE(ptyp)) {
    i = ptyp - PTYP_SLURM_FIRST;
    if (p2r_typ_rm_convtab[i].ptyp != (i + PTYP_SLURM_FIRST)) {
      ErrExit(ErrExit_INTERNAL,
	"rn_cast_attr_typconv: (p2r_typ_rm_convtab[ptyp].ptyp == (ptyp - PTYP_SLURM_FIRST)");
      return NULL;
    }
    /*XXX slurm_sanity_check_functions()*/
    
  } else {
    if (p2r_typ_convtab[ptyp].ptyp != ptyp) {
      ErrExit(ErrExit_ASSERT, "rn_cast_attr_typconv: p2r_typ_convtab[ptyp].ptyp != ptyp");
      return NULL;
    }
  }

  /*
   * save the config param type so that the rnode constructor and
   * the read file-op can interpret the datum contents correctly
   */
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "WARNING: rn_cast_attr_typconv: !derefable_cp(p_cp)");
  }
  p_cp->typ = ptyp;

  /*
   * if this is a resource manager type, use its converstion table, or
   * otherwise, if there is a known type mapping, use it.
   * fallback to a generic attribute type
   */

  if (IS_SLURM_TYPE(ptyp)) {
    rtyp = p2r_typ_rm_convtab[(ptyp-PTYP_SLURM_FIRST)].rtyp;

  } else if (p2r_typ_convtab[ptyp].rtyp != RN_NONE) {
    rtyp = p2r_typ_convtab[ptyp].rtyp;

  } else {
    rtyp = RNF_ATTRIBUTE;
  }

  p_rn = rn_cast(p_new, rtyp, p_cp, p_dyntyp, nm,
		 p_parent, p_children, n_children,
		 p_attr, p_subdir);
  return p_rn;
}
