
#include "rmfs.h"

/*
 * typ_copyout() 
 * returns # bytes written, possibly 0, or -errno
 */
int
typ_copyout(config_param_t *p_cp, char *out, size_t size) {
  int        l;
  ptyp_t     ptyp;
  struct tm *p_tm = NULL;

  if (!p_cp) {
    return -EIO;
  }
  if (!out) {
    return -ENOMEM;
  }
  if (p_cp->val.size > size) {
    return -ERANGE;
  }

  l = size;
  if (p_cp->val.size > 0) {
    l = min(size, p_cp->val.size);
  }
  ptyp = p_cp->typ;
  
  if (memset(out, 0, l) < 0) {
    return -EIO;
  }
  
  /*XXX tablify*/
  if (IS_SLURM_TYPE(ptyp)) {
    switch (ptyp) {
    case PTYP_NODESTATE:
    case PTYP_ALLOCJOB:
      ptyp = PTYP_OPAQUE;
      break;
    case PTYP_SLURMVERSION:
    case PTYP_SLURMUID:
    case PTYP_SLURMTMOUT:
      ptyp = PTYP_NUMERIC;
      break;
    default:
      ptyp = PTYP_ALPHANUM;
      break;
    }
  }
        
  if (IS_OPAQUE_TYPE(ptyp)) {
    snprintf(out, size, "0x%x\n", OPAQUE_EMIT);
    l = internal_strlen(out)+1;

  } else if (IS_CONTEXT_TYPE(ptyp)) {
    if (p_cp->val.ue.str && l > 0) {
      if (memcpy(out, p_cp->val.ue.str, l) != out) {
	return -EIO;
      }
    }
  }  else if (IS_FSVIS_TYPE(ptyp)) {
    if (!p_cp->val.ue.pathnm) {
      return -EFAULT;
    }
    if (memcpy(out, p_cp->val.ue.pathnm, l) != out) {
      return -EIO;
    }
  } else if (IS_ALPHA_TYPE(ptyp)) {
    if (!p_cp->val.ue.str) {
      return 0; /* successfully copy out an empty string */
    }
    if (memcpy(out, p_cp->val.ue.str, l) != out) {
      return -EIO;
    }
  } else if (IS_LOGICAL_TYPE(ptyp)) {
    char *from;
	
    if (IS_BOOLEAN_TYPE(ptyp)) {
      from = b2boolstr(p_cp->val.ue.btruth);
      
    } else if (IS_TRILENE_TYPE(ptyp)) {
      from = t2tristr(p_cp->val.ue.truth);
    }
    l = internal_strlen(from);
    if (l <= 0 || memcpy(out, from, l+1) != out) {
      return -EIO;
    }

  } else if (IS_NUMERIC_TYPE(ptyp)) {
    /* attempt to emit an understandable representation of the numeric value  */
    if (PTYP_UID == ptyp) {
      snprintf(out, size, "%d\n", p_cp->val.ue.uid);
      l = internal_strlen(out)+1;		

    } else if (PTYP_PID == ptyp) {
      snprintf(out, size, "%d\n", p_cp->val.ue.pid);
      l = internal_strlen(out)+1;	

    } else if (PTYP_NUMTIME_SECS == ptyp) {

      p_tm = localtime(&p_cp->val.ue.time);
      if (!p_tm) {
	snprintf(out, size, "%ld\n", p_cp->val.ue.time);
	l = internal_strlen(out)+1;
      } else {
	l = strftime(out, size, "%s\n", p_tm);
	p_cp->val.size = l;
      }
      
    } else if (PTYP_NUMERICTIME == ptyp) {

      p_tm = localtime(&p_cp->val.ue.time);
      if (!p_tm) {
	snprintf(out, size, "%ld\n", p_cp->val.ue.time);
	l = internal_strlen(out)+1;
      } else {
	l = strftime(out, size, "%F %T\n", p_tm);
	p_cp->val.size = l;
      }

    } else if (PTYP_NUMSIGNED == ptyp) {
      snprintf(out, size, "%ld\n", p_cp->val.ue.l);
      l = internal_strlen(out)+1;	

    } else if (PTYP_NUMERIC == ptyp) {
      snprintf(out, size, "%ld\n", (unsigned long) p_cp->val.ue.ul);
      l = internal_strlen(out)+1;	

    } else if (PTYP_UNSIGNED_INT16 == ptyp) {
      snprintf(out, size, "%u\n", p_cp->val.ue.ui_16);
      l = internal_strlen(out)+1;
      
    } else if (PTYP_UNSIGNED_INT32 == ptyp) {
      snprintf(out, size, "%u\n", p_cp->val.ue.ui_32);
      l = internal_strlen(out)+1;

    } else {
      (*(unsigned long *) out) = p_cp->val.ue.ul;
      l = sizeof(unsigned long);
    }
  } else {
    return -EINVAL;
  } 

  return l;
}

/*
 * typ_copyin()
 */
int
typ_copyin(config_param_t *p_cp, char *in, size_t size) {
  ptyp_t       ptyp;
  rmfs_param_t new_val;

  ptyp = p_cp->typ;

  if (IS_OPAQUE_TYPE(ptyp)) {
    /*refuse to blindly copyin an opaque value*/
    errno = -EFAULT;
    return -1;
  }
  new_val.ue.ptr = in;
  if (!in) {
    errno = -ENXIO;
    return -1;
  }
  if (!typ_check(ptyp, &new_val)) {
    errno = -EINVAL;
    return -1;
  }

  if (IS_CONTEXT_TYPE(ptyp)) {
    set_val_charptr(p_cp, in);

  } else if (IS_NUMERIC_TYPE(ptyp)) {
    set_val_ul(p_cp, strtol(in, /*endptr*/ NULL, /*base*/ 10));
	
  } else if (IS_NUMSIGNED_TYPE(ptyp)) {
    set_val_l(p_cp, strtol(in, /*endptr*/ NULL, /*base*/ 10));

  } else if (IS_ALPHA_TYPE(ptyp)) {
    set_val_charptr(p_cp, in);
	
  } else {
    set_val_ptr(p_cp, (void *) in);
  }
  return 0;
}

