
#include "rmfs.h"

/*
 * namer()
 *  convert a pathname to an rnode
 *   namer() is modeled after namei()/lookuppn()
 *** XXX not reentrant: calls strchr()
 */

rnode_t *
namer(const char *path, rnode_t *p_lookup, int *p_errno) {
  rnode_t             *p_match, *p_fsroot, *p_child;
  char                *non_terminal, *p_nextc, *needfree;
  int                  h, i, slashes;
  extern rn_param_t    rn_paramtab;

  non_terminal = NULL;
  needfree     = NULL;
  p_match      = NULL;
  p_nextc      = NULL;

  if (!path) {
    goto out;
  }
  if (!p_errno) {
    goto out;
  }
  *p_errno = 0;

  p_fsroot = rn_paramtab.p_fsroot;
  if (!p_fsroot) {
    *p_errno = EFAULT;
    goto out;
  }
  if (!p_lookup) {
    p_lookup = p_fsroot;
  }
  /*allocate space for next token, which we may need to modify, !const*/
  if (!(p_nextc = needfree = strdup(path))) {
    *p_errno = ENOMEM;
    goto out;
  }
  
  /*consume any leading slashes*/
  for (slashes = 0; *p_nextc == '/'; p_nextc++, slashes++) {
    ;
  }

  /* short-circuit on '.' and '..' */
  if (*p_nextc == '\0' || (*p_nextc == '.' && *(p_nextc+1) == '\0')) {
    p_match = p_lookup;
    goto out;
  }
  if (*p_nextc == '.' && *(p_nextc+1) == '.' && *(p_nextc+2) == '\0') {
    p_match = p_lookup->parent;
    goto out;
  }
  /*
   * p_nextc now contains the next token, but it may not be the terminal token
   */
  if (NULL != (non_terminal = strchr(p_nextc, '/'))) {
    int l    = (non_terminal - p_nextc);
    slashes += l;
    *non_terminal = '\0'; /*modifies p_nextc, terminating this token */
  }
  h = djb_strtohash(p_nextc);
  if (h == 0 || h == ~0) {
    *p_errno = EFAULT;
    goto out;
  }

  if (!p_lookup->is.dir) {
    if (p_lookup->h == h) {  /* non-directories match here */
      if (non_terminal) {
	*p_errno = ENOTDIR;
	goto out;
      }
      p_match = p_lookup;
    }
    goto out;
  } 

  /* directory */
  if (p_lookup->h == h) {
    if (!non_terminal) {  /* final component is this directory, p_lookup */
      p_match = p_lookup;
      goto out;
    }
  }
    
  /*
   * dir matched, but non_terminal, search for a match in our children
   */

  /*
   * walk the children list,
   * provided that there isn't a match yet found,
   * an errno has not been set
   * the children seem reasonably fit
   * and we have not walked off the end of the list of children nodes
   */
  for (p_match = NULL, p_child = p_lookup->children, i = 0;
       i < p_lookup->n_children;
                               p_child++, i++) {

    if (p_match) {
      break;
    }
    if (*p_errno != 0) {
      break;
    }
    if (!p_child) {
      break;
    }
    if (!p_child->nm) {
      break;
    }
    if (!IS_VALID_HASH(p_child->h)) {
      continue;
    }
    if (!IS_RTYPE_VALID(p_child->rtype)) {
      continue;
    }
    if (RN_GUARD == p_child->rtype) {
      continue;
    }

    if (h == p_child->h) {
      if (p_child->is.dir) {
      /*
       * matches one of our children, but may be a non-terminal component itself
       * so descend to check the child
       */
	p_match = namer((path + slashes), p_child, p_errno);
      } else {
	p_match = p_child;
      }
    }
  }
  if (!p_match) {
    *p_errno = ENOENT;
  }

 out:
  if (needfree) {
    free(needfree);
    needfree = NULL;
  }
  if (p_errno && *p_errno != 0) {
    return NULL;
  }

  return p_match;
}
