

#include "rmfs.h"


/*
 * the following descriptor tables are defined here but also used by rnode & rmfs
 * These are used to construct file attributes from the variable name presented
 * by slurm in the relevant struct <x>_info where <x> = partition, job or jobstep.
 * see: rn_mkattrd() and rn_mkattr()
 */

config_param_t partinfodesc_tab[] = {
  {.nm="flags",       .per_src.slurm.off=offsetof(partition_info_t, flags),       .typ=PTYP_OPAQUE,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_PARTITION},
  {.nm="name",        .per_src.slurm.off=offsetof(partition_info_t, name),        .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_PARTITION},
  {.nm="node_list",   .per_src.slurm.off=offsetof(partition_info_t, nodes),       .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_PARTITION},
  {.nm="state_up",    .per_src.slurm.off=offsetof(partition_info_t, state_up),    .typ=PTYP_NODESTATE,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_PARTITION},
  {.nm="total_cpus",  .per_src.slurm.off=offsetof(partition_info_t, total_cpus),  .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_PARTITION},
  {.nm="total_nodes", .per_src.slurm.off=offsetof(partition_info_t, total_nodes), .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_PARTITION},
 {.nm=NULL }
};



config_param_t *
collectslurm_attr_part(config_param_t *p_cp, rmfs_param_t *p_val, partition_info_t *pi) {
  config_param_t *p_2cp;
  
  if (!p_cp) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_part: !p_cp");
    return NULL;
  }
  if (!p_val) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_part: !p_val");
    return NULL;
  }
  if (!pi) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_part: !p_pi");
    return NULL;
  }

  if (!IS_VALID_HASH(p_cp->h)) {
    init_hash_cp(p_cp);
    ErrExit(ErrExit_WARN, "collextslurm_attr_part: invalid hash");
    if (!IS_VALID_HASH(p_cp->h)) {
      ErrExit(ErrExit_INTERNAL, "collextslurm_attr_part: invalid hash");
    }
  }

  if (!p_cp->per_src.slurm.dynamic) {
    return NULL;
  }
  if (p_cp->per_src.rmfs.local) {
    return NULL;
  }

  if (djb_strtohash("flags") == p_cp->h) {
    p_val->ue.ui_16 = pi->flags;

  } else if (djb_strtohash("name") == p_cp->h) {
    p_val->ue.str = pi->name;
    p_val->size   = internal_strlen(pi->name);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("node_list") == p_cp->h) {
    p_val->ue.str = pi->nodes;
    p_val->size   = internal_strlen(pi->nodes);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("state_up") == p_cp->h) {
    p_val->ue.ui_16 = pi->state_up;
      
  } else if (djb_strtohash("total_cpus") == p_cp->h) {
    p_val->ue.ui_32 = pi->total_cpus;

  } else if (djb_strtohash("total_nodes") == p_cp->h) {
    p_val->ue.ui_32 = pi->total_nodes;
      
  } else {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr_part; unknown attribute");
    return NULL;
  }
  p_2cp = dup_cp(p_cp);
  p_2cp->val = *p_val;

  return p_2cp;
}


param_source_t
collectslurm_partitions(config_param_t *p_cp) {
  time_t t = 0;

  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "collectslurm_partitions: !derefable(p_cp)");
    return PSRC_NONE;
  }
  if (!p_cp->per_src.slurm.pim) {
    if (slurm_load_partitions(t, &p_cp->per_src.slurm.pim, SHOW_ALL|SHOW_DETAIL) != 0) {
      ErrExit(ErrExit_ASSERT, "collectslurm_partitions: no partitions?");
      return PSRC_NONE;
    }
  }
  t = p_cp->per_src.slurm.pim->last_update;
  set_rnparam_time(offsetof(rn_param_t, last_update), t);
  return PSRC_SLURM;
}
