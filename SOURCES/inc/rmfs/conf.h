/*
 * configuration control of rmfs
 */

/*
 * these are parameters that control the configuration of rmfs
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
 *  as indicated by the truth value of the special 'Debug' configuration parameter.
 *
 * Mount options which are processed by fuse are used to construct
 * a fuse_opt table (see fuse/fuse_opt.h, fuse_linkage.h)
 */


#ifndef RMFS_CONF_H_
#define RMFS_CONF_H_

#include "rmfs.h"
#include "linkage/slurm.h"

#define CP_UNK_SIZE      (~0)
/*
 * rmfs_params are a common format for the values collected from various sources
 */

struct rmfs_param {
  union ue {        /* must have offsetof(0) for fuse_opt_param processing */
    /*opaque*/
    void     *ptr;
    void     *base_addr;
    
    /*alpha*/
    char     *str;
    char     *pathnm;
    char     *hostname;
    char    **array_str;

    /*numeric*/
    unsigned long ul;
    unsigned int  ui;
    uint16_t      ui_16;
    uint32_t      ui_32;
    long          l;
    int           i;
    uid_t         uid;
    gid_t         gid;
    pid_t         pid;
    time_t        time;
    tri_t         truth;
    bool_t        btruth;

    /*backingstore*/
    struct aligned_backingstore_record *bs;  /* bs_record_t   *bs; */
  } ue;
  size_t size; /* for char *, calloc()'d size, unless opaque */
  
  struct path_descriptor {
    char        *fullpath;
    pid_t        owner;
    int          fd;
    FILE        *fstr;
    int          is_mmapped:1;
    int          is_filestr:1;
    int          ours:1;
  } pd;
};
typedef struct rmfs_param rmfs_param_t;

/* configuration parameter value and characteristics of the source */
struct config_param {
  rmfs_param_t     val;  /* must start at offsetof(0) for fuse_opt param linkage */
  rmfs_param_t     default_val;
  ptyp_t           typ;

  struct src {
    param_source_t   allowed;
    param_source_t   actual;
    param_source_t   debug;
  } src;
  char            *nm;
  int              h;
  param_source_t (*collector)(struct config_param *);
  struct depends_on {
    char                *nm;
    struct config_param *p_cp;
  } depends_on;

  /* fuse linkage */
  struct slurm_fuse_opt_desc *p_fopd;   /* slurm fuse opt param descriptor slurm_fopts[] */
  struct fuse_opt            *p_fo;     /* see fuse_opt.h */

  /* per_src linkage */
  struct src_specific {
    struct slurm_info { /* see -> slurm.h */
      ptyp_t                        parent_type;
      void                         *base_addr;
      off_t                         off;
      partition_info_msg_t         *pim;
      node_info_msg_t              *nim;
      job_info_msg_t               *jim;
      job_step_info_response_msg_t *stim;
      int                           dynamic:1;
      int                           :0;
    } slurm;
    struct rmfs_info {
      ptyp_t                        parent_type;
      int                           local:1;
      int                           fs:1;
      int                           backingstore:1;
      int                           :0;
    } rmfs;
  } per_src;
  
  /* fs linkage */
  struct rnode        *p_rnode;
  struct config_param *p_nxt; /* arbitrary nxt list, used by extended attributes in fs */
};
typedef struct config_param config_param_t;

#define CONFPARAM_REQUIRED  ((config_param_t *) 0)
#define CONFPARAM_MISSINGOK ((config_param_t *) ~0)

#define IS_VALID_HASH(h)    (((h) != 0) && ((h) != (~0)))
#define IS_INVALID_HASH(h)  (((h) == 0) || ((h) == (~0)))

/*
 * functions which manipulate config_param_t
 * or are used heavily to support them
 */
extern config_param_t *getconfig_fromnm(char *);
extern config_param_t *getconfig_fromnm_nohash(char *);
extern config_param_t *getconfig_fromhash(unsigned long);

extern void            effluviate_config(void);
extern void            effluviate_one_cp(config_param_t *);
extern char           *t2tristr(tri_t);
extern char           *b2boolstr(bool_t);

extern config_param_t *dup_cp(config_param_t *);
extern bool_t          derefable_cp(const config_param_t *);
extern void            init_hash_cp(config_param_t *);

extern void            set_val_ul(config_param_t *, unsigned long);
extern void            set_val_l(config_param_t *, long);
extern void            set_val_ui(config_param_t *, unsigned int);
extern void            set_val_uid(config_param_t *, uid_t);
extern void            set_val_pid(config_param_t *, pid_t);
extern void            set_val_truth(config_param_t *, tri_t);

extern void            set_val_ptr(config_param_t *, void *);
extern void            set_val_charptr(config_param_t *, char *);

extern void            set_val_slurm(config_param_t *, void *);

extern tri_t           Debug(void);
extern tri_t           isCtlr(void);

extern param_source_t (*param_collection_functab[])(config_param_t *);

extern tri_t typ_check(ptyp_t, rmfs_param_t *);
extern int   typ_copyout(config_param_t *, char *, size_t);

/*
 * configuration parameters
 *  see rmfs_conf.c for definitions
 *  and the other files listed for usage or related data structures
 * consumed by:  rmfs_rnode.c/fuse_linkage.h/slu_rmfs.c
 */
extern config_param_t slurmfs_config[];
extern int get_fo_max_len(void);

/*
 * These are the types that fuse knows how to interpret
 * We impose additional semantics on, for example,
 *    (char *) that are filenames, pathnames, hostnames
 *    (char *) that are extended attributes, and may be a security context
 */

#endif
