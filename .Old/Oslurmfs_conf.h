/*
 * configuration control of slurmfs
 */

/*
 * these are parameters that control the configuration of slurmfs
 */

/*
 * Configuration parameters:
 *  may be set from various sources
 *
 * These are ordered by precedence. The security model driving this assumes
 * that run-time selection of parameters overrides fixed state from other
 * sources, such configuration files. But, this is tablified so that a future
 * implementation could rearrange this as needed by a different deployment
 * environment.
 *
 * MNT_NON_OPT must be a lower priority than MNT_OPT, to match the fuse
 * optional processing calling protocol which requires a single call with
 * a cumulative table of all possible options.
 *
 * [high precedence]
 *   User input (currently unused) overides a mount option
 *   Mount options (processed by fuse) overide env or config file
 *   The run-time environment overides config file parameters
 *   The MAC Configuration file overides the default slurm config.
 *   Derived values are computed from all other sources
 * [low precedence]
 *
 * Some sources are only honored when in 'debug' mode.
 * Mount options which are processed by fuse are used to construct
 * a fuse_opt table (see fuse/fuse_opt.h, fuse_linkage.h)
 */

#ifndef _SLURMFS_CONF_H_
#define _SLURMFS_CONF_H_

#include <slurm/slurm.h>

/*
 * XXXFUTURE allow order of sources  based on security profile: 'production', 'inputtrusted'
*/
typedef enum param_source {
  PSRC_NONE          = 0,
  PSRC_NOBITS        = PSRC_NONE,

  PSRC_USERINPUT     = 1,
  PSRC_MNT_OPT       = 2,
  PSRC_MNT_NONOPT    = 3, /* corresponds to fuse_opt.h FUSE_OPT_KEY_NONOPT */

  PSRC_ENVAR         = 4,
  PSRC_ENV           = PSRC_ENVAR,

  PSRC_MAC_CONF      = 5,
  PSRC_MAC           = PSRC_MAC_CONF,

  PSRC_SLURM         = 6,
  PSRC_DEFAULT       = 7,
  PSRC_DERIVED       = 8,


  PSRC_ALLBITS    = ( PSRC_DERIVED   |
		      PSRC_DEFAULT   |
		      PSRC_SLURM     |
		      PSRC_MAC_CONF  |
		      PSRC_ENV_VAR   |
		      PSRC_MNT_NONOPT|
		      PSRC_MNT_OPT   |
		      PSRC_USERINPUT
		    )
} param_source_t;

#define BIT(b)       (1<<(b))
#define ISSET(tst,b) (BIT(b) & (tst))

#define ANY(tst)     (ISSET((PSRC_ALLBITS), (tst)))
#define NONE(tst)    (!ISSET((PSRC_ALLBITS), (tst)))

#define PSRC_LSB(tst)    ((tst) & ((tst)-1))
#define PSRC_MSB(tst)    { unsigned int _v = (tst); unsigned int _r = 0; while (_v >>= 1) { _r++; }, r } /* http://graphics.stanford.edu/~seander/bithacks.html#ZerosOnRightLinear */

#define PSRC_NXT(b)      ((b)+1)
#define PSRC_FIRST       (PSRC_LSB(PSRC_ALLBITS))
#define PSRC_LAST        (PSRC_MSB(PSRC_ALLBITS))
#define PSRC_HIPRI       (PSRC_FIRST)
#define PSRC_LOPRI       (PSRC_LAST)
#define PSRC_BIT2FUNC(b) ((b)-1)

typedef union slurmfs_fuse_param {
  void     *ptr;
  char     *str;
  char     *pathnm;
  char     *hostname;
  tri_t     truth;
  time_t    time;
  ulong_t   ue;
  
  struct { /* -> slurm.h */
    partition_info_msg_t         *pim;
    node_info_msg_t              *nim;
    job_info_msg_t               *jim;
    job_step_info_response_msg_t *stim;
  } slurm;
} slurmfs_param_t;

#define slurm_conf_oof(field) (offsetof(slurm_ctl_conf_t, (field))
#define sloof(field) (.per_src.slurmconf_off = slurm_conf_oof(field))

/* configuration parameter value and characteristics of the source */
typedef struct config_param {
  slurmfs_param_t    val;  /* must be first for foop() macro and fuse_opt_parse() */
  ptyp_t             typ;
  char              *default_value;
  param_source_t     allowed;
  param_source_t     actual;
  char              *nm;
  int                h;
  param_source_t   (*collector)(config_param_t *);

  union src_specific {
    ulong_t          slurmconf_off;
  } per_src;
  
  /* fuse_linkage.h */
  slurm_fuse_opt_desc_t *p_fopd;   /* slurm fuse opt param descriptor slurm_fopts[] */
  struct fuse_opt       *p_fo;     /* see fuse_opt.h */
  config_param_t        *p_cnfval; /* slurm fuseslurmfs_fopt_params config value */

  /* fs linkage */
  rnode_t               *p_rnode; 

} config_param_t ;

