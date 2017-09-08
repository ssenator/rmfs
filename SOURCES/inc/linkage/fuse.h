/*
 *  rmfs <--> fuse rmfs linkage
 */


#include "rmfs/types.h"
#include "rmfs/conf.h"

#ifndef RMFS_FUSE_LINKAGE_H_
#define RMFS_FUSE_LINKAGE_H_

#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 26
#endif

#include <fuse/fuse.h>
#include <fuse/fuse_opt.h>

/*
 * These are used to
 *  - construct the struct fuse_opt (fuse/fuse_opt.h) linkage table
 *  - construct the usage message
 *  - enforce the input types of the parameters
 */

struct slurm_fuse_opt_desc {
  char                *_s;
  char                *_long;     /* _ because "long" is a keyword */
  
  char                *nm;        /* as in the slurmfs_config table */
  struct config_param *p_cp;      /* ptr to slurmfs_config entry */

  char                *help;      /* for constructing the usage msg */
  opt_flg_t            oflg;      /* option flags, rmfs/types.h */
};
typedef struct slurm_fuse_opt_desc slurm_fuse_opt_desc_t;

extern struct fuse_opt      *p_fuseopts_tbl; /*conf/conf_fuseopt.c*/
extern int                   fuseopts_len;   /*conf/conf_fuseopt.c*/

extern slurm_fuse_opt_desc_t slurm_fopts[];  /*rmfs_conf.c*/
extern struct fuse_operations rmfs_file_ops; /*rmfs_fuseops.c*/

#define N_SLURM_FOPTS (sizeof(slurm_fopts)/sizeof(struct slurm_fuse_opt_desc))
#define N_SLURMFS_CONFIG (sizeof(slurmfs_config)/sizeof(struct config_param))


/*
 * fuse arg linkage
 *  initialized in main(), consumed by fuse_opt_parse() [collect_fuse_mountopts()]
 */
struct fuse_args f_args;

#endif
