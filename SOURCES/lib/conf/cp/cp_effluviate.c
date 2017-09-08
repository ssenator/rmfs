#include "rmfs.h"

void
effluviate_one_cp(config_param_t *p_cp) {

  if (!p_cp) {
    return;
  }
  fprintf(stderr, "\n\tp_cp: ptyp:\"%s\" (%d) nm:\"%s\"\n",
	  ptyp2pname_tab[p_cp->typ], /* ptyp2pname_tab[] in rmfs_rnode.c */
	  p_cp->typ, p_cp->nm? p_cp->nm: "<null>");

  if (IS_ALPHA_TYPE(p_cp->typ)) { 
    fprintf(stderr, "\tp_cp: val.str=\"%s\" size=%ld\n",
	    p_cp->val.ue.str? p_cp->val.ue.str: "<null>",
	    p_cp->val.size);

  } else if (IS_NUMERIC_TYPE(p_cp->typ)) {
    fprintf(stderr, "\tp_cp: val.ue.l:%ld val.ue.ul:%ld val.size:%ld\n",
	    p_cp->val.ue.l,
	    p_cp->val.ue.ul,
	    p_cp->val.size);

  } else if (IS_SLURM_TYPE(p_cp->typ)) {
    fprintf(stderr, "\tp_cp: is a slurm type: %ld,  size: %ld\n",
	    p_cp->val.ue.ul, p_cp->val.size);
  }

  fprintf(stderr, "\tsrc.actual: %s (%d)\n\tsrc.allowed: %s (%d)\n\tsrc.debug: %s (%d)\n",
	   PSRCNAME(p_cp->src.actual),  p_cp->src.actual,
	   PSRCNAME(p_cp->src.allowed), p_cp->src.allowed,
	   PSRCNAME(p_cp->src.debug),   p_cp->src.debug
	  );

}

void
effluviate_config() {
  config_param_t *p_cp;
  int             i;

  fprintf(stderr, "\nslurmfs_config:\n");
  for (i = 0, p_cp = slurmfs_config; p_cp->nm; i++, p_cp++) {
    effluviate_one_cp(p_cp);
  }
  fprintf(stderr, "slurmfs_config: len=%d\n", i-1);
}
