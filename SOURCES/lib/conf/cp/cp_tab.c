
#include "rmfs.h"

/*
 * The names in the table below are those that would appear in the context specified by src.allowed
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
 * The ".nm" entry in this field is used to do the initial link to the config_param_t *p_cp.
 * After that link is established, the p_cp is authoritative.
 *
 */


extern param_source_t   collectslurm_confparam(config_param_t *);
extern param_source_t   collectmac_context(config_param_t *);
extern param_source_t   calcul_isController(config_param_t *);
extern param_source_t   record_mountpoint(config_param_t *);
extern param_source_t   collectslurm_nodes(config_param_t *);
extern param_source_t   collectslurm_partitions(config_param_t *);
extern param_source_t   collectslurm_jobs(config_param_t *);
extern param_source_t   collectslurm_jobsteps(config_param_t *);
extern param_source_t   calcul_rnodepool(config_param_t *);
extern param_source_t   collectos_pidfile(config_param_t *);
extern param_source_t   collectos_pid(config_param_t *);
extern param_source_t   collectos_hostname(config_param_t *);
extern param_source_t   calcul_unmountwait(config_param_t *);
extern param_source_t   collectslurm_api_version(config_param_t *);
extern void            *collectslurm_attr(rn_type_t, config_param_t *, void *);



/*
 * XXXreadabilty: pretify this table by making src.debug an OR on top of src.allowed
 * XXXreadability: further pretify this table by using clever preprocessor macros ##
 *
 */

config_param_t slurmfs_config[] =
{
  { .nm="BackingStore",     .src.allowed=BIT(PSRC_MAC)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_MAC)|BIT(PSRC_MNT)|BIT(PSRC_DEFAULT),
    .typ=PTYP_PATH, .default_val.ue.pathnm="/dev/shm/slurmfs" /*XXX '/dev/shm/slurmfs/'*/                                                                    },
  
  { .nm="ClusterName",      .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM),                 .typ=PTYP_CLUSTER,             .collector=collectslurm_confparam,
    .depends_on.nm="slurm_api_version", .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, cluster_name))                                                                              },
  
  { .nm="context",          .src.allowed=BIT(PSRC_MAC)|BIT(PSRC_DEFAULT),   .src.debug=BIT(PSRC_MAC)|BIT(PSRC_MNT)|BIT(PSRC_DEFAULT),     .typ=PTYP_CONTEXT,   .collector=collectmac_context,
    .default_val.ue.str=ctx_DEFAULT },
  
  { .nm="ControlMachine",   .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM)|BIT(PSRC_MNT),   .typ=PTYP_CNTRLMACH,                       .collector=collectslurm_confparam,
    .depends_on.nm="slurm_api_version", .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, control_machine))                                                                           },
  
  { .nm="Debug",            .src.allowed=BIT(PSRC_MAC)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_MAC)|BIT(PSRC_MNT)|BIT(PSRC_DEFAULT),
    .typ=PTYP_TRILENE,      .default_val.ue.truth=TRUE /*XXX FALSE*/                                                                                                                    },
  
  { .nm="DefContext",       .src.allowed=BIT(PSRC_MAC)|BIT(PSRC_DEFAULT),   .src.debug=BIT(PSRC_MAC)|BIT(PSRC_MNT)|BIT(PSRC_DEFAULT),  .typ=PTYP_CONTEXT,     .collector=collectmac_context,
    .default_val.ue.str=ctx_DEFAULT },
  
  { .nm="EnforceMAC",       .src.allowed=BIT(PSRC_MAC)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_MAC)|BIT(PSRC_DEFAULT), .typ=PTYP_TRILENE,         .default_val.ue.truth=FALSE /*XXX*/    },

  { .nm="fsid",             .src.allowed=BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_DEFAULT), .typ=PTYP_ALPHA,             .default_val.ue.str="slu_rmfs", .default_val.size=9              },
#if defined(PORTING_TO_SLURMv17)
  { .nm="gres_plugins",     .src.allowed=BIT(PSRC_SLURM),   .src.debug=BIT(PSRC_SLURM), .typ=PTYP_GRES_PLUGIN,     .collector=collectslurm_confparam, .depends_on.nm="slurm_api_version",
    .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, gres_plugins)) },
