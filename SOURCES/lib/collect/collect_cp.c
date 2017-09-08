
#include "rmfs.h"


param_source_t
collect_cp(config_param_t *p_cp) {
  config_param_t *p_cp_depends, *p_cp_di, *p_cp_Debug;
  param_source_t  dep_psrc, src, s;
  tri_t           debug;

  extern config_param_t *init_dependency_cp(config_param_t *); /*conf/config_param/cp_subr.c*/

  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "collect_cp: !p_cp");
    return PSRC_NONE;
  }
  init_hash_cp(p_cp);
  init_dependency_cp(p_cp);
  
  /*
   * this p_cp is dependent upon some other, which may itself have dependencies
   */
  
  if (p_cp->depends_on.nm) {
    if (!(p_cp_depends = init_dependency_cp(p_cp))) {
      ErrExit(ErrExit_ASSERT, "collect_cp: !p_cp_depends");
    }

    for (p_cp_di = p_cp->depends_on.p_cp;
	     p_cp_di /*XXX && !dependency_loop(p_cp) XXX*/;
         	 p_cp_di = p_cp_di->depends_on.p_cp) {
      
      dep_psrc = collect_cp(p_cp_di);
      
      if (NOBITS(dep_psrc)) {
	ErrExit(ErrExit_INTERNAL, "collect_cp: dependent p_cp collection failed");
      }
    }
  }

  /*
   * if this is the 1st collection of "Debug" it won't be set yet, only on the 2nd pass
   */
  
  p_cp_Debug = getconfig_fromnm("Debug");
  debug = !p_cp_Debug? FALSE: p_cp_Debug->val.ue.truth;
  
  /*
   * Parameters may be obtained from multiple sources, ordered by priority:
   * 1. mount options, which are parsed by fuse (PSRC_MNT_OPT|PSRC_MNT_NONOPT)
   * 2. run-time environment variables (PSRC_ENV)
   * 3. slurm configuration, which we collect from the slurm config api (PSRC_SLURM)
   * future: 4. slurmfs configuration, which we collect from $SLURMFS_CONF
   *    XXX:    but not until the slurm API & slurm.conf has an 'Include module' feature
   * 5. selinux-slurmfs configuration, which we collect from $SLURM_MAC_CONF (PSRC_MAC)
   * 6. calculated, based on the combination of configuration options
   *
   */
  
   /*
    * walk through collection agencies, in the order that we trust,
    * calling collector funcs (see rmfs_types.h for macros)
    */
  
  for (s = PSRC_MOST_TRUSTED, src = p_cp->src.actual;
           PSRC_TEST(s, PSRC_LEAST_TRUSTED) && src == PSRC_NONE;
               s = PSRC_NXT(s)) {
    /*
     * allowed from this source in this mode?
     *  ...and isn't already collected from a more trusted source?
     */
    if (ISSET(p_cp->src.allowed, s) ||
	(debug == TRUE && ISSET(p_cp->src.debug, s))
       ) {
	  
      if (NOBITS(p_cp->src.actual)) {
	if (s == PSRC_NONE) {
	  ErrExit(ErrExit_ASSERT, "collect_cp: s=PSRC_NONE");
	  
	} else {
	  if (param_collection_functab[s-1]) {
	    src = (param_collection_functab[s-1])(p_cp);
	    if (src != PSRC_NONE) {
	      p_cp->src.actual = BIT(src);
	    }
	  }
	}
      }
    }
  }

  p_cp_Debug = getconfig_fromnm("Debug");
  if (!p_cp_Debug) {
    ErrExit(ErrExit_INTERNAL, "collect_cp: cannot find Debug p_cp");
    
  } else {
    if (p_cp_Debug->h == p_cp->h && p_cp_Debug->val.ue.truth) {
      if (p_cp->depends_on.nm) {
	ErrExit(ErrExit_INTERNAL, "collect_cp: Debug is not at base of dependency chain");
      }
    }
  }

  return p_cp->src.actual;
}
