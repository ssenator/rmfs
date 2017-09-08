/*
 * absorbs configuration parameters from multiple sources
 * once a parameter has been found from a high priority source
 * it may not be overwritten from a lower priority source
 *
 * XXXFUTURE: different security models would order param_collection_funcs[]
 *            to fit the appropriate security profile
 *            this is the purpose of the PSRC_NXT() abstraction
 */

#include "rmfs.h"

/*
 * param source collection dispatch table
 *
 * configuration parameters are collected from various sources
 * special types of parameters require their own collectors
 * collector functions embed the specific knowledge of where to obtain
 * the values and to perform type-specific semantic checking
 *
 * configuration parameter collection functions
 *  IN: ptr to config_param_t
 *  OUT: ptr to rmfs_param_t (or NULL)
 * These are called by ingest_config() in priority order
 * XXX functions per semantics of each variable?
 *
 * processing functions for sources of configuration state
 *  these are called as the PSRC_* levels are ingested
 *
 * other security models would/could reorder this table
 * and their associated PSRC bits
 */

param_source_t construct_fuse_mountopts(config_param_t *);
param_source_t collect_fuse_mountopts(config_param_t *);
param_source_t collect_env(config_param_t *);
param_source_t collect_macconf(config_param_t *);
param_source_t collect_slurm(config_param_t *);
param_source_t collect_default(config_param_t *);
param_source_t calcul_derived(config_param_t *);

param_source_t (*param_collection_functab[])(config_param_t *) = { /*XXXswitchfunc*/
  collect_macconf,          /* PSRC_MAC_CONF=1 PSRC_HI */
  collect_slurm,            /* PSRC_SLURM=2            */
  collect_env,              /* PSRC_ENVAR=3            */
  construct_fuse_mountopts, /* PSRC_MNT_OPT=4          */
  collect_fuse_mountopts,   /* PSRC_MNT_NONOPT=5       */
  NULL,                     /* PSRC_FUSE=6             */
  NULL,                     /* PSRC_USERINPUT=7        */
  collect_default,          /* PSRC_DEFAULT=8           */
  calcul_derived,           /* PSRC_DERIVED=9, PSRC_LO  */
  NULL                      /* PSRC_N */
};

/*keep aligned with param_collection_functab and param_source_t*/
char *src2str[] = {
  "<0 none>",
  "<1 macconf>",
  "<2 slurm>",
  "<3 envar>",
  "<4 mntopt>",
  "<5 mntnonopt>",
  "<6 fuse>",
  "<7 userinput>",
  "<8 default>",
  "<9 derived>",
  NULL
};




/*
 * verify that all necessary and sufficient parameters have been provided
 */
int
config_consistent_complete() {
  config_param_t        *p_cp;
  slurm_fuse_opt_desc_t *p_fop;
  config_param_t        *p_cp_Debug;
  tri_t                  debug = FALSE;
  
  int exclusive          = 0;
  int missing_dependency = 0;
  int missing_mandatory  = 0;
  int only_if_debug      = 0;

  p_cp_Debug = getconfig_fromnm("Debug");
  if (!p_cp_Debug) {
    return FALSE;
  }
  if (p_cp_Debug->val.ue.truth) {
    debug = TRUE;
  }

  for (p_fop = slurm_fopts; p_fop->nm; p_fop++) {
    if (!p_fop->p_cp) {
      continue;
    }
    p_cp = p_fop->p_cp;
    if ((p_fop->oflg & OPT_MANDATORY) && NOBITS(p_cp->src.actual)) {
      missing_mandatory++;
    }
    if ((p_fop->oflg & OPT_VALID_IF_DEBUG) && (!debug && p_cp->src.debug)) {
      only_if_debug++;
    }
    if ((p_fop->oflg & OPT_EXCLUSIVE) && ANY(p_cp->src.actual)) {
      exclusive++;
    }
    /*
     * need to check for the following only *after* fuse_opt_parse()
     * since PSRC_MNT_OPT at this point just implies that it might be
     * specified by mount options, not that it actually has been yet.
     *
     * this check is now in parse_an_arg()
     *
     * if ((p_fop->oflg & OPT_SELECT_ONE) && ANY(p_cp->src.actual)) {
     *  select_only_one++;
     * }
    */
  }
  
  for (p_cp = &slurmfs_config[0]; p_cp->nm; p_cp++) {
    if (ANY(p_cp->src.actual) && p_cp->depends_on.nm) {
      if (!p_cp->depends_on.p_cp) {
	missing_dependency++;
      }
    }
  }

  if (missing_dependency > 0 || missing_mandatory > 0 || only_if_debug > 0 ||
      exclusive > 1
     ) {
    return FALSE;
  }
  return TRUE;
}


void
ingest_config(int pass_no)
{
  config_param_t         *p_cp, *p_cp_Debug;
  param_source_t          rc_src;
  tri_t                   debug;
  extern param_source_t   collect_cp(config_param_t *); /*collect_cp.c*/

#ifdef XXX_BackingStore_holds_previous_state
  p_BackingStore_cp = getconfig_fromnm("BackingStore");
  if (ck_prevMountState(p_BackingStore_cp) == TRUE) {
    recover_prevMountState(p_BackingStore_cp);
  }
#else
  if (pass_no == PREV_GENERATION) {
    return;
  }
#endif

  for (p_cp = &slurmfs_config[0]; p_cp->nm; p_cp++) {
    rc_src = collect_cp(p_cp);
  }

  p_cp_Debug = getconfig_fromnm("Debug");
  if (!p_cp_Debug) {
    ErrExit(ErrExit_INTERNAL, "ingest_config: cannot find Debug p_cp");
  }
  rc_src = collect_cp(p_cp_Debug);

  if (NOBITS(rc_src)) {
    ErrExit(ErrExit_INTERNAL, "ingest_config: cannot determine \"Debug\" mode");
  }
  debug = p_cp_Debug->val.ue.truth;
  if (Debug() && pass_no == 0) {
    ingest_config(pass_no+1);
  }
  
  if (!config_consistent_complete()) {
    Usage (ErrExit_INCOMPLETE, "configuration is inconsistent or incomplete");
  }
  return;
}