#define CONFPARAM_REQUIRED ((config_param_t *) NULL)
#define CONFPARAM_MISSINGOK ((config_param_t *) ~NULL)

/*
 * processing functions for sources of configuration state
 *  these are called as the PSRC_* levels are ingested
 *  note PSRC_BIT2FUNC(bit) => index into this table
 *
 * other security models would/could reorder this table
 * and their associated PSRC bits
 */

param_source_t (*param_collection_functab)(config_param_t *)[] = {
  NULL,                     /* PSRC_USERINPUT PSRC_HI */
  construct_fuse_mountopts, /* PSRC_MNT_NONOPT        */
  collect_fuse_mountopts,   /* PSRC_MNT_OPT           */
  collect_env,              /* PSRC_ENVAR             */
  collect_macconf,          /* PSRC_MAC_CONF          */
  collect_slurm,            /* PSRC_SLURM             */
  collect_default,          /* PSRC_DEFAULT           */
  calcul_derived,           /* PSRC_DERIVED, PSRC_LO  */
  NULL;                     /* PSRC_N */
};

/*
 * Parameters with some semantic grouping
 */

typedef enum param_type {
  PTYP_NONE        = 0,
  PTYP_ALPHANUM    = 1,
  PTYP_ALPHA       = 2,
  PTYP_NUMERICHAR  = 3,  /* numeric characters                             */
  PTYP_XATTR       = 4,  /* alphanum, interpreted as an extended attribute */
  PTYP_PATH        = 5,  /* ALPHANUM max(strlen(_POSIX_PATH_MAX))          */
  
  PTYP_FILE        = 6,  /* S_ISREG(PATH) no pre-existence requirement     */
  PTYP_FILEXIST    = 7,  /* S_ISREG(PATH) && must already exist            */
  PTYP_DIREXIST    = 8,  /* S_ISDIR(PATH) && must already exist            */
  PTYP_HOSTNAME    = 9,  /* ALPHANUM  (_POSIX_HOST_MAX, RFC1123)           */

  PTYP_ALPHA_FIRST = PTYP_ALPHANUM,
  PTYP_ALPHA_LAST  = PTYP_HOSTNAME,
  
  PTYP_NUMERIC     = 10,  /* long integer, native format  */
  PTYP_NUMERICTIME = 11,  /* long, interpreted as time    */
  PTYP_NUMTIM_SECS = 12,  /* time, interpreted as seconds */
  PTYP_TRILENE     = 13,  /* integers, E of [-1, 0, 1]    */
  PTYP_BOOL        = 14,  /* integers, E of [0,1]         */ 

  PTYP_INT_FIRST   = PTYP_NUMERIC,
  PTYP_INT_LAST    = PTYP_BOOL,
  
  /* => selinux.h */
  PTYP_SECONTEXT   = 15,  /* XATTR + valid security_context_t */
  PTYP_SE_FIRST    = PTYP_SECONTEXT,
  PTYP_SE_LAST     = PTYP_SECONTEXT,

  /* => slurm.h */
  PTYP_CLUSTER     = 16,  /* HOSTNAME   */
  PTYP_CNTRLMACH   = 17,  /* HOSTNAME   */
  PTYP_PARTITION   = 18,  /* ALPHANUM   */
  PTYP_NODE        = 19,  /* HOSTNAME   */
  PTYP_JOB         = 20,  /* NUMERICHAR */
  PTYP_STEP        = 21,  /* NUMERICHAR */

  PTYP_SLURM_FIRST = PTYP_CLUSTER,
  PTYP_SLURM_LAST  = PTYP_STEP,

  PTYP_FIRST       = PTYP_ALPHANUM,
  PTYP_LAST        = PTYP_STEP,
  PTYP_LEN         = PTYP_STEP+1,
} ptyp_t;

/*
 * flag values for incomplete types
 */
#define NO_DEFAULT_VAL (char *) (0)
#define BOOL_UNSET     (tri_t)  (~0)
  
/*
 * XXX use the slurm.h 'CRAPPY_COMPILER' boolean definition stdbool.h
 */
typedef enum boolean {
  BOOL_FALSE = 0,
  BOOL_TRUE = 1,
} bool_t;

typedef enum trilene {
  UNSET = NO_BOOL,
  FALSE = BOOL_FALSE,
  TRUE  = BOOL_TRUE,
} tri_t;

/*
 * type check functions
 */

