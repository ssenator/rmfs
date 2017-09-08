

#ifndef _SLURMFS_H_
#define _SLURMFS_H_

#include <sys/stat.h>
#include <sys/types.h>

#include <assert.h>
#include <attr/xattr.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <bsd/string.h>
#include <sysexits.h>
#include <time.h>

#include "slurm_linkage.h"
#include "rmfs_conf.h"
#include "rnode.h"

/*
 * attributes are obtained from the resource manager state structures
 * for slurm: partition_info_t, job_info_t, node_info_t, jobstep_info_t,
 * in some cases the attributes are derivative of resource manager state
 * for example, the RNL_ALLOCJOB is a constructed attribute based upon
 * the node and job state. In this case, the relevant rn_buildfn() is used
 * to construct the derived datum.
 *
 * XXXFUTURE use a preprocessor that digests the resource manager API
 * XXXFUTURE data descriptor and structures to generate these tables
 * XXXFUTURE it would be implemented as a dependency of the resource manager API
 * XXXFUTURE that defines the resource manager source structure and state
 */
struct external_datastruct_descriptor {
  char          *nm;
  unsigned int   base_addr;
  off_t          offset;
  ptyp_t         typ;
  unsigned int   local;  /*local parameters are not resource manager parameters*/
  param_source_t src;    /*they are obtained from src instead*/

};

typedef struct external_datastruct_descriptor extdata_info_desc_t;

extdata_info_desc_t partinfodesc_tab[] = {
  { .nm="flags",       .offset=offsetof(partition_info_t, flags),       .typ=PTYP_OPAQUE   },
  { .nm="name",        .offset=offsetof(partition_info_t, name),        .typ=PTYP_ALPHANUM },
  { .nm="nodes",       .offset=offsetof(partition_info_t, nodes),       .typ=PTYP_ALPHANUM },
  { .nm="state_up",    .offset=offsetof(partition_info_t, state_up),    .typ=PTYP_OPAQUE   },
  { .nm="total_cpus",  .offset=offsetof(partition_info_t, total_cpus),  .typ=PTYP_NUMERIC  },
  { .nm="total_nodes", .offset=offsetof(partition_info_t, total_nodes), .typ=PTYP_NUMERIC  },
  { .nm=NULL,                                                                              }
};

extdata_info_desc_t jobinfodesc_tab[] = {
  { .nm="comment",      .offset=offsetof(job_info_t, comment),           .typ=PTYP_ALPHANUM    },
  { .nm="end_time",     .offset=offsetof(job_info_t, submit_t),          .typ=PTYP_NUMERICTIME },
  { .nm="job_state",    .offset=offsetof(job_info_t, job_state),         .typ=PTYP_OPAQUE      },
  { .nm="name",         .offset=offsetof(job_info_t, name),              .typ=PTYP_ALPHANUM    },
  { .nm="nodes",        .offset=offsetof(job_info_t, nodes),             .typ=PTYP_ALPHANUM    },
  { .nm="partition",    .offset=offsetof(job_info_t, partition),         .typ=PTYP_ALPHANUM    },
  { .nm="state_desc",   .offset=offsetof(job_info_t, state_desc),        .typ=PTYP_ALPHANUM    },
  { .nm="state_reason", .offset=offsetof(job_info_t, state_reason),      .typ=PTYP_ALPHANUM    },
  { .nm="submit_time",  .offset=offsetof(job_info_t, submit_t),          .typ=PTYP_NUMERICTIME },
  { .nm="secontext",    .local=TRUE, .src=(bit(PSRC_MAC)|bit(PSRC_MNT)), .typ=PTYP_CONTEXT     },
  { .nm=NULL }
};

extdata_info_desc_t stepinfodesc_tab[] = {
  { .nm="job_id",       .offset=offsetof(job_step_info_t, job_id),    .typ=PTYP_NUMERIC    }, 
  { .nm="name",         .offset=offsetof(job_step_info_t, name),      .typ=PTYP_ALPHANUM   },
  { .nm="nodes",        .offset=offsetof(job_step_info_t, nodes),     .typ=PTYP_ALPHANUM   },
  { .nm="partition",    .offset=offsetof(job_step_info_t, partition), .typ=PTYP_ALPHANUM   },
  { .nm="step_id",      .offset=offsetof(job_step_info_t, step_id),   .typ=PTYP_ALPHANUM   },
  { .nm=NULL }
};

extdata_info_desc_t nodeinfodesc_tab[] = {
  { .nm="arch",          .offset=offsetof(node_info_t, arch),          .typ=PTYP_ALPHANUM   },
  { .nm="cores",         .offset=offsetof(node_info_t, cores),         .typ=PTYP_NUMERIC    },
  { .nm="cpus",          .offset=offsetof(node_info_t, cpus),          .typ=PTYP_NUMERIC    },
  { .nm="features",      .offset=offsetof(node_info_t, features),      .typ=PTYP_ALPHANUM   },
  { .nm="name",          .offset=offsetof(node_info_t, name),          .typ=PTYP_ALPHANUM   },
  { .nm="node_addr",     .offset=offsetof(node_info_t, node_addr),     .typ=PTYP_ALPHANUM   },
  { .nm="node_hostname", .offset=offsetof(node_info_t, node_hostname), .typ=PTYP_HOSTNAME   },
  { .nm="node_state",    .offset=offsetof(node_info_t, node_state),    .typ=PTYP_NODESTATE  },
  { .nm="os",            .offset=offsetof(node_info_t, os),            .typ=PTYP_ALPHANUM   },
  { .nm="real_memory",   .offset=offsetof(node_info_t, real_memory),   .typ=PTYP_NUMERIC    },
  { .nm="reason",        .offset=offsetof(node_info_t, reason),        .typ=PTYP_ALPHANUM   },
  { .nm="sockets",       .offset=offsetof(node_info_t, socket),        .typ=PTYP_NUMERIC    },
  { .nm="threads",       .offset=offsetof(node_info_t, threads),       .typ=PTYP_NUMERIC    },
  { .nm="allocjob",      .local=TRUE, .src=bit(PSRC_DERIVED),          .typ=PTYP_ALLOCJOB   },

  { .nm=NULL, .offset=(-1) }
};

/*
 * see sysexits.h
 */
typedef enum slurmfs_exitcode {
  ExitOK             = EX_OK,
  ErrExit_ENOMEM     = EX_BASE-1, /* [EX_OK+1, EX_BASE-1] for user programs */
  ErrExit_ARGPARSE   = EX_USAGE,
  ErrExit_INCOMPLETE = EX_DATAERR,
  ErrExit_CONFIG     = EX_CONFIG,
  ErrExit_UNAVAIL    = EX_UNAVAILABLE,
  ErrExit_NOPERM     = EX_NOPERM,
  ErrExit_OSERR      = EX_OSERR,
  ErrExit_TEMPFAIL   = EX_TEMPFAIL,
  ErrExit_STUCK      = EX_TEMPFAIL,
  ErrExit_INTERNAL   = EX_SOFTWARE
} slurmfs_exitcode_t;

#endif
