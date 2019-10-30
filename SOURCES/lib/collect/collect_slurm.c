/*
 * the collectslurm_*() routines are the consumers of datums provided by the slurm API.
 * It is attempted that the remainder of these algorithms depend upon the configuration
 * parameters collected from various sources, including slurm, but have as little or no
 * knowledge of the specific resource manager, its data structures, implementation, API
 * or, if possible, its design assumptions
 *
 * XXXFUTURE: switchout via an abstraction API for general resource managers
 * XXXFUTURE: such as DRMAA (http://www.drmaa.org)
 *
 * XXXFUTURE: remove rn_param_t dependency
 */

#include "rmfs.h"



param_source_t
collectslurm_api_version(config_param_t *p_cp) {
  long        v = 0;
  extern long slurm_api_version(void); /*slurm/slurm.h*/

  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "collectslurm_api_version: !derefable(p_cp)");
    return PSRC_NONE;
  }
  
  v = slurm_api_version();
  set_val_l(p_cp, v);
  return PSRC_SLURM;
}

/*
 * collect an arbitrary attribute value from a slurm-specific information
 * structure (->p_dyntyp) describing a dynamic resource manager data type
 * such as a node, partition, job or jobstep
 *
 * XXXFUTURE:
 * The outines in collect_slurm_<>.c are a non-optimal interim step to a more general API
 * collector, capable of consuming slurm.h and generating arbitrary pointer references
 * into the relevant <x>_info_t slurm structure. I don't like it any more than you do,
 * probably less.
 *
 */




config_param_t *
collectslurm_attr(rn_type_t rtype, config_param_t *p_cp, void *p_dyntyp)  {
  rmfs_param_t      new_val;
  config_param_t   *p_2cp;
  char              out[_POSIX_PATH_MAX];
  int               l;
  struct tm        *p_tm;
  int               size = _POSIX_PATH_MAX-1;

  extern config_param_t *collectslurm_attr_part(config_param_t *, rmfs_param_t *, partition_info_t *); /*collect_slurm_part.c*/
  extern config_param_t *collectslurm_attr_node(config_param_t *, rmfs_param_t *, node_info_t *);      /*collect_slurm_node.c*/
  extern config_param_t *collectslurm_attr_job(config_param_t *, rmfs_param_t *, slurm_job_info_t *);        /*collect_slurm_job.c*/
  extern config_param_t *collectslurm_attr_step(config_param_t *, rmfs_param_t *, job_step_info_t *);  /*collect_slurm_step.c*/

  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr: !p_cp");
    return NULL;
  }
  if (p_cp->per_src.rmfs.local) {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr: !slurm attr, rmfs.local");
    return NULL;
  }
  if (!p_cp->per_src.slurm.dynamic) {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr: !slurm attr, slurm.dynamic");
    return NULL;
  }
  if (!p_dyntyp) {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr: !p_dyntyp");
    return NULL;
  }
  /*
   * if the parameter is not local, it is sourced from the resource manager.
   * The resource manager provides an API for a table of attribute values.
   * The offset into that table for different types of objects was stored in the
   * associated <x>infodesc_tab, where <x> = {partition, job, jobstep, node}
   * The tables are initialized in slu_rmfs.c.
   *
   *XXX use a per-type resource-mgr specific pointer arithmetic operator?
   *XXX a function seems unnecessarily heavy-weight for just one ptr calculation
   *XXX may require a switch table of -tuples, including parent RTYP/PTYP
   */

  p_2cp = NULL;
  l     = 0;

  if (memset(&new_val, 0, sizeof(new_val)) != (void *) &new_val) {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr: memset");
    return NULL;
  }
  switch(rtype) {
  case RND_PARTNAME:
    p_2cp = collectslurm_attr_part(p_cp, &new_val, p_dyntyp);
    break;
  case RND_NODENAME:
    p_2cp = collectslurm_attr_node(p_cp, &new_val, p_dyntyp);
    break;
  case RND_JOBNAME:
    p_2cp = collectslurm_attr_job(p_cp, &new_val, p_dyntyp);
    break;
  case RND_JOBSTEPID:
    p_2cp = collectslurm_attr_step(p_cp, &new_val, p_dyntyp);
    break;
  default:
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr: switch(what type?)");
    break;
  }
  if (!p_2cp) {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr: per-type collector failed");
    return NULL;
  }

  if (typ_check(p_2cp->typ, &p_2cp->val) == FALSE) {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr: per-typ check failed");
    return NULL;
  }
  /* provide some sugar for people who want to read in ascii not binary */
  if (memset(out, 0, size+1) != out) {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr: memset");
    return NULL;
  }
  if (PTYP_UID == p_2cp->typ) {
    snprintf(out, size, "%d", (int) ((long) p_2cp->val.ue.uid));
    l = internal_strlen(out)+1;		

  } else if (PTYP_PID == p_2cp->typ) {
    snprintf(out, size, "%d", (int) ((long) p_2cp->val.ue.pid));
    l = internal_strlen(out)+1;	

  } else if (PTYP_NUMTIME_SECS == p_2cp->typ) {
    p_tm = localtime(&p_cp->val.ue.time);
    if (!p_tm) {
      snprintf(out, size, "%ld", p_cp->val.ue.time);
      l = internal_strlen(out)+1;
    } else {
      l = strftime(out, size , "%s ", p_tm);
    }

  } else if (PTYP_NUMERICTIME == p_2cp->typ) {
      p_tm = localtime(&p_cp->val.ue.time);
      if (!p_tm) {
	snprintf(out, size, "%u", (unsigned int) p_cp->val.ue.time);
	l = internal_strlen(out)+1;
      } else {
	l = strftime(out, size, "%s ", p_tm);
      }

  } else if (PTYP_NUMSIGNED == p_2cp->typ) {
    snprintf(out, size, "%ld", p_2cp->val.ue.l);
    l = internal_strlen(out)+1;	

  } else if (PTYP_NUMERIC == p_2cp->typ) {
    snprintf(out, size, "%u", (unsigned int) ((unsigned long) p_2cp->val.ue.ul));
    l = internal_strlen(out)+1;	

  } else if (PTYP_UNSIGNED_INT16 == p_2cp->typ) {
    snprintf(out, size, "%u", p_2cp->val.ue.ui_16);
    l = internal_strlen(out)+1;
      
  } else if (PTYP_UNSIGNED_INT32 == p_2cp->typ) {
    snprintf(out, size, "%u", p_2cp->val.ue.ui_32);
    l = internal_strlen(out)+1;
  }

  p_2cp->val.size = l > 0? l: 0;

  return p_2cp;
}