tri_t (*param_typecheck_functab)(config_param_t *)[] = {
  typ_alwaysfails,
  typ_isalphanum,
  typ_isalpha,
  typ_numericchar,
  typ_xattr,
  typ_path,
  typ_file,
  typ_filexist,
  typ_direxist,
  typ_host,
  typ_numeric,
  typ_numerictime,
  typ_numtimsecs,
  typ_trilene,
  typ_boolean,
  typ_secontext,  
  typ_slurm_cluster,
  typ_slurm_controlmachine,
  typ_slurm_partition,
  typ_slurm_node,
  typ_slurm_job,
  typ_slurm_jobstep,
  NULL,
};

/*
 * The names in the table below are those that would appear in the context specified by source.allowed
 *
 * For example, SLURM_CONF would be the environment variable
 *              ClusterName would be the parameter name in the slurm.conf file
 *              mac.conf would be the long form mount option
 *              EnforceMAC would be the parameter name in the mac.conf file
 *
 * Many cannot be over-ridden unless 'Debug' is TRUE, to fit a security model where system baseline
 * configuration would be specified by policy, with mount parameters overlain
 * 
 * This defines the parameters and stores the values once collected. A future, functional approach could
 * separate these too.
 *
 * later versions of slurm allow '#Include' logic. Once that is true we could use the slurm configuration
 * routines via its API: slurm_load_ctl_conf(), provided that they are coded to ignore parameters that
 * they don't recognize, or ideally, just store their values in a retrievable fashion
 *
 * The ".nm" entry in this field is used to do the initial link to the config_param_t *p_cp. After that link is established, the p_cp is authoritative.
 */
config_param_t   slurmfs_config[] =
{
  { .nm="BackingStore",     .allowed = bit(PSRC_MAC) | bit(PSRC_MNT), .typ = PTYP_PATH,      .default_value = "/dev/shm/slurmfs" },
  { .nm="ClusterName",      .allowed = bit(PSRC_SLURM),               .typ = PTYP_CLUSTER,                         .collector=collectslurm_confparam, sloof(cluster_name) },
  { .nm="Context",          .allowed = bit(PSRC_MAC) | bit(PSRC_MNT), .typ = PTYP_SECONTEXT,                       .collector=collectmac_secontext },
  { .nm="ControlMachine",   .allowed = bit(PSRC_SLURM),               .typ = PTYP_CNTRLMACH,                       .collector=collectslurm_confparam, sloof(control_machine) },
  { .nm="Debug",            .allowed = bit(PSRC_MAC),                 .typ = PTYP_BOOLEAN,   .default_value = FALSE },
  { .nm="DefContext",       .allowed = bit(PSRC_MAC) | bit(PSRC_MNT), .typ = PTYP_SECONTEXT,                         .collector=collectmac_secontext },
  { .nm="EnforceMAC",       .allowed = bit(PSRC_MAC) | bit(PSRC_MNT), .typ = PTYP_BOOLEAN,   .default_value = UNSET }, /* XXX derive from running env */
  { .nm="Hostname",         .allowed = bit(PSRC_MNT),                 .typ = PTYP_HOSTNAME },
  { .nm="isController",     .allowed = bit(PSRC_DERIVED),             .typ = PTYP_BOOLEAN,   .default_value = FALSE, .collector=calcul_isController },
  { .nm="Jobs",             .allowed = bit(PSRC_SLURM),               .typ = PTYP_JOB,                               .collector=collectslurm_jobs },
  { .nm="MountPoint",       .allowed = bit(PSRC_MNT_NONOPT),          .typ = PTYP_DIREXIST,  .default_value = "/etc/slurm/fs" },
  { .nm="nodes",            .allowed = bit(PSRC_SLURM),               .typ = PTYP_NODE,                              .collector=collectslurm_nodes },
  { .nm="partitions",       .allowed = bit(PSRC_SLURM),               .typ = PTYP_PARTITION,                         .collector=collectslurm_partitions },
  { .nm="SLURM_MAC_CONF",   .allowed = bit(PSRC_ENV),                 .typ = PTYP_FILEXIST,  .default_value = "/etc/slurm/mac.conf"  },
  { .nm="SlurmctldTimeout", .allowed = bit(PSRC_SLURM),               .typ = PTYP_TIME_SECS,                         .collector=collectslurm_confparam, sloof(slurmctld_timeout) },
  { .nm="SlurmdTimeout",    .allowed = bit(PSRC_SLURM),               .typ = PTYP_TIME_SECS,                         .collector=collectslurm_confparam, sloof(slurmd_timeout) },
  { .nm="jobsteps",         .allowed = bit(PSRC_SLURM),               .typ = PTYP_STEP,                              .collector=collectslurm_jobsteps },
  { .nm="unmountWait",      .allowed = bit(PSRC_DERIVED),             .typ = PTYP_TIME_SECS, .default_value = 61,    .collector=calcul_unmountwait },
  { .nm=NULL,               .allowed = PSRC_NOBITS,                   .typ = PTYP_NONE                                              
};

#endif _SLURMFS_CONF_H_
