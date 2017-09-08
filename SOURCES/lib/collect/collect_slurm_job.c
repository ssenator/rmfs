

#include "rmfs.h"


/*
 * the following descriptor tables are defined here but also used by rnode & rmfs
 * These are used to construct file attributes from the variable name presented
 * by slurm in the relevant struct <x>_info where <x> = partition, job or jobstep.
 * see: rn_mkattrd() and rn_mkattr()
 */

config_param_t jobinfodesc_tab[] = {
  {.nm="account",      .per_src.slurm.off=offsetof(job_info_t, account),      .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="comment",      .per_src.slurm.off=offsetof(job_info_t, comment),      .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="end_time",     .per_src.slurm.off=offsetof(job_info_t, end_time),     .typ=PTYP_NUMERICTIME,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="features",      .per_src.slurm.off=offsetof(node_info_t, features),   .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="group_id",     .per_src.slurm.off=offsetof(job_info_t, group_id),     .typ=PTYP_UID,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="gres",         .per_src.slurm.off=offsetof(job_info_t, gres),         .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="job_id",       .per_src.slurm.off=offsetof(job_info_t, job_id),       .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="job_state",    .per_src.slurm.off=offsetof(job_info_t, job_state),    .typ=PTYP_OPAQUE,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="name",         .per_src.slurm.off=offsetof(job_info_t, name),         .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="nodes",        .per_src.slurm.off=offsetof(job_info_t, nodes),        .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="num_cpus",    .per_src.slurm.off=offsetof(job_info_t, num_cpus),      .typ=PTYP_UNSIGNED_INT32,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="num_nodes",    .per_src.slurm.off=offsetof(job_info_t, num_nodes),    .typ=PTYP_UNSIGNED_INT32,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="partition",    .per_src.slurm.off=offsetof(job_info_t, partition),    .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="state_desc",   .per_src.slurm.off=offsetof(job_info_t, state_desc),   .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="state_reason", .per_src.slurm.off=offsetof(job_info_t, state_reason), .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="submit_time",  .per_src.slurm.off=offsetof(job_info_t, submit_time),  .typ=PTYP_NUMERICTIME,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="user_id",      .per_src.slurm.off=offsetof(job_info_t, user_id),      .typ=PTYP_UID,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="context",       .src.allowed=(BIT(PSRC_MAC)|BIT(PSRC_MNT)|BIT(PSRC_DEFAULT)),
                                                                              .typ=PTYP_CONTEXT,
   .per_src.rmfs.local=TRUE, .per_src.slurm.parent_type=PTYP_JOB},
  {.nm="signature",     .src.allowed=BIT(PSRC_DERIVED),                       .typ=PTYP_SIGNATURE,
   .per_src.rmfs.local=TRUE},
  {.nm=NULL }
};

config_param_t *
collectslurm_attr_job(config_param_t *p_cp, rmfs_param_t *p_val, job_info_t *ji) {
  config_param_t *p_2cp;
  
  if (!p_cp) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_job: !p_cp");
    return NULL;
  }
  if (!p_val) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_job: !p_val");
    return NULL;
  }
  if (!ji) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_job: !p_ji");
    return NULL;
  }

  if (!p_cp->per_src.slurm.dynamic) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_node: !slurm.dynamic");
  }
  if (p_cp->per_src.rmfs.local) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_node: rmfs.local");
  }

  if (!IS_VALID_HASH(p_cp->h)) {
    init_hash_cp(p_cp);
    ErrExit(ErrExit_WARN, "collectslurm_attr_job: invalid hash");
    if (!IS_VALID_HASH(p_cp->h)) {
      ErrExit(ErrExit_INTERNAL, "collectslurm_attr_job: invalid hash");
    }
  }
  if (djb_strtohash("account") == p_cp->h) {
    p_val->ue.str = ji->account;
    p_val->size   = internal_strlen(ji->account);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("command") == p_cp->h) {
    p_val->ue.str = ji->command;
    p_val->size   = internal_strlen(ji->command);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("comment") == p_cp->h) {
    p_val->ue.str = ji->comment;
    p_val->size   = internal_strlen(ji->comment);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("end_time") == p_cp->h) {
    p_val->ue.time = ji->end_time;

  } else if (djb_strtohash("features") == p_cp->h) {
    p_val->ue.str = ji->features;
    p_val->size  = internal_strlen(ji->features);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("group_id") == p_cp->h) {
    p_val->ue.gid = ji->group_id;

  } else if (djb_strtohash("gres") == p_cp->h) {
    p_val->ue.str = ji->gres;
    p_val->size  = internal_strlen(ji->gres);
    if (p_val->size > 0) {
      p_val->size += 1;
    }
  } else if (djb_strtohash("job_id") == p_cp->h) {
    p_val->ue.ui_32 = ji->job_id;

  } else if (djb_strtohash("job_state") == p_cp->h) {
    p_val->ue.ui_16 = ji->job_state;

  } else if (djb_strtohash("name") == p_cp->h) {
    p_val->ue.str = ji->name;
    p_val->size   = internal_strlen(ji->name);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("nodes") == p_cp->h) {
    p_val->ue.str = ji->nodes;
    p_val->size   = internal_strlen(ji->nodes);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("num_cpus") == p_cp->h) {
    p_val->ue.ui_32 = ji->num_cpus;
    
  } else if (djb_strtohash("num_nodes") == p_cp->h) {
    p_val->ue.ui_32 = ji->num_nodes;

  } else if (djb_strtohash("partition") == p_cp->h) {
    p_val->ue.str = ji->partition;
    p_val->size   = internal_strlen(ji->partition);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("state_desc") == p_cp->h) {
    p_val->ue.str = ji->state_desc;
    p_val->size   = internal_strlen(ji->state_desc);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("state_reason") == p_cp->h) {
    p_val->ue.i = ji->state_reason;

  } else if (djb_strtohash("submit_time") == p_cp->h) {
    p_val->ue.time = ji->submit_time;

  } else if (djb_strtohash("user_id") == p_cp->h) {
    p_val->ue.uid = (uid_t) ji->user_id; /*uint32_t*/

  } else {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr_job; unknown attribute");
    return NULL;
  }

  p_2cp = dup_cp(p_cp);
  p_2cp->val = *p_val;

  return p_2cp;
}


param_source_t
collectslurm_jobs(config_param_t *p_cp) {
  time_t t = 0;

  if (!derefable_cp(p_cp)) {
    return PSRC_NONE;
  }
  if (!p_cp->per_src.slurm.jim) {
    if (slurm_load_jobs(t, &p_cp->per_src.slurm.jim, SHOW_ALL|SHOW_DETAIL) < 0) {
      return PSRC_NONE;
    }
  }
  t = p_cp->per_src.slurm.jim->last_update;
  set_rnparam_time(offsetof(rn_param_t, last_update), t);
  return PSRC_SLURM;
}