#endif /*PORTING_TO_SLURMv17*/

  { .nm="Hostname",         .src.allowed=BIT(PSRC_DERIVED), .src.debug=BIT(PSRC_DERIVED), .typ=PTYP_HOSTNAME, .default_val.ue.hostname="localhost",
                                                                                                                     .collector=collectos_hostname                                      },
  
  { .nm="isController",     .src.allowed=BIT(PSRC_DERIVED), .src.debug=BIT(PSRC_DERIVED)|BIT(PSRC_DEFAULT), .typ=PTYP_TRILENE,               .default_val.ue.truth=FALSE,
                                                                                                                     .collector=calcul_isController, .depends_on.nm="ControlMachine"    },
  
  { .nm="jobs",             .src.allowed=BIT(PSRC_SLURM),   .src.debug=BIT(PSRC_SLURM), .typ=PTYP_JOB,               .collector=collectslurm_jobs,   .depends_on.nm="slurm_api_version",
    .per_src.slurm.dynamic = TRUE },

#if defined(PORTING_TO_SLURMv17)
  { .nm="job_submit_plugins", .src.allowed=BIT(PSRC_SLURM),   .src.debug=BIT(PSRC_SLURM), .typ=PTYP_JSUB_PLUGIN,     .collector=collectslurm_confparam, .depends_on.nm="slurm_api_version",
    .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, job_submit_plugins)) },
#endif /*PORTING_TO_SLURMv17*/
  
  { .nm="MountPoint",       .src.allowed=BIT(PSRC_MNT_NONOPT)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_MNT_NONOPT)|BIT(PSRC_DEFAULT), .typ=PTYP_DIREXIST,
                                                                                               .default_val.ue.str="fs",                             .depends_on.nm="SLURM_DIR"         },
  
  { .nm="mountpointfile",   .src.allowed=BIT(PSRC_DERIVED)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_DERIVED)|BIT(PSRC_DEFAULT), .typ=PTYP_PATH,        .default_val.ue.str="mp",
                                                                                                                     .collector=record_mountpoint,   .depends_on.nm="varrun"            },

  { .nm="nodes",            .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_NODE,                .collector=collectslurm_nodes,  .depends_on.nm="slurm_api_version",
    .per_src.slurm.dynamic = TRUE },

  { .nm="partitions",       .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_PARTITION,       .collector=collectslurm_partitions, .depends_on.nm="slurm_api_version",
    .per_src.slurm.dynamic = TRUE },

  { .nm="rnodepool",        .src.allowed=BIT(PSRC_DERIVED), .src.debug=BIT(PSRC_DERIVED), .typ=PTYP_NUMERIC,         .collector=calcul_rnodepool,    .depends_on.nm="partitions"        },

  { .nm="slurm_api_version", .src.allowed=BIT(PSRC_DERIVED)|BIT(PSRC_SLURM), .src.debug=BIT(PSRC_DERIVED)|BIT(PSRC_SLURM), .typ=PTYP_SLURMVERSION,   .collector=collectslurm_api_version },
  /*not in the slurm_info structure so requires custom collector */

  { .nm="SLURM_MAC_CONF",   .src.allowed=BIT(PSRC_ENV)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_ENV)|BIT(PSRC_DEFAULT),
#ifndef XXXDEBUG
    .typ=PTYP_FILE,
#else
    .typ=PTYP_FILEXIST,
