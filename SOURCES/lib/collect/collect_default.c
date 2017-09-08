
#include "rmfs.h"



param_source_t
collect_default(config_param_t *p_cp) {

    if (ANY(p_cp->src.actual)) {
      return p_cp->src.actual;
    }

    if (!ISSET(p_cp->src.allowed, PSRC_DEFAULT)) {
      return PSRC_NONE;
    }
    if (IS_CONTEXT_TYPE(p_cp->typ)) {
      return PSRC_NONE;
    }

    /*
     * order by most specialized to most general
     * XXXFUTURE call per-resource manager function
     * XXXFUTURE ie. set_slurm_default() or set_context_default()
     * XXX should there be a hierarchy? ie. slurm numeric types call a stack
     * XXXFUTURE set_val() -> switch function so IS_*_TYPE() checks are private
     */

    if (IS_SLURM_TYPE(p_cp->typ)) {
      set_val_ptr(p_cp, p_cp->default_val.ue.ptr); /*XXX set_val_rm()*/
      
    } else if (IS_NUMSIGNED_TYPE(p_cp->typ)) {
      set_val_l(p_cp, p_cp->default_val.ue.l);

    } else if (IS_TRUTH_TYPE(p_cp->typ)) {
      set_val_truth(p_cp, p_cp->default_val.ue.truth);
      
    } else if (IS_NUMERIC_TYPE(p_cp->typ)) {
      set_val_ul(p_cp, p_cp->default_val.ue.ul);
	
    } else if (IS_FSVIS_TYPE(p_cp->typ)) {
      
      if (!p_cp->depends_on.nm || !p_cp->depends_on.p_cp ||
	  !IS_FSVIS_TYPE(p_cp->depends_on.p_cp->typ)) {
	set_val_charptr(p_cp, p_cp->default_val.ue.pathnm);
	
      } else {
	char *pn;
	if (!(pn=calloc(_POSIX_PATH_MAX, sizeof(char)))) {
	  ErrExit(ErrExit_NOMEM, "collect_default: calloc(pn)");
	}
	/* XXX dive down the whole depends_on chain */
	snprintf(pn, _POSIX_PATH_MAX, "%s/%s", 
		 p_cp->depends_on.p_cp->val.ue.pathnm, p_cp->default_val.ue.pathnm);
	set_val_charptr(p_cp, pn); /* XXX set_val_pathnm() */
      }

    } else if (IS_HOST_TYPE(p_cp->typ)) {
      set_val_charptr(p_cp, p_cp->default_val.ue.hostname);
      
    } else if (IS_ALPHA_TYPE(p_cp->typ) && p_cp->default_val.size != 0) {
      set_val_charptr(p_cp, p_cp->default_val.ue.str);

    } else {
      set_val_ptr(p_cp, p_cp->default_val.ue.str);
    }
    return PSRC_DEFAULT;
}
