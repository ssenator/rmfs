#include "rmfs.h"


/*
 * attr_cnt_cp()
 *  returns # attributes of the rnode of type rtype
 */
int
attr_cnt_cp(rn_type_t rtype) {
  config_param_t *p_cp;
  char           *nm;
  int             hash, n_attr;
  tri_t         (*is_mine)(config_param_t *);

  p_cp = NULL;

  if (!IS_RTYPE_VALID(rtype)) {
    ErrExit(ErrExit_INTERNAL, "attr_cnt_cp: invalid rtype");
  }

  nm = rnode_buildtab[rtype].nm;
  if (!nm) {
    ErrExit(ErrExit_INTERNAL, "attr_cnt_cp: !nm for rtype");
  }
  hash = djb_strtohash(nm);
  if (!IS_VALID_HASH(hash)) {
    ErrExit(ErrExit_INTERNAL, "attr_cnt_cp: invalid hash for rtype nm");
  }
  is_mine = rnode_buildtab[rtype].attr_desc.is_mine;
  n_attr  = 0;
  for (p_cp = rnode_buildtab[rtype].attr_desc.table; p_cp->nm; p_cp++){

    if (!IS_VALID_HASH(p_cp->h)) {
      init_hash_cp(p_cp);
    }
    
    if (!is_mine || (*is_mine)(p_cp)) {
      n_attr++;
    }
  }
  if (n_attr <= 0) {
    ErrExit(ErrExit_INTERNAL, "attr_cnt_cp: n_attr <= 0");
  }
  return n_attr;
}


void
link_xattr2rn (rnode_t *p_rn, config_param_t *p_cp) {
  
  if (!p_rn) {
    ErrExit(ErrExit_INTERNAL, "link_xattr2rn: !p_rn");
  }
  if (!p_cp) {
    ErrExit(ErrExit_INTERNAL, "link_xattr2rn: !p_cp");
  }
  
  /*XXX acquire (upgrade to) rnode write lock*/
  if (p_cp->p_nxt) {
    ErrExit(ErrExit_INTERNAL, "link_xattr2rn: p_cp->p_nxt already set");
  }
  if (p_rn->xattr) {
    p_cp->p_nxt = p_rn->xattr;
  }
  p_rn->xattr = p_cp;
  p_rn->n_xattr += 1;
  /*XXX release rnode lock */

  return;
}
