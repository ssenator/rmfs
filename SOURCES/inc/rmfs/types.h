/*
 * fundamental types
 *
 * these are enums, flags, masks and other basic variations on C types
 * complex types are in rmfs_conf.h
 */

#ifndef RMFS_TYPE_H_
#define RMFS_TYPE_H_

#include <stddef.h>
#include <stdbool.h>
#include <sysexits.h>

#ifndef FALSE

enum boolean {
  BFALSE = 0,
  BTRUE  = 1
};
typedef enum boolean bool_t;

#else
typedef _Bool bool_t;

#endif

enum trilene {
  UNSET = (~0),
  FALSE = false,
  TRUE  = true
};
typedef enum trilene tri_t;

/*
 * option flags
 *  these are actually bit positions, so the actual flag is 2^(OPT_FLAG)
 */

enum opt_flag {
  OPT_INVAL          = ~0,
  OPT_NONE           = 0,
  OPT_VALID_IF_DEBUG = 1,
  OPT_DEBUG          = OPT_VALID_IF_DEBUG,
  OPT_MANDATORY      = 2,
  OPT_MAND           = OPT_MANDATORY,
  OPT_EXCLUSIVE_ALL  = 4,
  OPT_EXCL_ALL       = OPT_EXCLUSIVE_ALL,
  OPT_EXCLUSIVE      = OPT_EXCLUSIVE_ALL,
  OPT_SELECT_ONE     = 8,
  OPT_ONE            = OPT_SELECT_ONE,
  OPT_NONOPT         = 16 /* => FUSE_OPT_KEY_NONOPT, indicating a non-option "no '-'" */
};
typedef enum opt_flag opt_flg_t;

/*
 * parameter source indicators
 * note that these are actually used to form flags via BIT() macros
 *
 * XXXFUTURE allow order of sources
 * XXXFUTURE based on security profile: 'production', 'inputtrusted'
 *
 * XXXFUTURE name collision with bit values (for the low order powers of 2)
 *
 * changing the ordering of this table implies requires the param_collection_functab
 *
*/

enum param_source {
  PSRC_NONE          = 0,
  PSRC_NOBITS        = PSRC_NONE,
  
  PSRC_MAC_CONF      = 1,
  PSRC_MAC           = PSRC_MAC_CONF,

  PSRC_SLURM         = 2,
  
  PSRC_ENVAR         = 3,
  PSRC_ENV           = PSRC_ENVAR,

  PSRC_MNT_OPT       = 4,
  PSRC_MNT           = PSRC_MNT_OPT,
  PSRC_MNT_NONOPT    = 5, /*to trigger fuse option collection */
  PSRC_FUSE          = 6,
  
  PSRC_USERINPUT     = 7, /*if any*/
  
  PSRC_DEFAULT       = 8,
  PSRC_DERIVED       = 9, /*more trusted than this position implies, but should be last*/

  PSRC_MAX           = PSRC_DERIVED,
  PSRC_N             = PSRC_MAX+1 /*@+enumint@*/,
  PSRC_LAST          = PSRC_MAX,
  PSRC_LOPRI         = PSRC_LAST,
  PSRC_LEAST_TRUSTED = PSRC_LOPRI,
  PSRC_FIRST         = PSRC_NONE+1 /*@+enumint@*/,
  PSRC_HIPRI         = PSRC_FIRST,
  PSRC_MOST_TRUSTED  = PSRC_HIPRI
};
typedef enum param_source param_source_t;

extern char *decode_psrc(param_source_t);
#define PSRCNAME(psrc) decode_psrc(psrc)

#define BIT(b)       (1<<(b))

#define PSRC_ALLBITS ( BIT(PSRC_DERIVED)   | \
		       BIT(PSRC_DEFAULT)   | \
		       BIT(PSRC_SLURM)     | \
		       BIT(PSRC_MAC_CONF)  | \
		       BIT(PSRC_ENVAR)     | \
		       BIT(PSRC_MNT_NONOPT)| \
		       BIT(PSRC_MNT_OPT)   | \
		       BIT(PSRC_USERINPUT)   \
		     )
#define ISSET(tst,b) ((tst) & BIT(b))

#define ANY(tst)     (ISSET((PSRC_ALLBITS), (tst)))
#define NOBITS(tst)  (tst == PSRC_NOBITS)

#define PSRC_LSB(tst)    ((tst) & ((tst)-1))

extern unsigned int psrc_msb(unsigned int);
#define PSRC_MSB(tst)   (psrc_msb(tst)) 

#define PSRC_NXT(b)       ((b)+1)
#define PSRC_TEST(s, tst) ((s) <= (tst))

/*
 * Parameter types with some semantic grouping
 *
 * XXX make the related types bits?
 * XXX and the complex types bit combinations?
 * XXXFUTURE: when we parse external data definitions and APIs to generate internal structures
 */

enum param_type {
  PTYP_NONE         = 0,
  
  PTYP_OPAQUE       = PTYP_NONE+1,
  PTYP_FIRST        = PTYP_OPAQUE,

