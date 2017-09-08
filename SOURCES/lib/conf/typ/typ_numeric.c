
#include "rmfs.h"

tri_t
typ_numeric(rmfs_param_t *p_val) {
  long long  l;

  if (!p_val) {
        ErrExit(ErrExit_ASSERT, "typ_numeric(): !p_val");
	return FALSE;
  }
  l = (long long) p_val->ue.ul;
  if (l < LONG_MIN || l > LONG_MAX) {
    return FALSE;
  }
  return TRUE;
}

/*
 * XXX convert to next largest type for range checking?
 * int, long, unsigned etc
 */

tri_t
typ_numsigned(rmfs_param_t *p_val) {
  long long  l;
  
  if (!p_val) {
        ErrExit(ErrExit_ASSERT, "typ_numsigned(): !p_val");
	return FALSE;
  }
  l = (long long) p_val->ue.l;
  if (l < LONG_MIN || l > LONG_MAX) {
    return FALSE;
  }
  return TRUE;
}

tri_t
typ_unsigned_int(rmfs_param_t *p_val) {
  unsigned long ul;
  
  if (!p_val) {
    ErrExit(ErrExit_ASSERT, "typ_unsigned_int: !p_val");
    return FALSE;
  }
  ul = (unsigned int) p_val->ue.ui;
  if (ul > UINT_MAX) {
    return FALSE;
  }
  return TRUE;
  
}

tri_t
typ_unsigned_int16(rmfs_param_t *p_val) {
  uint16_t ui_16;
  
  if (!p_val) {
    ErrExit(ErrExit_ASSERT, "typ_unsigned_int16: !p_val");
    return FALSE;
  }
  ui_16 = p_val->ue.ui_16;
  if (ui_16 > UINT_MAX) {
    return FALSE;
  }
  return TRUE;
  
}

tri_t
typ_unsigned_int32(rmfs_param_t *p_val) {
  uint32_t ui_32;
  
  if (!p_val) {
    ErrExit(ErrExit_ASSERT, "typ_unsigned_int32: !p_val");
    return FALSE;
  }
  ui_32 = p_val->ue.ui_32;
  if (ui_32 > UINT_MAX) {
    return FALSE;
  }
  return TRUE;
  
}

tri_t
typ_numerictime(rmfs_param_t *p_val) {
  tri_t num = typ_numeric(p_val);

  /* not <0 in this universe http://journals.aps.org/prl/abstract/10.1103/PhysRevLett.113.181101 */
  return (num == TRUE && p_val->ue.time >= 0);
}

tri_t
typ_numtimsecs(rmfs_param_t *p_val) {
  
  if (!p_val) {
    ErrExit(ErrExit_ASSERT, "typ_numtimsecs: !p_val");
    return FALSE;
  }

  /* range checking? */
  return typ_numerictime(p_val);
}