#endif    
    .default_val.ue.str="mac.conf", .depends_on.nm="SLURM_DIR"                                                                                                                          },

  { .nm="SLURM_DIR",        .src.allowed=BIT(PSRC_ENV)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_ENV)|BIT(PSRC_DEFAULT), .typ=PTYP_DIR, .default_val.ue.pathnm="/etc/slurm"                },

  { .nm="SlurmctldTimeout", .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_SLURMTMOUT,         .collector=collectslurm_confparam,
    .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, slurmctld_timeout)),                                                                    .depends_on.nm="slurm_api_version"      },

  { .nm="SlurmUser",        .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_SLURMUID,                           .collector=collectslurm_confparam,
    .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, slurm_user_id)),                                                                        .depends_on.nm="slurm_api_version",     },

  { .nm="SlurmdUser",       .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_SLURMUID,                           .collector=collectslurm_confparam,
    .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, slurmd_user_id)),                                                                       .depends_on.nm="slurm_api_version"      },

  { .nm="SlurmdTimeout",    .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_SLURMTMOUT,                       .collector=collectslurm_confparam,
    .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, slurmd_timeout)),                                                                       .depends_on.nm="slurm_api_version"      },

  { .nm="jobsteps",         .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_STEP,            .collector=collectslurm_jobsteps, .depends_on.nm="slurm_api_version",
    .per_src.slurm.dynamic = TRUE   },
    
  { .nm="slurm_user_name",  .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_SLURMUNAME,     .collector=collectslurm_confparam, .depends_on.nm="slurm_api_version",
    .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, slurm_user_name)) },
  { .nm="slurmd_user_name",  .src.allowed=BIT(PSRC_SLURM), .src.debug=BIT(PSRC_SLURM), .typ=PTYP_SLURMDUNAME,   .collector=collectslurm_confparam, .depends_on.nm="slurm_api_version",
    .per_src.slurm.off=(offsetof(slurm_ctl_conf_t, slurmd_user_name)) },

  { .nm="pidfile",          .src.allowed=BIT(PSRC_MAC)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_MAC)|BIT(PSRC_MNT)|BIT(PSRC_DEFAULT), .typ=PTYP_PATH,
                                                                                                       .default_val.ue.str="pid", .collector=collectos_pidfile, .depends_on.nm="varrun" },
  { .nm="pid",              .src.allowed=BIT(PSRC_DERIVED), .src.debug=BIT(PSRC_DERIVED), .typ=PTYP_PID,                               .collector=collectos_pid                         },

  { .nm="unmountwait",      .src.allowed=BIT(PSRC_DERIVED)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_DERIVED)|BIT(PSRC_DEFAULT), .typ=PTYP_NUMTIM_SECS,
                                                                                               .default_val.ue.l=61, .collector=calcul_unmountwait, .depends_on.nm="SlurmdTimeout"      },
  { .nm="varrun",           .src.allowed=BIT(PSRC_DERIVED)|BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_DERIVED)|BIT(PSRC_DEFAULT), .typ=PTYP_DIR, .default_val.ue.pathnm="/var/run/slurmfs",
                                                                                                                                             .depends_on.nm="pid"                       },

  { .nm="version",          .src.allowed=BIT(PSRC_DEFAULT), .src.debug=BIT(PSRC_DEFAULT), .typ=PTYP_NUMERIC, .default_val.ue.ul=
#ifdef __COMPILE_DATE_TIME__    
    (unsigned long)(__COMPILE_DATE_TIME__)                                                                                                                                                               },
#else    
    20141224816L                                                                                                                                                                        },
#warning >>> Using default version: __COMPILE_DATE_TIME__ <<<
#endif  
  { .nm=NULL,               .src.allowed=PSRC_NOBITS,                   .typ=PTYP_NONE                                                                                                  }
};


char *
ptyp2pname_tab[] = {
  "none",
  "opaque (first)",
  "alphanumeric (alpha-first)",
  "alpha",
  "numeric char",
  "alpha list (ptr 2 ptr)",
  "extended attribute",
  "pathname (fs-visible-first)",
  "file",
  "file (must exist)",
  "symbolic link",
  "directory (must exist)",
  "directory (fs-visible-last)",
  "hostname (alpha-last, host-first, host-last)",
  "numeric (int-first)",
  "numeric time",
  "numeric time, in seconds",
  "numeric signed",
  "unsigned int",
  "unsigned int16",
  "unsigned int32",
  "uid",
  "pid", 
  "trilene",
  "boolean",
  "signature (numeric-last)",
  "context (cntxt-first, cntxt-last)",
  "slurm: cluster name (slurm-first)",
  "slurm: control machine",
  "slurm: partition",
  "slurm: node",
  "slurm: job",
  "slurm: job-step",
  "slurm: allocated job association (of a node)",
  "slurm: node-state",
  "slurm: API version",
  "slurm: uid_t",
  "slurm: timeout",
  "slurm: spank env",
  "slurm: spank env size",
  "slurm: jobsubmit plugins",
  "slurm: gres plugins",
  "slurm: slurm (ctld) user name",
  "slurm: slurmd user name (slurm-last)",
  "<guard>",
  NULL
};


