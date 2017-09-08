
#include <sys/param.h>

#ifndef RMFS_PARAM_H_
#define RMFS_PARAM_H_

#include "rnode.h"


/*
 * control parameters for rnode allocation
 *
 * Note that this is a deliberately simple allocation strategy designed
 * specifically for read-mostly access in a fuse implementation.
 *   - nodes are allocated when the file tree is constructed, based on
 *     external resource manager state; the external resource manager
 *     is authoritative for node state. The file system is a view into
 *     this resource manager state.
 *
 *   - the file system tree and its nodes are constant, once created
 *
 *   - only a limited set of nodes are modifiable, usually via a request
 *     to a "control" leaf node, which modifies state within the node, only.
 *     Specifically, no additional nodes are created or destroyed.
 *
 *   - this prototype's deliberate mechanism to reap memory is to unmount
 *     the whole file system
 *     XXXFUTURE: to build simply off of this code, a future implementation
 *     XXXFUTURE: could effectively do a remount, retrieving the previous
 *     XXXFUTURE: cached state from BackingStore=...
 *     XXXFUTURE: that is, use a working set/consistent set model with the
 *     XXXFUTURE: previous instance as the consistent set.
 */

#define DEFMODE_FILE    0
#define DEFMODE_DIR     1
#define N_DEFAULT_MODES 2

#define RM_UID_CTL      0 /*aka "slurmctld"*/
#define RM_UID_DISPATCH 1 /*aka "slurmstepd" or "slurmd"*/
#define RM_UIDS         2
/*
 * individual rnodes may have a uid of the user, if it is a RND_JOBID,
 * or related node, such as a jobid's attributes
 */


#define RNODEPOOL_DEFAULT_MINSIZE   256 /*XXX needs heuristic, esp. large clusters*/

/*
 * rn_paramtab may be viewed as an in-memory superblock
 * XXX locking is in need of serious work
 * XXX
 * XXX current strategy, since this is a FUSE implementation,  is to
 * XXX rely on the fact that FUSE is single-threaded until fuse_main()
 * XXX and that node linkage and state is immutable after fuse_main()
 * XXX
 * XXX with the only exceptions of: p_dirty, last_update
 */
struct rnode_params {
  unsigned long  rn_minpoolsize;

  unsigned long *rn_allocmap;
  rnode_t       *rn_pool;
  
  unsigned long  rn_maxalloc;
  unsigned long  rn_basealloc;
  unsigned long  rn_curalloc;

  rnode_t       *p_nxt_avail_rn;
  unsigned long  nxt_avail;

  uid_t               rm_uid[RM_UIDS];   /* from resource mgr */
  mode_t              def_mode[N_DEFAULT_MODES];
  security_context_t *scon;

  /* bookmarks */
  rnode_t       *p_fsroot;
  rnode_t       *p_cname;
  rnode_t       *p_jobd;
  
  rnode_t       *p_dirty;  /* list head */
  time_t         last_update;

  unsigned long  fsid;
  unsigned long  version;
};

typedef struct rnode_params rn_param_t;

extern rn_param_t rn_paramtab;

/*
 * paramtab accessor functions
 */

/*these require the caller to do explicit locking*/
extern void           rn_paramtab_release(void);
extern rn_param_t    *get_rn_params(int);

/*
 * the following will automatically lock & release,
 * as well as return a specific type
 */
extern rnode_t       *set_rnparam_rn(off_t, rnode_t *);
extern void           set_rnparam_time(off_t, time_t);
extern void           set_rnparam_ul(off_t, unsigned long);

extern void           set_rnparam_fsroot(rnode_t *);
extern void           set_rnparam_cname(rnode_t *);
extern void           set_rnparam_jobd(rnode_t *);
extern bool_t         rn_param_adddirty(rnode_t *);

extern mode_t        *get_rnparam_modep(void);
extern rnode_t       *get_rnparam_fsroot(void);

extern rnode_t       *get_rnparam_rn(off_t);
extern unsigned long  get_rnparam_ul(off_t);
extern unsigned long *get_rnparam_ulp(off_t);
extern mode_t        *get_rnparam_modep_offset(off_t);
extern uid_t         *get_rnparam_uidp(off_t);
extern unsigned long  get_rnparam_fsid(void);
extern unsigned long  get_rnparam_version(void);


/*
**  
#define RNPARAM_MINPOOLSIZE_OFF (offsetof(rn_param_t, rn_minpoolsize))
#define RNPARAM_MINPOOLSIZE     (get_rnparam_ul(RNPARAM_MINPOOLSIZE_OFF))

#define RNPARAM_ALLOCMAP_OFF    (offsetof(rn_param_t, allocmap))
#define RNPARAM_ALLOCMAP        (get_rnparam_ulp(RNPARAM_ALLOCMAP_OFF))

#define RNPARAM_POOL_OFF        (offsetof(rn_param_t, rn_pool))
#define RNPARAM_POOL            (get_rnparam_rn(RNPARAM_POOL_OFF))

#define RNPARAM_MAXALLOC_OFF    (offsetof(rn_param_t, rn_maxalloc))
#define RNPARAM_MAXALLOC        (get_rnparam_ulp(RNPARAM_MAXALLOC_OFF))

#define RNPARAM_BASEALLOC_OFF    (offsetof(rn_param_t, rn_basealloc))
#define RNPARAM_BASEALLOC        (get_rnparam_ulp(RNPARAM_BASEALLOC_OFF))

#define RNPARAM_CURALLOC_OFF    (offsetof(rn_param_t, rn_curalloc))
#define RNPARAM_CURALLOC        (get_rnparam_ulp(RNPARAM_CURALLOC_OFF))

#define RNPARAM_NXTAVAILRN_OFF  (offsetof(rn_param_t, p_nxt_avail_rn))
#define RNPARAM_NXTAVAILRN      (get_rnparam_rn(RNPARAM_NXTAVAILRN_OFF))

#define RNPARAM_NXTAVAIL_OFF    (offsetof(rn_param_t, nxt_avail))
#define RNPARAM_NXTAVAIL        (get_rnparam_ul(RNPARAM_NXTAVAIL_OFF))

#define RNPARAM_RMUID_OFF       (offsetof(rn_param_t, rm_uid))
#define RNPARAM_RMUID           (get_rnparam_uidp(RNPARAM_RMUID_OFF))

#define RNPARAM_DEFMODE_OFF     (offsetof(rn_param_t, def_mode))
#define RNPARAM_DEFMODE         (get_rnparam_modep(RNPARAM_DEFMODE_OFF))

#define RNPARAM_JOBD_OFF        (offsetof(rn_param_t, p_jobd))
#define RNPARAM_JOBD            (get_rnparam_rn(RNPARAM_JOBD_OFF))

#define RNPARAM_DIRTY_OFF       (offsetof(rn_param_t, p_dirty))
#define RNPARAM_DIRTY           (get_rnparam_rn(RNPARAM_DIRTY_OFF))

#define RNPARAM_LASTUPDATE_OFF  (offsetof(rn_param_t, last_update))
#define RNPARAM_LASTUPDATE      (get_rnparam_time(RNPARAM_LASTUPDATE_OFF))

#define RNPARAM_FSID_OFF        (offsetof(rn_param_t, fsid))
#define RNPARAM_FSID            (get_rnparam_ulp(RNPARAM_FSID_OFF))
**
*/

extern config_param_t partinfodesc_tab[];
extern config_param_t jobinfodesc_tab[];
extern config_param_t stepinfodesc_tab[];
extern config_param_t nodeinfodesc_tab[];

#endif
