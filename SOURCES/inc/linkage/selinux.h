

#include <selinux/selinux.h>

#ifndef SELINUX_LINKAGE_H_
#define SELINUX_LINKAGE_H_

/*
 * rmfs <--> selinux
 *
 *  most of rmfs has no direct dependency on selinux
 *  those that do are glued together here
 */

#define XATTR_NM_MAXLEN    (32)
#define XATTR_CTX_MAXLEN   (96)
/*
 * default security context extended attributes name and values
 */
#define CTX_XATTR_NM      "security.selinux"
#define ctx_DEFAULT       "system_u:object_r:slurmd_t"
#define CTX_DEFAULT_S0    "system_u:object_r:slurmd_t:s0"
#define CTX_JOBID         "system_u:object_r:slurm_job_t"
#define CTX_JOBID_S0      "system_u:object_r:slurm_job_t:s0"
#define CTX_JOBID_EXEC    "system_u:object_r:slurm_job_exec_t"
#define CTX_JOBID_EXEC_S0 "system_u:object_r:slurm_job_exec_t:s0"

#define CTX_JOBID_DEFAULT (is_selinux_enabled() && is_selinux_mls_enabled()? CTX_JOBID_S0: CTX_JOBID)
#define CTX_DEFAULT       (is_selinux_enabled() && is_selinux_mls_enabled()? CTX_DEFAULT_S0: ctx_DEFAULT)

#define XATTR_SIG         "security.signature"
#define XATTR_SIGTYP      "security.sigtype"
#define SIGTYP_DJBHASH    "djb_hash"

#endif
