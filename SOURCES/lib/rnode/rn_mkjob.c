#include "rmfs.h"


/*
 * rn_mkjobd
 *  construct directory with sub-directory entries corresponding to jobids
 *  there are no explicit attributes to the the "jobs" (RND_JOBS) directory
 */
rnode_t *
rn_mkjobd(rnode_t *p_cluster, rnode_t *p_jobd) {
  rnode_t        *p_new, *p_children, *p_subdir, *p_rn, *p_jobidX;
  rnode_t      *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t         rn_pro;
  config_param_t *p_cp, *p_2cp;
  unsigned long   n_children;
  int             i;

  job_info_msg_t *p_jim;                   /* slurm.h */ 
  job_info_t     *p_job_array;             /* slurm.h */
  job_info_t     *p_jai;                   /* job array indexer */

  extern rnode_t          *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);
   
   if (!p_cluster) {
     ErrExit(ErrExit_ASSERT, "rn_mkjobd(NULL parent)");
     return NULL;
   }
   
   p_cp = dup_cp(getconfig_fromnm(rnode_buildtab[RND_JOBS].nm));
   if (!derefable_cp(p_cp)) {
     ErrExit(ErrExit_ASSERT, "mkjobd: !p_cp");
     return NULL;
   }
   if (!p_cp->per_src.slurm.jim) {
     ErrExit(ErrExit_ASSERT, "mkjobd: no slurm job information msg parameter");
     return NULL;
   }

   p_jim                  = p_cp->per_src.slurm.jim;
   p_job_array            = p_jim->job_array;
   p_cp->val.ue.base_addr = (void *) p_job_array;

   if (p_jim->last_update == 0) {
     ErrExit(ErrExit_ASSERT, "mkjobd: j_pim->last_update == 0");
     return NULL;
   }
   if (!p_job_array) {
     ErrExit(ErrExit_ASSERT, "mkjobd: empty p_cp->per_src.slurm.jim->job_array");
     return NULL;
   }
   if (p_jim->record_count == 0) {
     ErrExit(ErrExit_WARN, "mkjobd: p_cp->per_src.slurm.jim->record_count == 0");
   }
   n_children = p_jim->record_count;

   /*
    *  (p_jobd was allocated by the parent's call to the provisioner)
    *   p_new->children = rnode[0], freshly provisioned
    *
    *   rnode[0] = 1st named job
    *   rnode[1] = 2nd named job
    *        ...
    *   rnode[n] = n+1 named partition 
    */

   p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
   rn_pro.n_children = n_children + 1;
   if (!(p_new = (*p_buildfn)(p_jobd, &rn_pro))) {
     ErrExit(ErrExit_ASSERT, "rn_mkjobd(provision failure)- out of memory?");
     return NULL;
   }
   p_children = p_subdir = p_new;
   p_jobd     = rn_cast(p_jobd, RND_JOBS,
			CONFPARAM_REQUIRED, p_job_array, p_cp->nm,
			p_cluster, p_children, n_children,
			/*attr*/ NULL, p_subdir);

   if (!derefable_cp(p_jobd->p_cp)) {
     ErrExit(ErrExit_ASSERT, "rn_mkjobd: NULL p_jobd->p_cp");
     return NULL;
   }

   /* ...  "jobs" [p_parent=RND_CLUSTER, rtype=RND_JOBS]
    *            |
    *            + <jobid1> [p_parent=RND_JOBS, rtype=PARTNAME]
    *            |
    *            + <jobid2> [p_parent=RND_JOBS, rtype=PARTNAME]
    */

   for (i = 0, p_rn = p_subdir, p_jai = p_job_array;
	    i < p_jim->record_count;
               i++, p_jai++
	) {
     char *nm;
     char  sbuf[_POSIX_PATH_MAX];
     int   l;

     /*
      * job names are arbitrary user-specified strings, and may not be unique
      * store them as an attribute; use the jobid as the directory entry
      */
     l = snprintf(sbuf, _POSIX_PATH_MAX-1, "%d", p_jai->job_id);
     if (l < 0) {
       ErrExit(ErrExit_ASSERT, "rn_mkjobd(snprintf(p_jai->job_id)))");
       return NULL;
     }
     nm = strndup(sbuf, l+1);
     if (!nm) {
       ErrExit(ErrExit_ASSERT, "rn_mkjobd: !nm");
       return NULL;
     }

     /*
      * each jobid requires its own p_cp, to hold the per-job steps
      * copy it from the parent, p_jobd
      */
     if (!(p_2cp = dup_cp(p_jobd->p_cp))) {
       ErrExit(ErrExit_ASSERT, "rn_mkjobd: out of memory? !dup_cp(p_job->p_cp)");
       return NULL;
     }
     p_cp->nm = p_cp->val.ue.str = nm;
     p_jobidX = rn_cast(p_rn, RND_JOBID,
		    p_cp, p_jai, nm,
		    p_jobd, /*children*/ NULL, 0,
		    /*attr*/ NULL, /*subdir*/ NULL);
     if (!p_jobidX) {
       ErrExit(ErrExit_ASSERT, "rn_mkjobd: !p_jobidX");
       return NULL;
     }
     p_rn++;
   }
   set_rnparam_jobd(p_jobd);
   return p_jobd;
}

/*
 * rn_mkjobid
 *  construct per-job directory with "jobsteps" sub-directory
 *  attributes are in the jobinfodesc_tab[]
 */


