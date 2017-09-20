
#include "rmfs.h"


/*
 * the following descriptor tables are defined here but also used by rnode & rmfs
 * These are used to construct file attributes from the variable name presented
 * by slurm in the relevant struct <x>_info where <x> = partition, job or jobstep.
 * see: rn_mkattrd() and rn_mkattr()
 */

config_param_t nodeinfodesc_tab[] = {
  {.nm="arch",          .per_src.slurm.off=offsetof(node_info_t, arch),          .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
  {.nm="cores",         .per_src.slurm.off=offsetof(node_info_t, cores),         .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
  {.nm="cpus",          .per_src.slurm.off=offsetof(node_info_t, cpus),          .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
#if defined(PORTING_TO_SLURMv17)  
  {.nm="features",      .per_src.slurm.off=offsetof(node_info_t, features),      .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
  {.nm="gres",          .per_src.slurm.off=offsetof(node_info_t, gres),          .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
#endif /*PORTING_TO_SLURMv17*/  
  {.nm="name",          .per_src.slurm.off=offsetof(node_info_t, name),          .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
#if defined(PORTING_TO_SLURMv17)  
  {.nm="node_addr",     .per_src.slurm.off=offsetof(node_info_t, node_addr),     .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
#endif /*PORTING_TO_SLURMv17*/
  {.nm="node_hostname", .per_src.slurm.off=offsetof(node_info_t, node_hostname), .typ=PTYP_HOSTNAME,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
   {.nm="node_state",    .per_src.slurm.off=offsetof(node_info_t, node_state),    .typ=PTYP_NODESTATE,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
  {.nm="os",            .per_src.slurm.off=offsetof(node_info_t, os),            .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
  {.nm="real_memory",   .per_src.slurm.off=offsetof(node_info_t, real_memory),   .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
#if defined(PORTING_TO_SLURMv17)  
  {.nm="reason",        .per_src.slurm.off=offsetof(node_info_t, reason),        .typ=PTYP_ALPHANUM,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
#endif /*PORTING_TO_SLURMv17*/  
  {.nm="sockets",       .per_src.slurm.off=offsetof(node_info_t, sockets),        .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
  {.nm="threads",       .per_src.slurm.off=offsetof(node_info_t, threads),       .typ=PTYP_NUMERIC,
   .per_src.slurm.dynamic=TRUE, .per_src.slurm.parent_type=PTYP_NODE},
  {.nm="allocjob",      .per_src.rmfs.local=TRUE, .src.allowed=BIT(PSRC_DERIVED), .typ=PTYP_ALLOCJOB},
  {.nm=NULL }
};


config_param_t *
collectslurm_attr_node(config_param_t *p_cp, rmfs_param_t *p_val, node_info_t *ni) {
  config_param_t *p_2cp;
  
  if (!p_cp) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_node: !p_cp");
    return NULL;
  }
  if (!p_val) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_node: !p_val");
    return NULL;
  }
  if (!ni) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_node: !p_ni");
    return NULL;
  }

  if (!p_cp->per_src.slurm.dynamic) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_node: !slurm.dynamic");
    return NULL;
  }
  if (p_cp->per_src.rmfs.local) {
    ErrExit(ErrExit_INTERNAL, "collectslurm_attr_node: rmfs.local");
    return NULL;
  }

  if (!IS_VALID_HASH(p_cp->h)) {
    init_hash_cp(p_cp);
    ErrExit(ErrExit_WARN, "collectslurm_attr_node: invalid hash");
    if (!IS_VALID_HASH(p_cp->h)) {
      ErrExit(ErrExit_INTERNAL, "collectslurm_attr_node: invalid hash");
    }
  }

  if (djb_strtohash("arch") == p_cp->h) {
    p_val->ue.str = ni->arch;
    p_val->size   = internal_strlen(ni->arch);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("cores") == p_cp->h) {
    p_val->ue.ui_16 = ni->cores;

  } else if (djb_strtohash("cpus") == p_cp->h) {
    p_val->ue.ui_16 = ni->cpus;

  } else if (djb_strtohash("features") == p_cp->h) {
    p_val->ue.str = ni->features;
    p_val->size   = internal_strlen(ni->features);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("gres") == p_cp->h) {
    p_val->ue.str = ni->gres;
    p_val->size   = internal_strlen(ni->gres);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("name") == p_cp->h) {
    p_val->ue.str = ni->name;
    p_val->size   = internal_strlen(ni->name);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("node_addr") == p_cp->h) {
    p_val->ue.str = ni->node_addr;
    p_val->size   = internal_strlen(ni->node_addr);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("node_hostname") == p_cp->h) {
    p_val->ue.str = ni->node_hostname;
    p_val->size   = internal_strlen(ni->node_hostname);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("node_state") == p_cp->h) {
    p_val->ue.ui_16 = ni->node_state;

  } else if (djb_strtohash("os") == p_cp->h) {
    p_val->ue.str = ni->os;
    p_val->size   = internal_strlen(ni->os);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("real_memory") == p_cp->h) {
    p_val->ue.ui_32 = ni->real_memory;

  } else if (djb_strtohash("reason") == p_cp->h) {
    p_val->ue.str = ni->reason;
    p_val->size   = internal_strlen(ni->reason);
    if (p_val->size > 0) {
      p_val->size += 1;
    }

  } else if (djb_strtohash("sockets") == p_cp->h) {
    p_val->ue.ui_16 = ni->sockets;

  } else if (djb_strtohash("threads") == p_cp->h) {
    p_val->ue.ui_16 = ni->threads;

  } else {
    ErrExit(ErrExit_ASSERT, "collectslurm_attr_node; unknown attribute");
    return NULL;
  }
  p_2cp = dup_cp(p_cp);
  p_2cp->val = *p_val;

  return p_2cp;
}


param_source_t
collectslurm_nodes(config_param_t *p_cp) {
  time_t t = 0;

  if (!derefable_cp(p_cp)) {
    return PSRC_NONE;
  }
  if (!p_cp->per_src.slurm.nim) {
    if (slurm_load_node(t, &p_cp->per_src.slurm.nim, SHOW_ALL|SHOW_DETAIL) != 0) {
      ErrExit(ErrExit_ASSERT, "collectslurm_nodes: no nodes?");
      return PSRC_NONE;
    }
  }
  t = p_cp->per_src.slurm.nim->last_update;
  set_rnparam_time(offsetof(rn_param_t, last_update), t);
  return PSRC_SLURM;
}