/*
 * slurm fuse options
 *  This table drives the linkage between fuse mount options and slurmfs configuration parameters
 *
 *  fuse parsing functions may pass back identical keys for differing options
 *  ie. short-form option "-x" vs. a long-form option "--extended" => KEY_EXT
 *  so, each of these that are not OPT_NONOPT count as two entries in the fuse_opt table
 *
 * fuse uses the offset() as the location to store the parameter, as well as the key
 * this double meaning prevents embedding the storage location into this table,
 * and forces the requirement for a separate structure with a unique location for
 * each parameter.
 */

slurm_fuse_opt_desc_t slurm_fopts[] =
  {
    { ._s=NULL, ._long="",
      .nm="MountPoint",
      .help="mount point of the slurm file system ",
      .oflg=(OPT_MANDATORY|OPT_NONOPT)
    },

    { ._s="-b %s", ._long="--backingstore=%s",
      .nm="BackingStore",
      .help = "local extended-attributed enabled file system",
      .oflg=OPT_MANDATORY
    },

    { ._s="-c %s", ._long="--context=%s",
      .nm="Context",
      .help="SELinux context",
      .oflg=OPT_SELECT_ONE
    },
    
    { ._s="-d %s", ._long="--defcontext=%s" ,
      .nm="DefContext",
      .help="default SELinux context",
      .oflg=OPT_SELECT_ONE
    },
    
    { ._s="-e", ._long="--enforce",
      .nm="EnforceMAC",
      .help="enforce Mandatory Access Control limits",
      .oflg=OPT_NONE /*or: OPT_MANDATORY*/
    }, 
    /*
     * { ._s="-V", ._long="--Verbose",
     * .nm="Verbose",
     * .help="enable runtime chatter",
     * .oflg=OPT_NONE
     * },
     */
    { ._s="-w", ._long="--wait %d",
      .nm="unmountwait",
      .help="maximum seconds to wait for previous mount to unmount",
      .oflg=OPT_NONE
    },
    
    /* only 'valid if Debug' options follow; note: capitalized as a reminder */
    { ._s="-C %s", ._long="--cluster=%s",
      .nm="ClusterName",
      .help="explicitly set slurmfs' clustername [Debug only]",
      .oflg=OPT_VALID_IF_DEBUG
    },

    { ._s="-D", ._long="--DEBUG",
      .nm="Debug",
      .help="enable debug-specific algorithms and options",
      .oflg=OPT_NONE
    },

    { ._s="-H %s", ._long="--hostname=%s",
      .nm="Hostname",
      .help="explicitly set slurmfs' hostname [Debug only]",
      .oflg=OPT_VALID_IF_DEBUG
    },

    { ._s="-M %s", ._long="--MACconf=%s",
      .nm="SLURM_MAC_CONF",
      .help="explicitly set Mandatory Access Control configuration file [Debug only]",
      .oflg=OPT_VALID_IF_DEBUG
    },

    { .nm=NULL }
};
/*
 * The fuse_opt table is constructed from the slurm_fopts array
 * see: collect_fuse_mountopts(), construct_fuse_mountopts()
 */
struct fuse_opt *p_fuseopts_tbl = NULL;  /* fuse_opt table (conf_tab.c) */
int              fuseopts_len   = 0;     /* its length that has been used */

int fo_max_len = 1 + N_SLURM_FOPTS + N_SLURMFS_CONFIG; /* actual total allocated */


/*
 * parameter source table of source names
 * => decode_psrc(param_source_t) 
 */
char *psrc2srcname_tab[] = {
  "<none>",
  "<MAC_CONF>"
  "<SLURM>",
  "<ENV>",
  "<MNTopt>",
  "<MNTnonopt>",
  "<FUSE>",
  "<user>",
  "<default>",
  "<derived>",
  NULL
};
  
int
get_fo_max_len() {
  return fo_max_len;
}

char *
decode_psrc(param_source_t psrc) {
  param_source_t s;
  int            any;
  char          *pbuf;

  pbuf = calloc(255, sizeof(char));
  any = 0;
  for (s = PSRC_MOST_TRUSTED;
           PSRC_TEST(s, PSRC_LEAST_TRUSTED);
               s = PSRC_NXT(s)) {

    if (ISSET(psrc, s)) {
      snprintf(pbuf, 254, "%s%s", any? pbuf: "", psrc2srcname_tab[s]);
      any++;
    }
  }
  /*caller frees pbuf*/
  return pbuf;
}