/*
 * collect a configuration parameter from slurm
 */
param_source_t
collectslurm_confparam(config_param_t *p_cp) {
  time_t         t    = 0;
  char          *p    = NULL;
  unsigned long *p_ul = NULL;
  
  extern slurm_ctl_conf_t *p_slcnf;             /*slurm.h*/
  extern time_t            slurm_update_time;
  extern int               slurm_load_ctl_conf(time_t, slurm_ctl_conf_t **);

  if (!derefable_cp(p_cp)) {
    return PSRC_NONE;
  }
  /*XXXFUTURE: walk a type hierarchy, with inheritance, verify slurm_api_version */

  if (!p_slcnf) {
    if (slurm_load_ctl_conf(t, &p_slcnf) < 0) {
      if (p_slcnf) {
	slurm_free_ctl_conf(p_slcnf);
      }
      return PSRC_NONE;
    }
    slurm_update_time = p_slcnf->last_update;
  }

  p = (char *) (p_slcnf + p_cp->per_src.slurm.off); /* XXX checkthis */

  if (IS_SLURM_TYPE(p_cp->typ)) {
    set_val_slurm(p_cp, p_slcnf);
    
  } else if (IS_NUMERIC_TYPE(p_cp->typ)) {
    p_ul = (unsigned long *) p;   /* XXX checkthis */
    set_val_ul(p_cp, *p_ul);

  } else if (IS_ALPHA_TYPE(p_cp->typ)) {  
    set_val_charptr(p_cp, p);
    
  } else {
    set_val_ptr(p_cp, p);
  }
  /* do not free p_slcnf, ptr to (parts of it) are in the p_cp */

  return PSRC_SLURM;
}

