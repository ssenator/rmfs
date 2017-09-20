#include "rmfs.h"

/*
 * rn_ctljobid_sign()
 *  - regenerate signature attribute of the jobid node
 */

static config_param_t
jobid_sigdesc_tab[] = {
  { .nm="fsid",        .per_src.rmfs = { .local=TRUE                      }, },
  { .nm="version",     .per_src.rmfs = { .local=TRUE                      }, },
  { .nm="ClusterName", .per_src.rmfs = { .local=TRUE                      }, },
  { .nm="job_id",      .per_src.rmfs = { .fs=TRUE, .parent_type=RND_JOBID }, },
  { .nm="user_id",     .per_src.rmfs = { .fs=TRUE, .parent_type=RND_JOBID }, },
  { .nm="submit_time", .per_src.rmfs = { .fs=TRUE, .parent_type=RND_JOBID }, },
  { .nm=NULL }
};

config_param_t *
rn_getsig(rnode_t *p_rn, rnode_t *p_attrd) {
  config_param_t sig_cp, *p_sig_cp, *p_cp, *p_cp_match;
  int new_hash;

  if (!p_rn) {
    ErrExit(ErrExit_ASSERT, "rn_getsig: !p_rn");
    return FALSE;
  }
  if (!p_attrd) {
    if (!p_rn->attr) {
      ErrExit(ErrExit_ASSERT, "rn_getsig: !p_attrd && !p_rn->attr");
      return FALSE;
    }
    p_attrd = p_rn->attr;
  }

  for (p_cp = jobid_sigdesc_tab, new_hash = 0; p_cp->nm; p_cp++) {

    p_cp_match = NULL;
    if (p_cp->per_src.rmfs.local) {
      p_cp_match = getconfig_fromnm(p_cp->nm);
      
    } else if (p_cp->per_src.rmfs.fs) {
      if (!IS_RTYPE_VALID(p2r_typ_convtab[p_cp->per_src.rmfs.parent_type].rtyp)) {
	ErrExit(ErrExit_ASSERT, "rn_getsig: !invalid sig field rnode type");
	return FALSE;
      }
      if (p2r_typ_convtab[p_cp->per_src.rmfs.parent_type].rtyp != RND_JOBID) {
	ErrExit(ErrExit_ASSERT, "rn_getsig: !signature rnode field source");
	return FALSE;
      }
      /*XXX p_cp_match = getconfig_from_nm(p_attrd, p_cp->nm); */
      return FALSE;
    }
    new_hash = djb_accumulate(new_hash, p_cp_match->val.ue.i);
  }
  sig_cp.val.ue.i = new_hash;
  init_hash_cp(&sig_cp);

  p_sig_cp = dup_cp(&sig_cp);
  if (!p_sig_cp) {
    ErrExit(ErrExit_ASSERT, "rn_getsig: !dup_cp(sig_cp)");
    return FALSE;
  }
  return p_sig_cp;
}

  
tri_t
rn_ctljobid_sign(rnode_t *p_sig, config_param_t *p_sig_cp) {
  config_param_t *p_cp, *p_cp_match, *p_sigxattr_cp, *p_sigtyp_cp, *p_2sigxattr_cp, *p_2sigtyp_cp;
  rnode_t        *p_attrd, *p_jobid;
/*  int             new_hash; XXX*/
  rn_param_t     *p_rn_paramtab;
  char            pbuf[_POSIX_PATH_MAX];
  
  extern config_param_t *getconfig_from_myattrnm(rnode_t *, char *); /*rn_mkattr.c*/

  p_cp = p_cp_match = p_sigtyp_cp = p_2sigxattr_cp = p_2sigtyp_cp = p_sigxattr_cp = NULL;

  if (!p_sig) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !p_sig");
    return FALSE;
  }
  if (!p_sig->parent) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !p_sig->parent");
    return FALSE;
  }
  if (!IS_RTYPE_VALID(p_sig->parent->rtype)) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !IS_RTYPE_VALID(p_sig->parent)");
    return FALSE;
  }
  p_attrd = p_sig->parent;
  if (!p_attrd->parent) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !p_attrd->parent");
    return FALSE;
  }
  if (!IS_RTYPE_VALID(p_attrd->rtype)) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !IS_RTYPE_VALID(p_attrd)");
    return FALSE;
  }
  p_jobid = p_attrd->parent;
  if (!IS_RTYPE_VALID(p_jobid->rtype)) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !IS_RTYPE_VALID(p_jobid)");
    return FALSE;
  }
  if (RND_JOBID != p_jobid->rtype) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: rtype !JOBID");
    return FALSE;
  }
  if (!p_sig_cp) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !p_sig_cp");
    return FALSE;
  }

  p_rn_paramtab = get_rn_params(/*needlock*/ FALSE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !get_rn_params");
    return FALSE;
  }

  p_cp = rn_getsig(p_jobid, p_attrd);
  if (!p_sig_cp) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !rn_getsig");
    return FALSE;
  }
  *p_sig_cp = *p_cp;

  /* add an extended attribute with the signature type */
  p_sigtyp_cp = dup_cp(p_sig_cp);
  if (!p_sigtyp_cp) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: dup_cp(\"p_cp\")");
    return FALSE;
  }

  p_sigtyp_cp->nm = strdup(XATTR_SIGTYP);
  if (!p_sigtyp_cp->nm) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: xattr sigtyp nm)");
    return FALSE;
  }

  if (memset(pbuf, 0, _POSIX_PATH_MAX) != pbuf) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !memset(\"xattr\" sigtype)");
    return FALSE;
  }
  if (memcpy(pbuf, SIGTYP_DJBHASH, internal_strlen(SIGTYP_DJBHASH)+1) != pbuf) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !memcpy sigtyp DJBHASH");
    return FALSE;
  }
  
  p_sigtyp_cp->typ = PTYP_XATTR;
  set_val_charptr(p_sigtyp_cp, strdup(pbuf));
  init_hash_cp(p_sigtyp_cp);

  /* and the extended attribute containing the signature */
  p_sigxattr_cp = dup_cp(p_sig_cp);
  p_sigxattr_cp->nm = strdup(XATTR_SIG);
  if (!p_sigxattr_cp->nm) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: strdup(\"XATTR_SIG\")");
    return FALSE;
  }

  if (memset(pbuf, 0, _POSIX_PATH_MAX) != pbuf) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !memset(\"xattr\" sig)");
    return FALSE;
  }
  if (snprintf(pbuf, _POSIX_PATH_MAX-1, "%0x", p_sig_cp->val.ue.i) < 0) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: !snprintf sig");
    return FALSE;
  }
  
  p_sigxattr_cp->typ = PTYP_XATTR;
  set_val_charptr(p_sigxattr_cp, strdup(pbuf));
  init_hash_cp(p_sigxattr_cp);
  
  /* duplicates so that the p_cp->p_nxt ptrs don't conflict with the ones in p_jobid */
  p_2sigxattr_cp = dup_cp(p_sigxattr_cp);
  if (!p_2sigxattr_cp) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: dup_cp(\"p_sigxattr_cp\")");
    return FALSE;

  }
  p_2sigtyp_cp = dup_cp(p_sigtyp_cp);
  if (!p_2sigtyp_cp) {
    ErrExit(ErrExit_ASSERT, "ctljobid_sign: dup_cp(\"p_sigtyp_cp\")");
    return FALSE;
  }
  init_hash_cp(p_2sigxattr_cp);
  init_hash_cp(p_2sigtyp_cp);

  /* link these to both the RND_JOBID and the RNF_SIGNATURE attribute nodes */
  link_xattr2rn(p_jobid, p_sigxattr_cp);
  link_xattr2rn(p_jobid, p_sigtyp_cp);

  link_xattr2rn(p_sig, p_2sigxattr_cp);
  link_xattr2rn(p_sig, p_2sigtyp_cp);

  return TRUE;
}

