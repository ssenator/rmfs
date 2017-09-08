

#include "rmfs.h"


/*
 * the following descriptor tables are defined here but also used by rnode & rmfs
 * These are used to construct file attributes from the variable name presented
 * by slurm in the relevant struct <x>_info where <x> = partition, job or jobstep.
 * see: rn_mkattrd() and rn_mkattr()
 */


config_param_t stepinfodesc_tab[] = {
  {.nm="job_id",       .per_src.slurm.off=offsetof(job_step_info_t, job_id),     .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_STEP },
  {.nm="name",         .per_src.slurm.off=offsetof(job_step_info_t, name),       .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_STEP},
  {.nm="nodes",        .per_src.slurm.off=offsetof(job_step_info_t, nodes),      .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_STEP},
  {.nm="partition",    .per_src.slurm.off=offsetof(job_step_info_t, partition),  .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_STEP},
  {.nm="run_time",     .per_src.slurm.off=offsetof(job_step_info_t, run_time),   .typ=PTYP_NUMTIME,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_STEP},
  {.nm="start_time",   .per_src.slurm.off=offsetof(job_step_info_t, start_time), .typ=PTYP_NUMTIME,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_STEP},
  {.nm="step_id",      .per_src.slurm.off=offsetof(job_step_info_t, step_id),    .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_STEP},
  {.nm="user_id",      .per_src.slurm.off=offsetof(job_step_info_t, user_id),    .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_STEP},
  {.nm=NULL }
};


config_param_t *
collectslurm_attr_step(config_param_t *p_cp, rmfs_param_t *p_val, job_step_info_t *jsti) {
  config_param_t *p_2cp;
  
  if (!p_cp) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_step: !p_cp");
    return NULL;
  }
  if (!p_val) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_step: !p_val");
    return NULL;
  }
  if (!jsti) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_step: !p_jsti");
    return NULL;
  }

  if (!p_cp->per_src.slurm.dynamic) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_step: !slurm.dynamic");
  }
  if (p_cp->per_src.rmfs.local) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_step: rmfs.local");
  }

  if (!IS_VALID_HASH(p_cp->h)) {
    init_hash_cp(p_cp);
    ErrExit(ErrExit_WARN, "collectslurm_attr_step: invalid hash");
    if (!IS_VALID_HASH(p_cp->h)) {
      ErrExit(ErrExit_INTERNAL, "collectslurm_attr_step: invalid hash");
    }
  }

  if (djb_strtohash("job_id") == p_cp->h) {
    p_val->ue.ui_32 = jsti->job_id;

  } else if (djb_strtohash("name") == p_cp->h) {
    p_val->ue.str = jsti->name;
    p_val->size   = internal_strlen(jsti->name);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("nodes") == p_cp->h) {
    p_val->ue.str = jsti->nodes;
    p_val->size   = internal_strlen(jsti->nodes);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("partition") == p_cp->h) {
    p_val->ue.str = jsti->partition;
    p_val->size   = internal_strlen(jsti->partition);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("run_time") == p_cp->h) {
    p_val->ue.time = jsti->run_time;

  } else if (djb_strtohash("start_time") == p_cp->h) {
    p_val->ue.time = jsti->start_time;

  } else if (djb_strtohash("step_id") == p_cp->h) {
    p_val->ue.ui_32 = jsti->step_id;

  } else if (djb_strtohash("user_id") == p_cp->h) {
    p_val->ue.ui_32 = jsti->step_id;

  } else {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr_step; unknown attribute");
    return NULL;
  }
  p_2cp      = dup_cp(p_cp);
  p_2cp->val = *p_val;

  return p_2cp;
}

param_source_t
collectslurm_jobsteps(config_param_t *p_cp) {
  time_t t = 0;
  int    rc;

  if (!derefable_cp(p_cp)) {
    return PSRC_NONE;
  }

  if (!p_cp->per_src.slurm.stim) {
    rc = slurm_get_job_steps(t,
			     NO_VAL, NO_VAL,
			     &p_cp->per_src.slurm.stim,
			     SHOW_ALL|SHOW_DETAIL);
    if (rc < 0) {
      ErrExit(ErrExit_ASSERT, "collectslurm_jobsteps: no steps?");
      return PSRC_NONE; /* capture errno? */
    }
  }
  t = p_cp->per_src.slurm.stim->last_update;
  set_rnparam_time(offsetof(rn_param_t, last_update), t);
  return PSRC_SLURM;
}