void
set_val_slurm(config_param_t *p_cp, void *p_opaque)
{
  rmfs_param_t             new_val;
  ptyp_t                   ptyp;
  extern slurm_ctl_conf_t *p_slcnf;             /*slurm.h*/

  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "set_val_slurm: !p_cp");
    return;
  }
  ptyp = p_cp->typ;

  if (!IS_SLURM_TYPE(p_cp->typ)) {
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: p_cp->typ is not a SLURM type");
    return;
  }
  if (!p_opaque) {
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: no slurm data !p_opaque");
    return;
  }
  
  switch (ptyp) {
  case PTYP_CLUSTER:
    p_slcnf = p_opaque;
    if (!p_slcnf->cluster_name) {
      ErrExit(ErrExit_SLURM, "set_val_slurm: slurm reports no cluster_name");
      return;
    }
    new_val.ue.hostname = p_slcnf->cluster_name;
    new_val.size        = internal_strlen(new_val.ue.hostname)+1;
    
    if (typ_check(ptyp, &new_val) == FALSE) {
      ErrExit(ErrExit_SLURM, "set_val_slurm: typ_check(cluster_name) failed");
      return;
    }
    p_cp->val.ue.hostname = new_val.ue.hostname;
    p_cp->val.size        = new_val.size;
    break;
    
  case PTYP_CNTRLMACH:
    p_slcnf = p_opaque;
    if (!p_slcnf->control_machine) {
      ErrExit(ErrExit_SLURM, "set_val_slurm: slurm reports no control_machine");
      return;
    }
    new_val.ue.hostname = p_slcnf->cluster_name;
    new_val.size        = internal_strlen(new_val.ue.hostname)+1;
    if (typ_check(ptyp, &new_val) == FALSE) {
      ErrExit(ErrExit_SLURM, "set_val_slurm: typ_check(cluster_name) failed");
      return;
    }
    p_cp->val.ue.hostname = new_val.ue.hostname;
    p_cp->val.size        = new_val.size;
    break;
    
  case PTYP_PARTITION:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: PTYP_PARTITION");
    break;
  case PTYP_NODE:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: PTYP_NODE");
    break;
  case PTYP_JOB:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: PTYP_JOB");
    break;
  case PTYP_STEP:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: PTYP_STEP");
    break;
  case PTYP_ALLOCJOB:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: PTYP_ALLOCJOB");
    break;
  case PTYP_NODESTATE:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: PTYP_NODESTATE");
    break;

  case PTYP_SLURMVERSION:
    collectslurm_api_version(p_cp);
    break;

  case PTYP_SLURMUID:
    p_slcnf = p_opaque;
    set_val_uid(p_cp, p_slcnf->slurm_user_id);
    break;

  case PTYP_SLURMTMOUT:
    p_slcnf = p_opaque;
    set_val_ui(p_cp, p_slcnf->slurmd_timeout);
    break;

  case PTYP_SPANKENV:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: PTYP_SPANKENV");
    break;

  case PTYP_SPANKENVSIZE:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: PTYP_SPANKENVSIZE");
    break;

  case PTYP_GRES_PLUGIN:
    p_slcnf          = p_opaque;
    p_cp->val.ue.str = NULL;
    p_cp->val.size   = 0;

    if (p_slcnf->gres_plugins) {
      set_val_charptr(p_cp, p_slcnf->gres_plugins);
    }
    break;

  case PTYP_JSUB_PLUGIN:
    p_slcnf          = p_opaque;
    p_cp->val.ue.str = NULL;
    p_cp->val.size   = 0;

    if (p_slcnf->job_submit_plugins) {
      set_val_charptr(p_cp, p_slcnf->job_submit_plugins);
    }
    break;

  case PTYP_SLURMUNAME:
    p_slcnf          = p_opaque;
    p_cp->val.ue.str = NULL;
    p_cp->val.size   = 0;

    if (p_slcnf->slurm_user_name) {
      set_val_charptr(p_cp, p_slcnf->slurm_user_name);
    }
    break;

  case PTYP_SLURMDUNAME:
    p_slcnf          = p_opaque;
    p_cp->val.ue.str = NULL;
    p_cp->val.size   = 0;

    if (p_slcnf->slurmd_user_name) {
      set_val_charptr(p_cp, p_slcnf->slurmd_user_name);
    }
    break;

  default:
    ErrExit(ErrExit_INTERNAL, "set_val_slurm: p_cp->typ is an unknown SLURM type");
    break;
  }
  return;
}

time_t slurm_update_time  = 0; /* XXX */
slurm_ctl_conf_t *p_slcnf = NULL;

param_source_t
collect_slurm(config_param_t *p_cp) {

  param_source_t    rc = PSRC_NONE;
  
  if (!ISSET(p_cp->src.allowed, PSRC_SLURM)) {
    ErrExit(ErrExit_ASSERT, "collect_slurm: !bit(PSRC_SLURM)");
    goto out;
  }

  if (p_cp->collector) {
    rc = (*p_cp->collector)(p_cp);
  }
   /*
   * XXX else:
   * XXX try for a type-specific collector
   * XXX ...and fallback to a generic object collector?
   */
 out:
  return rc;
}
