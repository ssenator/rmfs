#include "rmfs.h"

/*
 * rn_mkjobstepd
 *  construct directory with sub-directory entries corresponding to jobids
 *  there are no explicit attributes to the the "jobs" (RND_JOBS) directory
 */
rnode_t *
rn_mkjobstepd(rnode_t *p_jobid, rnode_t *p_jobstepd) {
  rnode_t          *p_new, *p_children, *p_subdir, *p_rn, *p_stepX;
  rnode_t        *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t           rn_pro;
  config_param_t   *p_cp;
  unsigned long     n_children;
  int               i;

  job_step_info_response_msg_t *p_stim;      /* slurm.h */ 
  job_step_info_t              *p_job_steps; /* slurm.h */
  job_step_info_t              *p_jsai;      /* job step array indexer */

  extern rnode_t          *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);
  
   if (!p_jobid) {
     ErrExit(ErrExit_ASSERT, "rn_mkjobstepd(NULL parent jobid)");
     return NULL;
   }
   if (!p_jobid->p_dyntyp) {
     ErrExit(ErrExit_ASSERT, "rn_mkjobstepd(NULL jobid->p_dyntyp)");
     return NULL;
   }

   p_cp = p_jobstepd->p_cp;
   if (!derefable_cp(p_cp)) {
     ErrExit(ErrExit_ASSERT, "mkjobstepd: !p_cp");
     return NULL;
   }
   /*
    * because our parent node (jobid) is dynamic, the collectslurm_* routines
    * had no opportunity to collect the job step tables for this job, do so now
    */
  
   if (!p_cp->per_src.slurm.stim) {
     ErrExit(ErrExit_ASSERT, "mkjobstepd: no slurm job step information msg parameter");
     return NULL;
   }

   p_stim                 = p_cp->per_src.slurm.stim;
   p_job_steps            = p_stim->job_steps;
   p_cp->val.ue.base_addr = (void *) p_job_steps;
   
   if (p_stim->last_update == 0) { /* XXX verify that slurm sets this on OUT */
     ErrExit(ErrExit_ASSERT, "mkjobstepd: p_cp->p_stim->last_update == 0");
     return NULL;
   }
   if (!p_job_steps) {
     ErrExit(ErrExit_WARN, "mkjobstepd: empty p_cp->per_src.slurm.p_stim.stim->job_steps");
     return NULL;
   }
   if (p_stim->job_step_count == 0) {
     ErrExit(ErrExit_WARN, "mkjobstepd: p_cp->p_stim->job_step_count == 0");
     return NULL;
   }

   n_children = p_stim->job_step_count;
   /*
    *  (p_jobstepsd was allocated by the parent's call to the provisioner)
    *   p_new->children = rnode[0], freshly provisioned
    *
    *   rnode[0] = 1st job step
    *   rnode[1] = 2nd job step
    *        ...
    *   rnode[n] = n+1 job step
    */

   p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
   rn_pro.n_children = n_children + 1;
   if (!(p_new = (*p_buildfn)(p_jobstepd, &rn_pro))) {
     ErrExit(ErrExit_ASSERT, "rn_mkjobstepd(provision failure)- out of memory?");
     return NULL;
   }

   p_children = p_subdir = p_new;

   /* ...  "jobsteps" [p_parent=RND_JOBID, rtype=RND_JOBSTEP]
    *            |
    *            + <jobX:step1> [p_parent=RND_JOBSTEP, rtype=RND_STEPID]
    *            |
    *            + <jobX:step2> [p_parent=RND_JOBSTEP, rtype=RND_STEPID]
    *           ...
    */

   /*
    * a user is allowed access to their own jobs
    */
   p_jobstepd->uid = p_jobstepd->parent->uid;
   p_jobstepd->gid = p_jobstepd->parent->gid;
  
   p_jobstepd = rn_cast(p_jobstepd, RND_JOBSTEPS,
			CONFPARAM_REQUIRED, p_job_steps,
			rnode_buildtab[RND_JOBSTEPS].nm,
			p_jobid, p_children, n_children,
			/*attr*/ NULL, p_subdir);

   for (i = 0, p_rn = p_subdir, p_jsai = p_job_steps;
	    i < p_stim->job_step_count;
               i++, p_jsai++
       ) {
     char *nm;
     char  sbuf[_POSIX_PATH_MAX];
     int   l;

     /*
      * job step names are arbitrary user-specified strings, and may not be unique
      * store them as an attribute
     */
     l = snprintf(sbuf, _POSIX_PATH_MAX-1, "%d", p_jsai->step_id);
     if (l < 0) {
       ErrExit(ErrExit_ASSERT, "rn_mkjobstepd(snprintf(p_jsai->step_id)))");
       return NULL;
     }
     nm = strndup(sbuf, l+1);
     if (!nm) {
       ErrExit(ErrExit_ASSERT, "rn_mkjobstepd: !nm");
       return NULL;
     }
     
     p_cp = dup_cp(p_jobstepd->p_cp);
     if (!p_cp) {
       ErrExit(ErrExit_ASSERT, "mkjobstepd: p_stepX, dup_cp fail - out of memory?");
       return NULL;
     }
     p_cp->nm = p_cp->val.ue.str = nm;

     p_stepX = rn_cast(p_rn, RND_JOBSTEPID,
		       p_cp, p_jsai, nm,
		       p_jobstepd, /*children*/ NULL, 0,
		       /*attr*/ NULL, /*subdir*/ NULL);
     if (!p_stepX) {
       ErrExit(ErrExit_ASSERT, "rn_mkjobstepd: !p_rn");
       return NULL;
     }
     
     p_stepX->uid = p_stepX->parent->uid;
     p_stepX->gid = p_stepX->parent->gid;
     
     p_rn++;
   }
   return p_jobstepd;
}