/*
 * rn_ctljobid_read() [=>from BackingStore]
 *  - synchronize in-core jobid node *from* on-disk jobid node
 *    provided that they describe the same job (signatures match & node contents are sensible)
 *
 * call to ...handleBackingStore___*()
 * may imply a call to validate_BackingStore()
 *
 */
tri_t
rn_ctljobid_read(rnode_t *p_rn, config_param_t *p_cp) {
  return FALSE;
}

/*
 * rn_ctljobid_write()
 *  - write jobid node contents to BackingStore, provided it has been modified (is dirty)
 *    may imply a call to rn_ctljobid_sign()
 */
tri_t
rn_ctljobid_write(rnode_t *p_rn, config_param_t *p_cp) {
  return FALSE;
}


/*
 * rn_jobid_loosematch(jobid)
 *  return rnode * { jobid }
 *   is in the current known list of jobids
 *  else
 *   FALSE
 */

rnode_t *
rn_jobid_matchloose(uint32_t job_id)  {
  rnode_t    *p_rn, *p_jobd, *p_match;
  int         i;
  rn_param_t *p_rn_paramtab;

  /*
   * walk current list of jobids (children of jobd)
   * if one of the children has a jobid that matches the one passed in
   * look for its signature attribute and see if it matches the signature passed in
   * if so, return TRUE
   * else keep looking
   * FALLTHROUGH: return FALSE
   */ 
  p_rn_paramtab = get_rn_params(/*needlock*/ FALSE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr: !get_rn_params");
    return FALSE;
  }
  p_jobd = p_rn_paramtab->p_jobd;

  for (p_rn = p_jobd->children, i = 0, p_match = NULL;
           !p_match &&
           i < p_jobd->n_children && p_rn && IS_RTYPE_VALID(p_rn->rtype) &&
	   p_rn->p_cp && IS_VALID_TYPE(p_rn->p_cp->typ);
               i++, p_rn++) {

    if (p_rn->p_cp->val.ue.ui_32 == job_id) {
      p_match = p_rn;
    }
  }
  return p_match;
}

