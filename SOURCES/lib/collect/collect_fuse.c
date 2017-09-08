/*
 * fuse_opt_parse() calls parse_an_arg() for each arg encountered in a separate pass
 *  xref: fuse.sourceforge.net/doxygen/fuse__opt_8h.html
 *  the "key" arg is the offset that was used to process the template & store the arg value
 *
 * parse_an_arg() => collect_config_params(PSRC_MNT_NONOPT)
 */

#include "rmfs.h"

/*
 *
 * From fuse_opt.h:
 *   Option description
 *   This structure describes a single option, and and action associated
 *   with it, in case it matches.
 *
 *   More than one such match may occur, in which case the action for
 *   each match is executed.
 *
 * There are three possible actions in case of a match:
 *   i) An integer (int or unsigned) variable determined by 'offset' is
 *    set to 'value'
 *   ii) The processing function is called, with 'value' as the key
 *   iii) An integer (any) or string (char *) variable determined by
 *    'offset' is set to the value of an option parameter
 *
 * 'offset' should normally be either set to
 *  1- FUSE_OPT_KEY: 'offsetof(struct foo, member)'  actions i) and iii)
 *  2- FUSE_OPT_KEY:  -1			            action ii)
 *  -------
 *
 * Most of our argument processing, the template/offset: i) and iii) is called.
 * For the FUSE_OPT_NON_OPT, parse_an_arg() will be called
 *
 * 
 *
 */

static int select_only_one = 0;

int
parse_an_arg(void *data, const char *arg, int fusekey, struct fuse_args *outarg) {
  
  config_param_t        *p_fuse_cp = NULL;
  config_param_t        *p_cp;
  slurm_fuse_opt_desc_t *p_fopd;
  rmfs_param_t           new_val;
  tri_t                  tc;
  int                    len;

  p_fopd = (slurm_fuse_opt_desc_t *) data;

  if (fusekey == FUSE_OPT_KEY_NONOPT) {
    if (!(p_fopd->oflg | OPT_NONOPT)) {
      ErrExit(ErrExit_ASSERT, "parse_an_arg: fusekey==FUSE_OPT_KEY_NONOPT, but p_fopd->oflg !OPT_NONOPT");
      return 0;
    }
    p_cp = getconfig_fromnm(p_fopd->nm);
    if (!p_cp) {
      ErrExit(ErrExit_ASSERT, "parse_an_arg: NONOPT: !p_cp ");
      return 0;
    }

    len = internal_strlen((char *) arg);
    if (len >= _POSIX_PATH_MAX) {
      ErrExit(ErrExit_ENOMEM, "parse_an_arg: arg is >= _POSIX_PATH_MAX ");
      return 0;
    }
    new_val.ue.pathnm = (char *) arg;
    new_val.size = len + 1; /*strlen() + terminator '\0'*/ 
    
    tc = typ_check(p_cp->typ, &new_val);
    if (tc == TRUE) {
      p_cp->val.size      = new_val.size;
      p_cp->val.ue.pathnm = strndup(arg, new_val.size);
      if (!p_cp->val.ue.pathnm) {
	ErrExit(ErrExit_ENOMEM, "parse_an_arg: NONOPT strdup returned NULL");
      }
    }

  } else if (fusekey == FUSE_OPT_KEY_OPT) {

    if (!(p_fopd->oflg | OPT_SELECT_ONE)) {
      select_only_one++;
    }
    if (!arg) {
      return 0;
    }
    if (!(p_fuse_cp = getconfig_fromnm((char *) arg))) {
      return 0;
    }
    new_val.ue.ptr = (void *) arg;
    new_val.size   = p_fuse_cp->val.size;
    tc = typ_check(p_fuse_cp->typ, &new_val);
    if (tc == TRUE)  {
      set_val_ptr(p_fuse_cp, new_val.ue.ptr);
    }
  } else {
    ErrExit(ErrExit_INTERNAL, "parse_an_arg: unknown key?");
  }
  if (TRUE == tc) {
    p_cp->src.actual = BIT(PSRC_FUSE);
  }
  if (select_only_one > 1) {
    ErrExit(ErrExit_CONFIG, "parse_an_arg: too many SELECT_ONE options specified");
  }
  return 1;
}