/*
 * rn_mkjobstep
 *  construct per-step directory
 *  attributes are in the stepinfodesc_tab[]
 *
 */
rnode_t *
rn_mkjobstep(rnode_t *p_jobstepd, rnode_t *p_stepX) {
  rnode_t        *p_new, *p_children, *p_subdir, *p_attrd, *p_attr;
  rnode_t      *(*p_buildfn)(rnode_t *, rnode_t *);
  rnode_t         rn_pro;
  config_param_t *p_cp, *p_2cp;
  unsigned long   n_children;
  int             n_attr;

  job_step_info_t *p_jsti;   /* slurm.h */

  extern int       attr_cnt_cp(rn_type_t); /*rn_subr.c*/
  extern rnode_t          *rn_cast(rnode_t *, rn_type_t, config_param_t *, void *, char *, rnode_t *, rnode_t *, int, rnode_t *, rnode_t *);

  if (!p_jobstepd) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobstep(NULL parent) p_jobstepd");
    return NULL;
  }
  p_cp = p_stepX->p_cp;
  if (!derefable_cp(p_cp)) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobstep: !p_cp RND_JOBSTEPID");
    return NULL;
  }
  /*
   * slurm info step array entry was squirreled away by the RND_JOBSTEPS setup
   */
  p_jsti = (job_step_info_t *) p_stepX->p_dyntyp;
  if (!p_jsti) {
    ErrExit(ErrExit_ASSERT, "mkjobstep: !job step info ptr");
    return NULL;
  }

  n_children = 0;
  n_attr = attr_cnt_cp(RND_JOBSTEPID);
  
  if (n_attr > 0) { /*RND_ATTRIBUTES*/
    n_children++;
  }
  /*
   *  (p_stepid was allocated by the parent's call to the provisioner)
   *   p_new->children = rnode[0], freshly provisioned
   *
   *   rnode[0] = "attributes"
   */

  p_buildfn = rnode_buildtab[RN_PROVISION].buildfn;
  rn_pro.n_children = n_children + 1;
  if (!(p_new = (*p_buildfn)(p_stepX, &rn_pro))) {
    ErrExit(ErrExit_ASSERT, "rn_mkstepid(provision failure)- out of memory?");
    return NULL;
  }

  p_attr = p_children = p_subdir = p_new;
  /* ...  <stepidX> [p_parent="jobsteps", rtype=RND_JOBSTEPID]
   *            |
   *            |
   *            + "attributes" [p_parent=p_stepX, rtype=RND_ATTRIBUTES]
   */

  p_stepX = rn_cast(p_stepX, RND_JOBSTEPID,
		    p_cp, p_jsti, p_cp->nm,
		    p_jobstepd, p_children, n_children,
		    p_attr, p_subdir);
  if (!p_stepX) {
    ErrExit(ErrExit_ASSERT, "rn_mkjobstep: rn_cast(!p_stepX)");
    return NULL;
  }

  if (n_attr > 0) {
    p_2cp = dup_cp(p_cp);
    if (!derefable_cp(p_2cp)) {
      ErrExit(ErrExit_ASSERT, "rn_mkjobstep: !derefable-cp(p_2cp) - out of mem");
      return NULL;
    }
    p_2cp->nm = rnode_buildtab[RND_ATTRIBUTES].nm;
    p_attrd = rn_cast(p_children, RND_ATTRIBUTES,
		      p_2cp, /*dyntyp*/ p_jsti, p_2cp->nm,
		      p_stepX, /*children*/ NULL, 0,
		      /*attr*/ NULL, /*subdir*/ NULL);
    if (!p_attrd) {
      ErrExit(ErrExit_ASSERT, "rn_mkjobstep: rn_cast_attr_typconv() = !p_attrd");
      return NULL;
    }
  }
  return p_stepX;
}

