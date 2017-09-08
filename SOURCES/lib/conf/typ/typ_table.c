
#include "rmfs.h"

/*
 * type check functions
 * note that these have a "is a" relationship from specific to general types
 * ex. typ_numericchar() also calls or implies typ_isalphanum()
 */


extern tri_t typ_always_fails(rmfs_param_t *);
extern tri_t typ_opaque(rmfs_param_t *);
extern tri_t typ_isalphanum(rmfs_param_t *);
extern tri_t typ_isalphanum_ornul(rmfs_param_t *);
extern tri_t typ_isalpha(rmfs_param_t *);
extern tri_t typ_numericchar(rmfs_param_t *);
extern tri_t typ_xattr(rmfs_param_t *);
extern tri_t typ_path(rmfs_param_t *);
extern tri_t typ_file(rmfs_param_t *);
extern tri_t typ_filexist(rmfs_param_t *);
extern tri_t typ_direxist(rmfs_param_t *);
extern tri_t typ_dir(rmfs_param_t *);
extern tri_t typ_host(rmfs_param_t *);
extern tri_t typ_numeric(rmfs_param_t *);
extern tri_t typ_numerictime(rmfs_param_t *);
extern tri_t typ_numtimsecs(rmfs_param_t *);
extern tri_t typ_numsigned(rmfs_param_t *);
extern tri_t typ_unsigned_int(rmfs_param_t *);
extern tri_t typ_unsigned_int16(rmfs_param_t *);
extern tri_t typ_unsigned_int32(rmfs_param_t *);
extern tri_t typ_trilene(rmfs_param_t *);
extern tri_t typ_boolean(rmfs_param_t *);
extern tri_t typ_context(rmfs_param_t *);

extern tri_t typ_slurm_cluster(rmfs_param_t *);
extern tri_t typ_slurm_controlmach(rmfs_param_t *);
extern tri_t typ_slurm_partition(rmfs_param_t *);
extern tri_t typ_slurm_node(rmfs_param_t *);
extern tri_t typ_slurm_job(rmfs_param_t *);
extern tri_t typ_slurm_step(rmfs_param_t *);
extern tri_t typ_slurm_allocjob(rmfs_param_t *);
extern tri_t typ_slurm_nodestate(rmfs_param_t *);

/*
 * typecheck dispatcher & function switch table
 *
 * this table must be kept in alignment with the values of ptyp_t (rmfs_types.h)
 */
tri_t (*param_typecheck_functab[])(rmfs_param_t *) = {
  typ_always_fails,                     /* NONE */
  typ_opaque,                           /* OPAQUE = FIRST */
  typ_isalphanum,                       /* ALPHANUM = ALPHA_FIRST */
  typ_isalpha,                          /* ALPHA */
  typ_numericchar,                      /* NUMERICCHAR */
  typ_always_fails, /*XXXtyp_p2palpha*/ /* ALPHA_P2P */
  typ_xattr,                            /* XATTR */
  typ_path,                             /* PATH */
  typ_file,                             /* FILE */
  typ_filexist,                         /* FILEXIST */
  typ_always_fails, /*XXXtyp_sym,*/     /* SYM */
  typ_direxist,                         /* DIREXIST */ 
  typ_dir,                              /* DIR */
  typ_host,                             /* HOST  = ALPHA_LAST */
  typ_numeric,                          /* NUMERIC = INT_FIRST */
  typ_numerictime,                      /* NUMERICTIME */
  typ_numtimsecs,                       /* NUMTIMSECS */
  typ_numsigned,                        /* NUMSIGNED */
  typ_unsigned_int,                     /* UNSIGNED_INT */
  typ_unsigned_int16,                   /* UNSIGNED_INT16 */
  typ_unsigned_int32,                   /* UNSIGNED_INT32 */
  typ_opaque,                           /* UID */
  typ_opaque,                           /* PID */
  typ_trilene,                          /* TRILENE */
  typ_boolean,                          /* BOOLEAN = INT_LAST */
  typ_always_fails, /*XXXtyp_sig,*/     /* SIGNATURE */
  typ_context,                          /* CONTEXT = CNTXT_FIRST = CNTXT_LAST */
  typ_slurm_cluster,                    /* CLUSTER = SLURM_FIRST */  
  typ_slurm_controlmach,                /* CNTRLMACH */
  typ_slurm_partition,                  /* PARTITION */
  typ_slurm_node,                       /* NODE */
  typ_slurm_job,                        /* JOB */ 
  typ_slurm_step,                       /* STEP */
  typ_slurm_allocjob,                   /* ALLOCJOB */
  typ_slurm_nodestate,                  /* NODESTATE */
  typ_numeric,                          /* SLURM_VERSION */
  typ_numeric,                          /* UID */
  typ_numeric,                          /* TMOUT */
  typ_always_fails, /*XXXtyp_spankenv*/ /* SPANKENV */
  typ_numeric,                          /* SPANKENVSIZE = SLURM_LAST*/
  typ_isalphanum_ornul,                 /* JSUB_PLUGIN */
  typ_isalphanum_ornul,                 /* GRES_PLUGIN */
  typ_isalphanum,                       /* slurmctl SLURMUNAME */
  typ_isalphanum,                       /* slurm SLURMDUNAME */
  NULL,                                 /* terminator/guard */
};