  PTYP_ALPHANUM     = PTYP_OPAQUE + 1,
  PTYP_ALPHA_FIRST  = PTYP_ALPHANUM,
  PTYP_ALPHA        = PTYP_ALPHA_FIRST + 1,
  PTYP_NUMERICHAR   = PTYP_ALPHA_FIRST + 2,  /* numeric characters                             */
  PTYP_ALPHA_P2P    = PTYP_ALPHA_FIRST + 3,  /* ptr to ptr to alpha */
  PTYP_XATTR        = PTYP_ALPHA_FIRST + 4,  /* alphanum, interpreted as an extended attribute */
  PTYP_PATH         = PTYP_ALPHA_FIRST + 5,  /* ALPHANUM max(strlen(_POSIX_PATH_MAX))          */
  PTYP_FSVIS_FIRST  = PTYP_PATH,             /* visible in file system */
  
  PTYP_FILE         = PTYP_ALPHA_FIRST + 6,  /* S_ISREG(PATH) no pre-existence requirement     */
  PTYP_FILEXIST     = PTYP_ALPHA_FIRST + 7,  /* S_ISREG(PATH) && must already exist            */
  PTYP_SYM          = PTYP_ALPHA_FIRST + 8,  /* S_ISLNK(PATH)                                  */  
  PTYP_DIREXIST     = PTYP_ALPHA_FIRST + 9,  /* S_ISDIR(PATH) && must already exist            */
  PTYP_DIR          = PTYP_ALPHA_FIRST + 10, /* S_ISDIR(PATH), no-preexistence req'd            */
  PTYP_FSVIS_LAST   = PTYP_DIR,
  PTYP_HOST         = PTYP_ALPHA_FIRST + 11, /* ALPHANUM  (_POSIX_HOST_MAX, RFC1123)          */
  PTYP_HOST_FIRST   = PTYP_HOST,
  PTYP_HOSTNAME     = PTYP_HOST,
  PTYP_HOST_LAST    = PTYP_HOST,
  PTYP_ALPHA_LAST   = PTYP_HOST,

  PTYP_NUMERIC        = PTYP_ALPHA_LAST + 1,   
  PTYP_INT_FIRST      = PTYP_NUMERIC,        /* unsigned long integer */
  PTYP_NUMERICTIME    = PTYP_INT_FIRST + 1,  /* unsigned long => time_t    */
  PTYP_NUMTIME        = PTYP_NUMERICTIME,
  PTYP_NUMTIM_SECS    = PTYP_INT_FIRST + 2,  /* time_t, interpreted as seconds */
  PTYP_NUMTIME_SECS   = PTYP_NUMTIM_SECS,
  PTYP_NUMSIGNED      = PTYP_INT_FIRST + 3,  /* signed long integer */
  PTYP_UNSIGNED_INT   = PTYP_INT_FIRST + 4,
  PTYP_UNSIGNED_INT16 = PTYP_INT_FIRST + 5,
  PTYP_UNSIGNED_INT32 = PTYP_INT_FIRST + 6,
  PTYP_UID            = PTYP_INT_FIRST + 7,
  PTYP_PID            = PTYP_INT_FIRST + 8,
  PTYP_TRILENE        = PTYP_INT_FIRST + 9,  /* integers, E of [-1, 0, 1]    */
  PTYP_BOOLEAN        = PTYP_INT_FIRST + 10,  /* integers, E of [0,1]         */
  PTYP_SIGNATURE      = PTYP_INT_FIRST + 11,
  PTYP_INT_LAST       = PTYP_SIGNATURE,
  
  PTYP_CONTEXT      = PTYP_INT_LAST + 1,     /* XATTR++ */
  PTYP_CNTXT_FIRST  = PTYP_CONTEXT,
  PTYP_CNTXT        = PTYP_CONTEXT,
  PTYP_CNTXT_LAST   = PTYP_CONTEXT,

  /* => slurm.h */
  PTYP_CLUSTER      = PTYP_CNTXT_LAST + 1,
  PTYP_SLURM_FIRST  = PTYP_CLUSTER,
  PTYP_CNTRLMACH    = PTYP_SLURM_FIRST + 1,  /* HOST   */
  PTYP_PARTITION    = PTYP_SLURM_FIRST + 2,  /* ALPHANUM   */
  PTYP_NODE         = PTYP_SLURM_FIRST + 3,  /* HOSTNAME   */
  PTYP_JOB          = PTYP_SLURM_FIRST + 4,  /* NUMERICHAR */
  PTYP_STEP         = PTYP_SLURM_FIRST + 5,  /* NUMERICHAR */
  PTYP_ALLOCJOB     = PTYP_SLURM_FIRST + 6,  /* XXXfuture SYMLINK, PTYP_FILE */
  PTYP_NODESTATE    = PTYP_SLURM_FIRST + 7,  /* slurm-api specific */
  PTYP_SLURMVERSION = PTYP_SLURM_FIRST + 8,
  PTYP_SLURMUID     = PTYP_SLURM_FIRST + 9,
  PTYP_SLURMTMOUT   = PTYP_SLURM_FIRST + 10,
  PTYP_SPANKENV     = PTYP_SLURM_FIRST + 11,
  PTYP_SPANKENVSIZE = PTYP_SLURM_FIRST + 12,
  PTYP_JSUB_PLUGIN  = PTYP_SLURM_FIRST + 13,
  PTYP_GRES_PLUGIN  = PTYP_SLURM_FIRST + 14,
  PTYP_SLURMUNAME   = PTYP_SLURM_FIRST + 15,
  PTYP_SLURMDUNAME  = PTYP_SLURM_FIRST + 16,
  PTYP_SLURM_LAST   = PTYP_SLURM_FIRST + 16,

