
#include "rmfs.h"

/*
 * a context is an opaque string, which may be collected from fuse mount options
 * or from the MAC configuration file
 *
 * XXXFUTURE: call out to an implementatin specific function which is aware
 * XXXFUTURE: of the contents of the opaque context
 */

param_source_t
collect_macconf(config_param_t *p_cp) {
  config_param_t *p_cp_macconf;
  struct stat    st;
  int            macconf_fd;
  void          *macconf;
  char          *pnam, *pval;

  p_cp_macconf = getconfig_fromnm("SLURM_MAC_CONF");
  if (!p_cp_macconf) {
    ErrExit(ErrExit_ASSERT, "collect_macconf: !p_cp_macconf");
    goto out;
  }

  /*
   * this is not an error in this version,
   * simply an indication that the source does not exist in this installation
   */
  if (-1 == stat(p_cp_macconf->nm, &st)) {
    goto out;
  }
  if (-1 == (macconf_fd = open(p_cp_macconf->nm, O_RDONLY, 0))) {
    goto out;
  }
  if (MAP_FAILED == (macconf = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, macconf_fd, 0))) {
    goto close_out;
  }
  
  /*
   * MACCONF file is simple: no comments, just parameters
   * XXX use some well-tested param=value parser library
   */
  if ((pnam = strstr(macconf, p_cp->nm))) { /* "PARAM" */
    if ((pval = strtok(pnam, "="))) {       /* "PARAM=" */
      pval = strtok(NULL, "\n\0");
      set_val_ptr(p_cp, pval);
      close(macconf_fd);
      return PSRC_MAC_CONF;
    }
  }
  
 close_out:  
  close(macconf_fd);
 out:
  return PSRC_NONE;
}

param_source_t
collectmac_context(config_param_t *p_cp) {

  extern tri_t typ_context(rmfs_param_t *); /*conf/typ/typ_context.c*/

  if (!p_cp) {
    return PSRC_NONE;
  }
  if (ANY(p_cp->src.actual)) { /* graceful no-op, value already collected */
    return p_cp->src.actual;
  }
  if (TRUE != typ_context(&p_cp->val)) {
    return PSRC_NONE;
  }
  return collect_macconf(p_cp);
}

