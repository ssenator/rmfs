#include "rmfs.h"


/*
 * rn_mkctld
 *  construct a RND_CONTROL directory with leaf nodes corresponding to the
 *   commands known to the parent being controlled
 *
 * These nodes implement commands that are appropriate to their parent
 * So, for an fsroot, the control node requests a list of known commands
 * from the fsroot, and calls the appropriate routine triggered by that command
 * The rnode_buildtab contains the commands and implementing function.
 *
 * Commands are simple verbs to be executed. If the command requires parameters
 * these are to be supplied by a writable file descriptor on the cmd attr entry
 */
rnode_t *
rn_mkctld(rnode_t *p_parent, rnode_t *p_ctl) {
  rnode_t        *p_children, *p_cmdX, *p_new, *p_attr, *p_rn, *p_subdir;
  rnode_t      *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t         rn_pro;
  rn_ctl_t       *p_ctl_cmd;
  int             n_cmds, i;
  unsigned long   n_children;

  extern rnode_t          *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);
   
  if (!p_parent) {
    ErrExit(ErrExit_ASSERT, "rn_mkctl(NULL parent)");
    return NULL;
  }

  for(i = 0, n_cmds = 0; i < MAX_RN_CMDS && p_parent->ctl[i].nm; i++) {
    if (p_parent->ctl[i].nm) {
      n_cmds++;
    }
  }
  n_children = n_cmds;

  /*
   *  (p_ctl was allocated by the parent's call to the provisioner)
   *   p_new->children = rnode[0], 1st cmd
   *
   *   rnode[0] = 1st cmd (attribute)
   *   rnode[1] = 2nd cmd     ... 
   */

  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  rn_pro.n_children = n_children + 1;
  if (!(p_new = (*p_buildfn)(p_ctl, &rn_pro))) {
    ErrExit(ErrExit_ASSERT, "rn_mkctl(provision failure)- out of memory?");
    return NULL;
  }
  p_children = p_attr = p_subdir = p_new;

  p_ctl = rn_cast(p_ctl, RND_CONTROL,
		  CONFPARAM_MISSINGOK, /*dyntyp*/ NULL,
		  rnode_buildtab[RND_CONTROL].nm,
		  p_parent, p_children, n_children,
		  /*attr*/ NULL, p_subdir);


  /*
   *  ...  "control" [p_parent=<variable> rtype=RND_CONTROL ]
   *            |
   *            + cmdX [p_parent=p_ctl, rtype=RNF_KNOB]
   *
   * control nodes have no sub-directories, only attributes
   * these are attributes with special semantics => control knobs
   * control knobs may be executed or written to trigger the relevent command
   *
   * The semantics are implemented in the file op function
    */
   for (i = 0, p_rn = p_attr, p_ctl_cmd = &p_parent->ctl[i];
	    p_ctl_cmd->nm;
	        i++, p_ctl_cmd++
	) {
     config_param_t *p_cp; 
    
     if (!p_ctl_cmd->nm) {
       ErrExit(ErrExit_ASSERT, "rn_mkctl: !p_ctl_cmd->nm");
       return NULL;
     }
     /*
      * the dyntyp ptr is set to this attribute's associated control function
      */
     p_cp = calloc(1, sizeof(config_param_t));
     if (!p_cp) {
       ErrExit(ErrExit_ENOMEM, "rn_mkctl: !calloc(RNF_KNOB)");
       return NULL;
     }
     p_cp->nm = p_cp->val.ue.str = p_ctl_cmd->nm;
     p_cmdX = rn_cast(p_rn, RNF_KNOB,
		      p_cp, p_ctl_cmd->fn,
		      p_ctl_cmd->nm,
		      p_ctl, /*children*/ NULL, 0,
		      /*attr*/ NULL, /*subdir*/ NULL);

     if (!p_cmdX) {
       ErrExit(ErrExit_ASSERT, "rn_mkctl: !p_rn post rn_cast_attr_typconv() ");
       return NULL;
     }
     p_rn++;
   }
   return p_ctl;
}