slurm_fuse_opt_desc_t *
getfuseoptdesc_fromconfig(config_param_t *p_cp) {
  slurm_fuse_opt_desc_t *p_fu_opd = NULL; /*fuse_linkage.h*/

  if (p_cp->p_fopd) { /* previously cached */
    return p_cp->p_fopd;
  }
  if (IS_INVALID_HASH(p_cp->h)) {
    init_hash_cp(p_cp);
  }
  for (p_fu_opd = slurm_fopts; p_fu_opd->nm; p_fu_opd++) {
    if (p_cp->h == djb_strtohash(p_fu_opd->nm)) {
     return p_fu_opd;
   }
  }
  return NULL;
}


/*
 * mount options are handled by the fuse option parsing functions
 * so this function constructs the fuse option table from the slurm configuration table
 *
 * - initialize the fuse option linkage table from the configuration parameters
 * - call fuse_opt_parse() which will call back to parse_an_arg() for each option matched
 *
 * called from ingest_config(PSRC_MNT_NONOPT) and ingest_config(PSRC_MNT_OPT)
 */
param_source_t
construct_fuse_mountopts(config_param_t *p_cp) {
  int i = 0, n = 0, new_size = 0;
  enum { NOTHING = 0, TERMINATOR = 1, OPT = 2, NONOPT = 4 } to_do;
  param_source_t rc = PSRC_NONE;

  /* fuse_linkage.h */  
  extern int                   fuseopts_len;
  extern struct fuse_opt      *p_fuseopts_tbl;
  extern slurm_fuse_opt_desc_t slurm_fopts[];
  
  struct fuse_opt        *p_fo       = NULL;
  struct fuse_opt        *p_f1       = NULL;
  slurm_fuse_opt_desc_t  *p_fopd     = NULL;

  if (!(p_fopd = getfuseoptdesc_fromconfig(p_cp))) {
    /*
     * a fuse mount option cannot be found to match this config parameter
     * this is OK, provided that slurmfs_config doesn't require fuse to provide this
     */
    return rc;
  }

  /*
   * link so that the slurmfs configuration parameter entry (p_cp)
   * points to the slurmfs fuse opt table (slurm_fopts) entry
   *  p_cp->p_slfsopt = p_sflop;
   *
   * and the slurm_fuse_opt table entry (psflop) entry
   * points to the slurmfs configuration parameter entry
   *  p_sflop->p_cp = p_cp;
   */
  
  p_cp->p_fopd = p_fopd;    /* this config parameter matches this SlurmFS opt */
  p_fopd->p_cp = p_cp;      /* this SlurmFS opt matches this config parameter */

  /*
   * to adhere to the slurmfs configuration calling protocol, this routine is called
   * multiple times, once per each configuration parameter
   *
   * but, because fuse expects only a single call with a single fuse_opt key table,
   * we use the multiple calls to construct the fuse_opt key table
   *
   * then, when complete*, call into fuse to parse the actual mount options,
   * which will call back into parse_an_arg(). It will match the struct fuse_opt
   * entry, which was allocated as the individual options were constructed. 
   *
   * *complete is keyed off of the OPT_NONOPT flag, which is processed after all other options
   */
  i = fuseopts_len;
  to_do = NOTHING;
  
  /* no parameter name */
  if (!p_fopd->nm) {
    to_do = TERMINATOR;
    n = 1;

  /* "non-option" ex. "MountPoint" */
  } else if (p_fopd->oflg & OPT_NONOPT) {
    to_do = NONOPT; n = 1;
    
  /* shortname and longname templates exist */
  } else if (p_fopd->_s && p_fopd->_long) {
    to_do = OPT;
    n = 2;
    
  } else {
    /*UNREACHED*/
    ErrExit(ErrExit_INTERNAL, "construct_fuse_mountops: broken parameter table");
  }
  new_size = fuseopts_len + n;

  if (new_size <= 0) {
    ErrExit(ErrExit_ASSERT, "construct_fuse_mountopts: new_size <= 0");
    return PSRC_NONE;
  }

  if (!p_fuseopts_tbl) {
    /* options can be from either fuse args or configuration parameters, use the sum */
    p_fuseopts_tbl = (struct fuse_opt *) calloc(get_fo_max_len(), sizeof(struct fuse_opt));
    if (!p_fuseopts_tbl) {
      ErrExit(ErrExit_TEMPFAIL, "calloc(fuseopts_tbl) returned NULL");
    }
  }
  p_fo = &p_fuseopts_tbl[n];  /*this record*/
  p_f1 = p_fo + 1;            /*the next record*/
  fuseopts_len = new_size;

  p_cp->p_fo = p_fo; /* this config parameter matches this FUSE opt table entry*/

  switch (to_do) {
  case TERMINATOR:
    *p_fo = (struct fuse_opt) FUSE_OPT_END; /* .templ = NULL */
     rc  = PSRC_MNT_NONOPT;
    break;
    
  case NONOPT:
   /*
    * p_fo=NONOPT because this is explicitly not an option,
    * it is the terminator of options. In this implementation this is
    * getconfig_fromnm("MountPoint")
    */
    /*NOTREACHED see above*/
    p_fo->templ  = p_cp->nm;
    p_fo->offset = -1U;
    p_fo->value  = /*key*/ (int) djb_strtohash(p_cp->nm);
    rc           = PSRC_MNT_NONOPT;
    break;
    
  case OPT:
    /*
     * These construct unique values that will be returned by the
     * fuse mountopt protocol
     * as the options are parsed
     * See parse_an_arg(), FUSE_OPT_KEY(templ, key) { templ, -1U, key } (fuse_opt.h)
     */

    /* short option string, ex. "-s" */
    p_fo->templ  = p_fopd->_s;
    p_fo->offset = -1U;
    p_fo->value  = /*key*/ (int) djb_strtohash(p_cp->nm);

    p_f1 = (p_fo + 1);
    /* long option string, ex. "--longparameter" */
    p_f1->templ  = p_fopd->_long;
    p_f1->offset = -1U;
    p_f1->value  = /*key*/ (int) djb_strtohash(p_cp->nm);
    rc           = PSRC_MNT_OPT;
    break;
    
  default:
    break;
  }
  return rc;
}

