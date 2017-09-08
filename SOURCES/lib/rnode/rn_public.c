
#include "rmfs.h"

/*
 * the following two functions: get_rn_buildfn() and mk_fs() constitute the
 * public interface to the rnode() node type builders
 *
 *
 */

/*
 * get_rn_buildfn()
 *  accessor function for the (*build_function)() member of the rnode
 *
 * given an rtype, returns a pointer to a function that returns a ptr to an rnode
 * the function referenced & returned takes two rnode_t ptrs as its args
 */

rnode_t *
(*get_rn_buildfn(rn_type_t rtyp))(rnode_t *, rnode_t *) {
  rnode_t   *p_rn;
  rnode_t *(*p_buildfn)(rnode_t *, rnode_t *);
  
  if (!IS_RTYPE_BUILDABLE(rtyp)) {
    ErrExit(ErrExit_ASSERT, "get_rn_buildfn: unbuildable rtyp");
    return NULL;
  }
  p_rn = &rnode_buildtab[rtyp];
  if (!p_rn) {
    ErrExit(ErrExit_ASSERT, "get_rn_buildfn: null rnode_buildtab[rtyp]");
    return NULL;
  }
  p_buildfn = p_rn->buildfn;
  if (!p_buildfn) {
    ErrExit(ErrExit_ASSERT, "get_rn_buildfn: null rnode_buildtab[rtyp].buildfn");
    return NULL;
  }
  return p_buildfn;
}

/*
 * mk_fs()
 *  trigger construction of the fs by calling the build function for the root
 */
extern void
mk_fs(void) {
  rnode_t    *p_new;
  rnode_t  *(*p_buildfn)(rnode_t *, rnode_t *);
  
  extern void effluviate_fstree(rnode_t *, int);
  extern void rn_poolinit(void);

  rn_poolinit();

  p_buildfn = get_rn_buildfn(RND_ROOT);
  if (!p_buildfn) {
    ErrExit(ErrExit_INTERNAL, "mk_fs(no build func)");
  }

  if (!(p_new = (*p_buildfn)(/*parent*/ NULL, 0))) {
    ErrExit(ErrExit_INTERNAL, "mk_fs: build rnode(RND_ROOT) failed");
  }
  /*XXX ck_fs(p_new);*/
  return;
}
