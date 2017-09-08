
#include "rmfs.h"

ptyp_t slurm2basetyp[] = {
  PTYP_ALPHANUM,     /* PTYP_CLUSTER, PTYP_SLURM_FIRST */
  PTYP_HOSTNAME,     /* PTYP_CNTRLMACH                 */
  PTYP_ALPHANUM,     /* PTYP_PARTITION                 */
  PTYP_HOSTNAME,     /* PTYP_NODE                      */
  PTYP_NUMERICHAR,   /* PTYP_JOB                       */
  PTYP_NUMERICHAR,   /* PTYP_STEP                      */
  PTYP_FILE,         /* PTYP_ALLOCJOB                  */
  PTYP_OPAQUE,       /* NODESTATE                      */
  PTYP_NUMERIC,      /* SLURMVERSION                   */
  PTYP_UID,          /* SLURMUID                       */
  PTYP_NUMTIM_SECS,  /* SLURMTMOUT                     */
  PTYP_ALPHANUM,     /* SPANKENV                       */
  PTYP_NUMERIC,      /* SPANKENVSIZE, PTYP_SLURM_LAST  */
  PTYP_NONE
};
  

