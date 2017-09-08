
#include "rmfs.h"

param_source_t
calcul_derived(config_param_t *p_cp) {

  if (p_cp && p_cp->collector) {
    return (*p_cp->collector)(p_cp);
  }
  return PSRC_NONE;
}


param_source_t
calcul_isController(config_param_t *p_cp) {
  unsigned long   h_hostnm, h_controlmachine, h_iscontroller;
  config_param_t *p_cp_control;
  
  h_iscontroller = djb_strtohash("isController");
  if (p_cp->h != h_iscontroller) {
    ErrExit(ErrExit_ASSERT, "calcul_isController: !djb_strtohash(isController)");
    return PSRC_NONE;
  }
    
  p_cp_control = getconfig_fromnm("ControlMachine");
  h_hostnm         = p_cp->h;
  h_controlmachine = p_cp_control->h;
  /* our hostname is the ControlMachine's hostname? */
  
  if (h_hostnm != 0 && h_hostnm != ~0 &&
      h_controlmachine != 0 && h_controlmachine != ~0
     ) {
    
    if (h_hostnm == h_controlmachine) {
      p_cp->val.ue.truth = 1;
      return PSRC_DERIVED;
    }
  }
  return PSRC_NONE;
}

param_source_t
calcul_rnodepool(config_param_t *p_cp) {
  param_source_t    src;
  unsigned long     n;
  partition_info_t *p_pi;       /*ptr to partition_info*/ 
  int               i;
  rn_param_t       *p_rn_params;
  config_param_t   *p_partitions_cp;

  extern param_source_t collectslurm_partitions(config_param_t *); /*collect_slurm_part.c*/

  p_rn_params = get_rn_params(/*needlock*/ TRUE);
  if (!p_rn_params) {
   ErrExit(ErrExit_ASSERT, "calcul_rnodepool: !p_rn_params");
   return PSRC_NONE;
  }
  p_rn_params->rn_minpoolsize = -1;

  p_partitions_cp = getconfig_fromnm("partitions");
  if (!p_partitions_cp) {
      ErrExit(ErrExit_ASSERT, "calcul_rnodepool: !p_partitions_cp");
      return PSRC_NONE;
  }
  if (!ISSET(p_partitions_cp->src.actual, PSRC_SLURM)) {
    src = collectslurm_partitions(p_partitions_cp);
    if (src != PSRC_SLURM) {
      ErrExit(ErrExit_ASSERT, "calcul_rnodepool: collect(p_partitions_cp) src !SLURM");
      return PSRC_NONE;
    }
  }
  if (!p_partitions_cp->per_src.slurm.pim) {
      ErrExit(ErrExit_ASSERT, "calcul_rnodepool: !slurm.pim");
      return PSRC_NONE;
  }
  if (!p_partitions_cp->per_src.slurm.pim->partition_array) {
    ErrExit(ErrExit_ASSERT, "calcul_rnodepool: !slurm.pim.partition_array");
    return PSRC_NONE;
  }
  if (p_partitions_cp->per_src.slurm.pim->record_count <= 0) {
    ErrExit(ErrExit_ASSERT, "calcul_rnodepool: empty partition_array");
    return PSRC_NONE;
  }
  p_cp->per_src.slurm.pim = p_partitions_cp->per_src.slurm.pim; 
  
  for (p_pi = p_cp->per_src.slurm.pim->partition_array, i = 0, n = 0;
           i < p_cp->per_src.slurm.pim->record_count;
               i++, p_pi++) {
    
    if (p_pi > 0) {
      n += p_pi->total_nodes;
    }
    /* heurestic, assuming nodes used exclusively so 1 job / node plus overhead */
    n += n <= 2? 4: ((n & (n-1))<<1); /*XXXbitnerd roundup to higher nearest power of 2*/
  }

  if (n <= 0) {
    ErrExit(ErrExit_ASSERT, "calcul_rnodepool: slurm_partitions total nodes <= 0");
    return PSRC_NONE;
  }

  p_cp->val.ue.ul = n * sizeof(rnode_t);
  p_rn_params->rn_minpoolsize = n;
  rn_paramtab_release();
  
  return PSRC_DERIVED;
}


param_source_t
calcul_unmountwait(config_param_t *p_cp) {
  config_param_t *slctl_timeout_p_cp = NULL;
  unsigned long   h;
  long            sl_tmout;
  
  h = djb_strtohash("unmountWait");
  if (h != p_cp->h) {
    ErrExit(ErrExit_ASSERT, "calcul_unmountwait: h != p_cp->h");
    return PSRC_NONE;
  }

  slctl_timeout_p_cp = getconfig_fromnm("SlurmCtldTimeout");
  
  sl_tmout = slctl_timeout_p_cp->val.ue.time;
  /* give slurm ctl daemon reasonable time to check or change state */
  set_val_l(p_cp, (sl_tmout * 2) + 1);
  return PSRC_DERIVED;
}
