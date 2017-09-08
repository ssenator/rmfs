
#include "rmfs.h"

/*
 * config_param_t manipulators
 */

void
init_hash_cp (config_param_t *p_cp) {

  if (IS_VALID_HASH(p_cp->h)) {
    return;
  }
  
  p_cp->h = djb_strtohash(p_cp->nm);
    
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_INTERNAL, "init_hash_cp: !derefable_cp");
  }

  return;
}


/*
 * is this p_cp a valid allocated pointer
 */
bool_t
derefable_cp(const config_param_t *p_cp) {
  if (p_cp == CONFPARAM_REQUIRED) {
    return FALSE;
  }

  if (p_cp == CONFPARAM_MISSINGOK) {
    return FALSE;
  }
  return TRUE;
}

config_param_t *
dup_cp(config_param_t *p_from_cp) {
  config_param_t *p_to_cp = NULL;
  
  if (!derefable_cp(p_from_cp)) {
    return NULL;
  }
  /*
   * individual jobs are dynamic; there is no per-job configuration parameter
   * so, create a new one, mostly copied from the job directory, p_jobd.
   *
   * This needs to be a copy, so that individual jobs don't collide.
   */
  if (!p_from_cp) {
    ErrExit(ErrExit_ASSERT, "dup_cp: NULL p_from_cp");
    return NULL;
  }
  
  if (!(p_to_cp = calloc(1, sizeof(config_param_t)))) {
    ErrExit(ErrExit_ASSERT, "dup_cp: !calloc(p_cp)");
    return NULL;
  }

  /*
   * sub-fields point to original storage locations
   * they are not duplicated
   */
  if (memcpy(p_to_cp, p_from_cp, sizeof(config_param_t)) != p_to_cp) {
    ErrExit(ErrExit_ASSERT, "dup_cp: !memcpy()");
    free(p_to_cp);
    return NULL;
  }
  return p_to_cp;
}

config_param_t *
init_dependency_cp(config_param_t *p_cp) {
  config_param_t *p_cp_depends_on;
  
  if (!p_cp) {
    ErrExit(ErrExit_INTERNAL, "init_dependency_cp: !p_cp");
    return NULL;
  }
  init_hash_cp(p_cp); /* just in case it's not yet set */

  if (!p_cp->depends_on.nm) {
    return p_cp;                  /* no dependency */
  }
  if (derefable_cp(p_cp->depends_on.p_cp)) {
    return p_cp->depends_on.p_cp; /* previously found */
  }
  
  p_cp_depends_on = getconfig_fromnm_nohash(p_cp->depends_on.nm);
  if (!p_cp_depends_on) {
    ErrExit(ErrExit_INTERNAL, "collect_dependency_nm: !p_cp_depends_on");
  }
  
  if (p_cp->depends_on.p_cp) {
    if (p_cp->depends_on.p_cp != p_cp_depends_on) {
      ErrExit(ErrExit_INTERNAL, "init_dependency_nm: p_cp_depends_on mismatch");
    }
  } else {
    p_cp->depends_on.p_cp = p_cp_depends_on;
  }
  if (!derefable_cp(p_cp_depends_on)) {
    init_hash_cp(p_cp_depends_on);
  }
  return p_cp_depends_on;
}

