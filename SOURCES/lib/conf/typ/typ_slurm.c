
#include "rmfs.h"

/*
 * XXXFUTURE: following types are implementation-specific,
 * XXXFUTURE: refactor to USERTYPE1, USERTYPE2, etc
 *
 * XXXFUTURE: context semantics for parsing component validity
 * XXXFUTURE: slurm semantics of valid nodes, jobs, etc
 *
 * much of this is at least partially redundant with base slurm functionality
 * the major benefit to catch these errors in typ_*() funcs is to provide
 * more meaningful failure modes & messages in the slu_rmfs context.
 * 
 */

extern tri_t typ_isalphanum(rmfs_param_t *);
extern tri_t typ_numeric(rmfs_param_t *);
extern tri_t typ_host(rmfs_param_t *);

tri_t
typ_slurm_cluster(rmfs_param_t *p_val) {

  /*XXX get "ClusterName" parameter from slurm, compare*/
  return typ_host(p_val);
}

tri_t
typ_slurm_controlmach(rmfs_param_t *p_val) {

  /*XXX get "ControlMachine" parameter from slurm, compare*/
  return typ_host(p_val);
}

tri_t
typ_slurm_partition(rmfs_param_t *p_val) {

  /*XXX get partition list, be sure p_val is listed*/
  return typ_isalphanum(p_val);
}

tri_t
typ_slurm_node(rmfs_param_t *p_val) {

  /*XXX get nodename list, be sure p_val is listed*/
  return typ_host(p_val);
}

tri_t
typ_slurm_job(rmfs_param_t *p_val) {

  /*XXX get job list, be sure jobid is in the list*/
  return typ_numeric(p_val);
}

tri_t
typ_slurm_step(rmfs_param_t *p_val) {

  /*XXX get job step list, be sure step list entry is listed*/
  return typ_numeric(p_val);
}

tri_t
typ_slurm_allocjob(rmfs_param_t *p_val) {

  /*XXXTODO  verify to match the numeric expression of states in slurm.h*/
  return TRUE;
}

tri_t
typ_slurm_nodestate(rmfs_param_t *p_val) {

  /*XXXTODO  verify to match the ascii string states in slurm.h*/
  return TRUE;
}