void
rn_resetugid(rnode_t *p_dir, uid_t uid, gid_t gid) {
  rnode_t *p_rn;
  int      i;

  p_rn = NULL;
  if (!p_dir) {
    ErrExit(ErrExit_INTERNAL, "resetattrid: !p_dir");
  }
  if (!IS_RTYPE_VALID(p_dir->rtype)) {
    ErrExit(ErrExit_ASSERT, "resetugid: !IS_VALID_RTYPE");
  }

  p_dir->uid = uid;
  p_dir->gid = gid;

  for (i = 0, p_rn = p_dir->children; i < p_dir->n_children; i++, p_rn++) {
    
    if (p_rn->children) {
      rn_resetugid(p_rn, uid, gid);
      
    } else {
      p_rn->uid = uid;
      p_rn->gid = gid;
    }
  }
  return;
}


rnode_t *
rn_mkjobid(rnode_t *p_jobd, rnode_t *p_jobid) {
  rnode_t        *p_new, *p_jobsteps, *p_children, *p_subdir, *p_attrd, *p_attr, *p_control;
  rnode_t      *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t         rn_pro;
  config_param_t *p_cp, *p_2cp, *p_cp_uid, *p_cp_gid;
  unsigned long  n_children;
  int            n_attr;
  
  extern config_param_t *getconfig_from_myattrnm(rnode_t *, char *);
  extern int             attr_cnt_cp(rn_type_t); /*rn_subr.c*/
  extern rnode_t        *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);
  
  job_info_t    *p_ji;   /* slurm.h */

   if (!p_jobd) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid(NULL parent) p_jobd");
    return NULL;
  }
  p_cp = p_jobid->p_cp;
  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid: !p_cp");
    return NULL;
  }

  /*
   * slurm info job array entry was squirreled away by the RND_JOBS setup
   */
  p_ji = (job_info_t *) p_jobd->p_dyntyp;
  if (!p_ji) {
    ErrExit(ErrExit_ASSERT, "mkjobid: !job info ptr");
    return NULL;
  }
  n_children = 2; /*RND_JOBSTEPS, RND_CONTROL */
  n_attr = attr_cnt_cp(RND_JOBID);
  
  if (n_attr > 0) { /*RND_ATTRIBUTES*/
    n_children++;
  }

  /*
   *  (p_jobid was allocated by the parent's call to the provisioner)
   *   p_new->children = rnode[0], freshly provisioned
   *
   *   rnode[0] = "jobsteps" subdirectory
   *   rnode[1] = "attributes" subdirectory
   *   rnode[2] = "control" subdirectory
   */

  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  rn_pro.n_children = n_children + 1;
  if (!(p_new = (*p_buildfn)(p_jobid, &rn_pro))) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid(provision failure)- out of memory?");
    return NULL;
  }

  p_subdir = p_children = p_new;
  p_attr   = p_children + 1;

  /* ...  <jobidX> [p_parent=RND_JOBS, rtype=RND_JOBID]
   *            |
   *            + "jobsteps" [p_parent=p_jobid, rtype=RND_JOBSTEPS]
   *            |
   *            + "attributes" [p_parent=p_jobid, rtype=RND_ATTRIBUTES]
   *            |
   *            + "control" [parent=p_jobid, rtype=RND_CONTROL]
   */

  p_jobid = rn_cast(p_jobid, RND_JOBID,
		    p_cp, p_ji, p_cp->nm,
		    p_jobd, p_children, n_children,
		    p_attr, p_subdir);
  if (!p_jobid) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid: !p_jobid");
    return NULL;
  }

  p_cp = dup_cp(getconfig_fromnm(rnode_buildtab[RND_JOBSTEPS].nm));
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "mkjobid: jobsteps, dup_cp fail - out of memory?");
    return NULL;
  }
  p_jobsteps = rn_cast(p_subdir, RND_JOBSTEPS,
		       p_cp, /*dyntyp*/ NULL, p_cp->nm,
		       p_jobid, /*children*/ NULL, 0,
		       /*attr*/ NULL, /*subdir*/ NULL);
  if (!p_jobsteps) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid: !p_jobsteps");
    return NULL;
  }

  if (n_attr <= 0) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid: attr count <= 0");
    return NULL;
  }
    
  p_2cp = dup_cp(p_cp);
  if (!derefable_cp(p_2cp)) {
    ErrExit(ErrExit_ASSERT, "mkjobid: attributes, dup_cp fail - out of memory?");
    return NULL;
  }
  p_2cp->nm = rnode_buildtab[RND_ATTRIBUTES].nm;
  p_attrd = rn_cast(p_jobsteps+1, RND_ATTRIBUTES,
		    p_2cp, /*dyntyp*/ p_ji, p_2cp->nm,
		    p_jobid, /*children*/ NULL, 0,
		    /*attr*/ NULL, /*subdir*/ NULL);

  if (!p_attrd) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid: rn_cast_attr_typconv() = !p_attrd");
    return NULL;
  }

  p_control = rn_cast(p_attrd+1, RND_CONTROL,
		      CONFPARAM_MISSINGOK, /*dyntyp*/ NULL,
		      rnode_buildtab[RND_CONTROL].nm,
		      /*parent*/ p_jobid, /*children*/ NULL, /*n_children*/ 0,
		      /*attr*/ NULL, /*subdir*/ NULL);
  if (!p_control) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid: rn_cast(control) NULL");
    return NULL;
  }

  /*
   * set job ownership attributes so that a user may view their own jobs & jobsteps
   */
  p_cp_uid = getconfig_from_myattrnm(p_attrd, "user_id");
  p_cp_gid = getconfig_from_myattrnm(p_attrd, "group_id");
  if (!p_cp_uid) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid: !p_cp = getconfig_from_myattrnm(user_id)");
    return NULL;
  }
  if (!p_cp_gid) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobid: !p_cp = getconfig_from_myattrnm(group_id)");
    return NULL;
  }
  rn_resetugid(p_jobid, p_cp_uid->val.ue.uid, p_cp_gid->val.ue.gid);

  return p_jobid;
}

