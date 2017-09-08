
#include "rmfs.h"

config_param_t *
getconfig_fromnm(char *nm) {
  config_param_t *p_cp = NULL;
  int             h;
  
  if (!nm) {
    ErrExit(ErrExit_ASSERT, "getconfig_fromnm: !nm ");
    return NULL;
  }
  h = djb_strtohash(nm);

  if (IS_VALID_HASH(h)) {
    p_cp = getconfig_fromhash(h);
  }
  if (!derefable_cp(p_cp)) {
    p_cp = getconfig_fromnm_nohash(nm);
  }
  return derefable_cp(p_cp)? p_cp: NULL;
}

/*
 * getconfig_fromnm_nohash()
 *  should only be used when it is too early to use hash comparisons
 */
config_param_t *
getconfig_fromnm_nohash(char *nm) {
  config_param_t *p_cp;

  if (!nm) {
    ErrExit(ErrExit_ASSERT, "getconfig_fromnm_nohash: !nm");
    return NULL;
  }
  
  for (p_cp = slurmfs_config; p_cp->nm; p_cp++) {
    if (strcmp(p_cp->nm, nm) == 0) {
      init_hash_cp(p_cp); /* to calculate and store the hash */
      return p_cp;
    }
  }
  return NULL;
}

config_param_t *
getconfig_fromhash(unsigned long h) {
  config_param_t *p_cp;

  if (IS_INVALID_HASH(h)) {
    ErrExit(ErrExit_ASSERT, "getconfig_fromhash: invalid hash");
    return NULL;
  }

  /* XXX FIXME: need to use hashes intelligently, with buckets */
  for (p_cp = slurmfs_config; p_cp->nm; p_cp++) {
    if (h == p_cp->h) {
      return p_cp;
    }
  }
  return NULL;
}