rnode_t *
rn_jobid_loosematch(uint32_t job_id) {
	return FALSE;
}

tri_t
rn_jobid_matchsig(uint32_t job_id, int signature) {
  rnode_t *p_rn;
#if defined(PORTING_TO_SLURMv17)
  rnode_t *p_attr;
  int      s;
#endif /*PORTING_TO_SLURMv17*/  
  
  if (!(p_rn = rn_jobid_loosematch(job_id))) {
    return FALSE;
  }

#if defined(PORTING_TO_SLURMv17)
  s = rn_getsig(p_rn, p_rn->attr);

  /*ASSERT(sigtyp == DJBHASH)*/
  
  if (!IS_VALID_HASH(s)) {
    ErrExit(ErrExit_ASSERT, "rn_jobid_matchsig: !rn_getsig");
    return FALSE;
  }
  if (s != signature) {
    return FALSE;
  }
  return TRUE;
#endif /* PORTING_TO_SLURMv17 */

  return FALSE;
}

/*
 * rn_jobid_addxattr(uint32_t job_id, char *xattr_nm, char *xattr_val)
 *  store xattr into the job_id rnode associated with job_id
 * TRUE on success, FALSE otherwise
 */

tri_t
rn_jobid_addxattr(uint32_t job_id, char *xattr_nm, char *xattr_val) {
  rnode_t        *p_rn, *p_jobd, *p_match;
  rmfs_param_t    xattr = { { NULL } };
  rn_param_t     *p_rn_paramtab;
#if defined(PORTING_TO_SLURMv17)
  rnode_t        *p_cp_match, *p_cp;
#endif

#if defined(PORTING_TO_SLURMv17)
  int             i;
#endif /*PORTING_TO_SLURMv17*/

  if (job_id == 0) {
    ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr(): implausible job_id");
    return FALSE;
  }
  if (!xattr_nm) {
    ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr(): !xattr_nm");
    return FALSE;
  }
  if (!xattr_val) {
    ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr(): !xattr_val");
    return FALSE;
  }

  xattr.ue.ptr = xattr_val;
  if (!typ_check(PTYP_XATTR, &xattr)) {
    ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr(): !typ_check(xattr_val)");
    return FALSE;
  }

  p_rn_paramtab = get_rn_params(/*needlock*/ FALSE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr: !get_rn_params");
    return FALSE;
  }
  p_jobd = p_rn_paramtab->p_jobd;

  if (!(p_match = rn_jobid_matchloose(job_id))) {
    ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr(): !rn_jobid_matchloose()");
    return FALSE;
  }
#if defined(PORTING_TO_SLURMv17)
  for (p_cp = p_cp_match = NULL; !p_cp_match && p_cp; p_cp = p_cp->p_nxt) {
    if (strcmp(p_cp->nm, xattr_nm) == 0) {
      p_cp_match = p_cp;
    }
  }

  if (!p_cp_match) {
    p_cp_new = calloc(1, sizeof(config_param_t));
    if (!p_cp_new) {
      ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr: calloc(!p_cp_new)");
      return FALSE;
    }
    p_cp_new->nm = strdup(xattr_nm);
    if (!p_cp_new->nm) {
      ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr: calloc(!p_cp_new->nm)");
      return FALSE;
    }
    p_cp_new->val.ue.str = strdup(xattr_val);
    if (!p_cp_new->val.ue.str) {
      ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr: calloc(!p_cp_new->val.ue.str)");
      return FALSE;
    }
    p_cp_new->size = internal_strlen(xattr_val);

    link_xattr2rn(p_match, p_cp_new);
    
  } else {
    p_cp_match->val.ue.str = strdup(xattr_val);
    if (!p_cp_match->val.ue.str) {
      ErrExit(ErrExit_ASSERT, "rn_jobid_addxattr: calloc(!p_cp_match->val.ue.str)");
      return FALSE;
    }
    p_cp_match->size = internal_strlen(xattr_val);
  }
  return TRUE;
#else
  p_jobd = NULL;
  p_rn = NULL;
  return FALSE;
#endif /*PORTING_TO_SLURMv17*/

}

 