/*
 * collect_fuse_mountopts() 
 * - called from ingest_config(PSRC_MNT_NONOPT)
 * - should be called only once, when processing the NONOPT after all
 *   other MNT options have been constructed
 */
static int term_count = 0;

param_source_t
collect_fuse_mountopts(config_param_t *p_cp) {
  param_source_t rc = PSRC_NONE;
  extern struct fuse_args f_args;

  if (term_count++ > 1) {
    ErrExit(ErrExit_INTERNAL, "collect_fuse_mountopts: (re)terminate?");
  }
  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "collect_fuse_mountopts: !p_cp");
    return rc;
  }
  
  /* add the NONOPT entry into the p_fuseopts_tbl */
  if ((rc = construct_fuse_mountopts(p_cp))) {

    if (p_cp != slurm_fopts[0].p_cp) {
      ErrExit(ErrExit_INTERNAL, "collect_fuse_mountopts: NONOPT p_cp linkage broken");
    }
    
    /* and have fuse crack the arg nut */
    if (fuse_opt_parse(&f_args, &slurm_fopts[0], p_fuseopts_tbl, &parse_an_arg)) {
      Usage (ErrExit_ARGPARSE, NULL);
    }
    rc = PSRC_MNT_NONOPT;
  }
  return rc;
}