  PTYP_LAST         = PTYP_SLURM_LAST,
  PTYP_GUARD        = PTYP_LAST + 1,
  PTYP_LEN          = PTYP_GUARD          /* extra for placeholder in PTYP_NONE */
};
typedef enum param_type ptyp_t;

#define IS_SLURM_TYPE(ptyp)     ((ptyp) >= PTYP_SLURM_FIRST && ((ptyp) <= PTYP_SLURM_LAST))
#define IS_NUMERIC_TYPE(ptyp)   ((ptyp) >= PTYP_INT_FIRST && ((ptyp) <= PTYP_INT_LAST))
#define IS_NUMSIGNED_TYPE(ptyp) ((ptyp) == PTYP_NUMSIGNED)
#define IS_SIGNATURE_TYPE(ptyp) ((ptyp) == PTYP_SIGNATURE)
#define IS_ALPHA_TYPE(ptyp)     ((ptyp) >= PTYP_ALPHA_FIRST && ((ptyp) <= PTYP_ALPHA_LAST))
#define IS_CONTEXT_TYPE(ptyp)   ((ptyp) >= PTYP_CNTXT_FIRST && ((ptyp) <= PTYP_CNTXT_LAST))
#define IS_XATTR_TYPE(ptyp)     ((ptyp) == PTYP_XATTR || IS_CONTEXT_TYPE(ptyp))
#define IS_OPAQUE_TYPE(ptyp)    ((ptyp) == PTYP_OPAQUE)
#define IS_VALID_TYPE(ptyp)     ((ptyp) >= PTYP_FIRST && (ptyp) <= PTYP_LAST)
#define IS_BOOLEAN_TYPE(ptyp)   ((ptyp) == PTYP_BOOLEAN)
#define IS_TRILENE_TYPE(ptyp)   ((ptyp) == PTYP_TRILENE)
#define IS_TRUTH_TYPE(ptyp)     (IS_BOOLEAN_TYPE(ptyp) || IS_TRILENE_TYPE(ptyp))
#define IS_LOGICAL_TYPE(ptyp)   (IS_TRUTH_TYPE(ptyp))
#define IS_FSVIS_TYPE(ptyp)     ((ptyp) >= PTYP_FSVIS_FIRST && (ptyp) <= PTYP_FSVIS_LAST)
#define IS_HOST_TYPE(ptyp)      ((ptyp) >= PTYP_HOST_FIRST && (ptyp) <= PTYP_HOST_LAST)

extern char *ptyp2pname_tab[];  /*rmfs_rnode.c*/
extern ptyp_t slurm2basetyp[];  /*slu_rmfs.c*/

/* the value that gets emitted for opaque types */
#define OPAQUE_EMIT 0x0D06F00D

/*
 * => sysexits.h
 */
enum slurmfs_exitcode {
  ExitOK             = EX_OK,
  Exit_OK            = EX_OK,
  ErrExit_ENOMEM     = EX__BASE-1, /* [EX_OK+1, EX_BASE-1] for user programs */
  ErrExit_ARGPARSE   = EX_USAGE,
  ErrExit_INCOMPLETE = EX_DATAERR,
  ErrExit_CONFIG     = EX_CONFIG,
  ErrExit_UNAVAIL    = EX_UNAVAILABLE,
  ErrExit_NOPERM     = EX_NOPERM,
  ErrExit_OSERR      = EX_OSERR,
  ErrExit_TEMPFAIL   = EX_TEMPFAIL,
  ErrExit_INTERNAL   = EX_SOFTWARE,
  
  ErrExit_STUCK      = EX_OK+1, /*previous session won't relinquish*/
  ErrExit_SLURM      = EX_OK+2, /*some slurm problem*/
  ErrExit_ASSERT     = EX_OK+3, /* *must be unique* internal inconsistency */
  ErrExit_NOMEM      = EX_OK+4, /* out of memory */
  ErrExit_WARN       = EX_OK+9  /* not a cause to Exit */
};
typedef enum slurmfs_exitcode rmfs_exitcode_t;

/*
 * exit codes above are invoked throughout the code, but all funnel to
 * the following, defined in usage.c
 */

extern void Usage(int, char *);
extern void ErrExit(int, char *);
extern void CleanExit(void);

/*constructed error message based upon various configuration tables*/
extern char *pusage_msg;

#ifndef max
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif

#ifndef min
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif

#endif
