/*
 * See the fs tree ascii art in rmfs_rnode.c and slu_rmfs.c
 * rnode = resource manager node
 *
 *  Design goal:
 *    as little as possible or, preferably, no slurm-specific dependency
 *    in this file system representation of resource manager data
 *
 *    For now, per-datum functions encapsulate knowledge of specific attributes,
 *    data structures or the API to collect them. If this were to be generalized
 *    to multiple resource managers, this would become a switch function table.
 *
 * 1. Most nodes are represented as directories:
 *    A. If the root node, this represents file system data structures
 *       as collected in the configuration parameters
 *
 *    B. If these are RM data, then they are of the following types:
 *
 *       i. arrays of RM datums, such as "partitions", "jobs", "nodes", "jobsteps"
 *          The directory name is an exact string, corresponding to the datum type.
 *          ex. the directory "nodes" appears as a directory with contents
 *              associated with each node list entry
 *           
 *       ii. named RM datums:
 *            a. fixed names: ex. "ControllerMachine", "ClusterName"
 *            b. dynamic names: ex. node123, job456, jobstep012
 *
 * 2. Attributes of directories are represented as leaf nodes, or files
 *    A. Read/Write/control:
 *        Ex. "control" - will allow commands to be sent to the enclosing object
 *                        currently this is how a request to "unmount" is sent to
 *                        the file system object, as represented by the mount point
                          the commands are attributes of the parent node
 *            "context" - allows the "set" and "get" of arbitrary context values
 *
 *    B. Read-Only:
 *        i. "Normal" nodes
 *            Ex. "update_time" - shows the datum for the relevant slurm object
 *                            such as the slurm config message
 *            "state" - for a node shows whether the node is "allocated", "draining", &c.
 *            ...
 *
 *        ii. Symlinks refer to other portions of the tree, such as to point from an
 *            allocated node to the job which has been allocated to run on it.
 *            [XXXFUTURE: mini-internal "bind" mount]
 *
 * Resource manager nodes "rnodes" types builder functions are indexed by type.
 * Known, static rnodes have an inunmber identical to their type.
 * ex. the fs root inode is of type RND_ROOT and has an inumber of (int) RND_ROOT
 * The dynamic nodes (partition-name, node-name, jobs, job stepid) have inumbers
 * that are not identical to their types.
 */

#ifndef RNODE_H_
#define RNODE_H_

#include "rmfs.h"

/*
 * some of the node types are singletons:
 *  RND_ROOT, RND_CLUSTER, RND_PARTS, RND_JOBS
 *
 * the rest may have multiple instances:
 *  RND_PARTNAME,
 *  RND_NODENAME,
 *  RND_JOBID/RND_JOBNAME,
 *  RND_JOBSTEPS,
 *  RND_JOBSTEPID/RND_JOBSTEPNAME,
 *  RND_CONTROL,
 *  RND_NODE_STATE,
 *  RNL_ALLOCJOB, 
 *  RNF_ATTRIBUTE/RNF_ATTR,
 *  RNF_KNOB
 *  RNF_CONTEXT
 */

enum rnode_type {
  RN_NONE         = 0,
  RN_FIRST        = RN_NONE,
  RN_PROVISION    = RN_FIRST,
  RN_GUARD        = RN_FIRST + 1 /*@+enumint@*/,
  
  RND_ROOT        = RN_FIRST + 2 /*@+enumint@*/,
  RND_CLUSTER     = RN_FIRST + 3 /*@+enumint@*/,

  RND_PARTS       = RN_FIRST + 4 /*@+enumint@*/,
  RND_PARTITIONS  = RND_PARTS,
  RND_PARTNAME    = RN_FIRST + 5 /*@+enumint@*/,

  RND_NODES       = RN_FIRST + 6 /*@+enumint@*/,
  RND_NODENAME    = RN_FIRST + 7 /*@+enumint@*/,

  RND_JOBS        = RN_FIRST + 8 /*@+enumint@*/,
  RND_JOBID       = RN_FIRST + 9 /*@+enumint@*/,
  RND_JOBNAME     = RND_JOBID,

  RND_JOBSTEPS    = RN_FIRST + 10 /*@+enumint@*/,
  RND_JOBSTEPID   = RN_FIRST + 11 /*@+enumint@*/,
  RND_JOBSTEP     = RND_JOBSTEPID,
  RND_JOBSTEPNAME = RND_JOBSTEPID,

 #ifdef XXXFUTURE
  RND_JOBSTEP_CTX,
  RND_JOBSTEP_LAUNCH,
  RND_RESV,
  RND_RESERVATION,
#endif

  RNF_ALLOCJOB    = RN_FIRST + 12 /*@+enumint@*/,
  RND_NODESTATE   = RN_FIRST + 13 /*@+enumint@*/,

  RND_ATTRIBUTES  = RN_FIRST + 14 /*@+enumint@*/,
  RNF_ATTRIBUTE   = RN_FIRST + 15 /*@+enumint@*/,
  RNF_ATTR        = RNF_ATTRIBUTE,

  RND_CONTROL     = RN_FIRST + 16 /*@+enumint@*/,
  RND_CTRL        = RND_CONTROL,

  RNF_CONTEXT     = RN_FIRST + 17 /*@+enumint@*/,
  RNF_CNTXT       = RNF_CONTEXT,
  RNF_CNTX        = RNF_CONTEXT,
  
  RNF_KNOB        = RN_FIRST + 18 /*@+enumint@*/,
  RNF_EXEC        = RNF_KNOB,
  
  RNF_SIGNATURE   = RN_FIRST + 19 /*@+enumint@*/,
  RNF_SIG         = RNF_SIGNATURE,
  
  RND_RNPARAMS    = RN_FIRST + 20 /*@+enumint@*/,
  RND_PARAMS      = RND_RNPARAMS,

  RN_RAZE         = RN_FIRST + 21 /*@+enumint@*/,
  RN_LAST         = RN_RAZE,
  
  RN_LEN          = RN_LAST + 1   /*@+enumint@*/,

  RN_BASEALLOC    = RND_ROOT,
  RN_INVALID      = (~0)          /*@+enumint@*/
};

typedef enum rnode_type rn_type_t;

#define IS_RTYPE_BUILDABLE(_rt)    (((_rt) != RN_NONE &&		       \
				     (_rt) >= RN_FIRST && (_rt) <= RN_LAST) && \
				    (rnode_buildtab[_rt].buildfn)	       \
				   )

#define IS_RTYPE_VALID(_rt)        (IS_RTYPE_BUILDABLE(_rt))

#define IS_RTYPE_ATTR_SUBTYPE(_rt) (RNF_EXEC == (_rt) || RNF_KNOB == (_rt)         || \
				    RNF_CONTEXT == (_rt) || RNF_SIGNATURE == (_rt)    \
				   )
#define IS_RTYPE_ATTRIBUTE(_rt)    (RNF_ATTRIBUTE == (_rt) || IS_RTYPE_ATTR_SUBTYPE(_rt))
#define IS_RTYPE_CONTROL(_rt)      (RND_CONTROL == (_rt)    || \
				    RNF_KNOB == (_rt)       || \
				    RNF_SIGNATURE == (_rt)  || \
				    RNF_EXEC == (_rt)          \
				   )
#define IS_RTYPE_CTL(_rt)          (IS_RTYPE_CONTROL(_rt))

#define IS_RTYPE_XATTRIBUTE(_rt)   (RNF_CONTEXT == (_rt))
#define IS_RTYPE_XATTR(_rt)        (IS_RTYPE_XATTRIBUTE(_rt))

/*
 * rnode control commands are associated with parents of control attribute
 * leaf nodes.
 * For example, the fsroot rnode implements the command "unmount"
 */

/*
 * rnode aka "resource manager node"
 */

#define MAX_RN_CMDS (6)
struct rnode {
  rn_type_t            rtype;
  unsigned long        rino; /*not enum/rn_type_t so may be >RN_LAST, esp. RN_GUARD */
  char                *nm;

                     /*builder/constructor of nodes of this type*/
  struct rnode      *(*buildfn)(struct rnode *, struct rnode *);
  
  struct config_param *p_cp;
  int                  gen;
  time_t               ctime;
  uid_t                uid;
  gid_t                gid;
  void                *p_dyntyp;
  int                  h;    /*hash(nm)*/

  /* in-memory lists */
  struct rnode        *nxt_dirty; /* dirty nodes not yet written to BackingStore              */
  struct rnode        *signature; /* this node has a signature, visible through its attribute */
  struct rnode        *prev_version;
  
   /*
    * Attributes for this rnode type are described in <attr_desc.table> and may be iterated over.
    *
    * Some tables describe attributes with potential associations with an rnode_t.
    * In this case, the is_mine() function will claim them.
    * NULL (*is_mine)() is equivalent to an is_mine() always returning TRUE.
    */
  struct attr_desc {
    struct config_param *table;
    tri_t              (*is_mine)(struct config_param *);
  } attr_desc;
  
  /* static attributes for the lifetime of the rnode instantiation */
  struct is {
    unsigned int   dir:1;
    unsigned int   file:1;
    unsigned int   link:1;
  } is;
  
  /* attributes which may change over the lifetime of the rnode */
  struct may_be {
    unsigned int      :0; /*align to word boundary*/
    unsigned int      controllable:1;
    unsigned int      dirty:1;
    unsigned int      readable:1;
    unsigned int      writable:1;
    unsigned int      execable:1;
    unsigned int      :0;
    
    struct rnode     *notify;
  } maybe;

  /* ctl functions replace ioctl()'s, fcntls()'s etc, as in plan9 */
  struct rn_ctl {
    enum rnode_type  rtype; /*rnode_type to be linked to the control function*/
    char            *nm;
    tri_t          (*fn)(struct rnode *, struct config_param *);
  } ctl[MAX_RN_CMDS];

  /* the following effectively constitute a traditional struct dirent list */
  int                  n_children; 
  struct rnode        *children;  /* if any                                          */
  struct rnode        *parent;    /* always exists, fsroot->parent->fsroot           */ 
  struct rnode        *attr;      /* children which are leaf nodes, are attributes   */
  struct rnode        *subdir;    /* children which are not leaf nodes, are sub-dirs */
  
  int                  n_xattr;   /* # extended attributes                           */
  struct config_param *xattr;     /* config param holding nm, value & type, p_nxt->  */
};

typedef struct rnode  rnode_t;
typedef struct rn_ctl rn_ctl_t;

/* ptype to rnode type mapping tables */
struct convtyp {
  enum param_type ptyp;
  rn_type_t       rtyp;
};
typedef struct convtyp pt2rn_t;


/* interfaces for rnode manipulaton */
extern rnode_t *(*get_rn_buildfn(rn_type_t))(rnode_t *, rnode_t *);
extern tri_t    (*get_rn_signfn(rnode_t *))(rnode_t *, config_param_t *);
extern void       link_xattr2rn(rnode_t *, config_param_t *);
extern tri_t      collect_predstate(void);

/* tables to reference associated datums */
extern rnode_t rnode_buildtab[];
extern pt2rn_t p2r_typ_convtab[];
extern pt2rn_t p2r_typ_rm_convtab[];

#endif
