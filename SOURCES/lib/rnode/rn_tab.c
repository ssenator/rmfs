#include "rmfs.h"

/*
 * initialization tables
 */

/*
 * the buildtab contains embryonic forms of the actual rnodes
 *  new rnodes copy some of these values from the keys in this table
 *  for example, the names match those of the relevant config_param_t
 *  ("MountPoint", "ClusterName") if they are associated
 *
 * a name in single angle brackets ("<name>") indicates a variable, so
 * "<partition>" becomes an actual partition name.
 *
 * "[[guard]]" indicates an allocated, but unused node.
 */

extern rnode_t  *rn_provision(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkfsroot(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkcname(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkpartd(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkpart(rnode_t *, rnode_t *);
extern rnode_t  *rn_mknoded(rnode_t *, rnode_t *);
extern rnode_t  *rn_mknode(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkjobd(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkjobid(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkjobstepd(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkjobstep(rnode_t *, rnode_t *);
extern rnode_t  *rn_mknstate(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkallocjob(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkattrd(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkattr(rnode_t *, rnode_t *);
extern rnode_t  *rn_mkctld(rnode_t *, rnode_t *);
extern rnode_t  *rn_raze(rnode_t *, rnode_t *);

extern tri_t     rn_ctlroot_check(rnode_t *, config_param_t *);
extern tri_t     rn_ctlroot_effluviate(rnode_t *, config_param_t *); /*Q(highfalutin'(dump))="effluviate"*/
extern tri_t     rn_ctlroot_umount(rnode_t *, config_param_t *);
extern tri_t     rn_ctlroot_write(rnode_t *, config_param_t *);

extern tri_t     rn_ctljobid_sign(rnode_t *, config_param_t *);
extern tri_t     rn_ctljobid_read(rnode_t *, config_param_t *);
extern tri_t     rn_ctljobid_write(rnode_t *, config_param_t *);

extern tri_t     is_fsroot_attr(config_param_t *);
extern tri_t     is_cname_attr(config_param_t *);

rnode_t rnode_buildtab[] = {
  { .rtype=RN_PROVISION,   .nm="provision",       .buildfn=rn_provision              },
  { .rtype=RN_GUARD,       .nm="[[guard]]",       .buildfn=NULL                      },
  { .rtype=RND_ROOT,       .nm="MountPoint",      .buildfn=rn_mkfsroot,   .is.dir=1,
       .ctl={{.rtype=RNF_KNOB, .nm="check",       .fn=rn_ctlroot_umount,             },
	     {.rtype=RNF_KNOB, .nm="effluviate",  .fn=rn_ctlroot_effluviate          },     
	     {.rtype=RNF_KNOB, .nm="umount",      .fn=rn_ctlroot_check,              },
	     {.rtype=RNF_KNOB, .nm="write",       .fn=rn_ctlroot_write               },
	     {.nm=NULL}                                                       }/*ctl*/,
     .maybe.readable = TRUE,
     .attr_desc = { .table=slurmfs_config, .is_mine=is_fsroot_attr }                  },

  { .rtype=RND_CLUSTER,    .nm="ClusterName",     .buildfn=rn_mkcname,    .is.dir=1,
     .maybe.readable = TRUE,
     .attr_desc = { .table=slurmfs_config, .is_mine=is_cname_attr }                  },

  { .rtype=RND_PARTS,      .nm="partitions",      .buildfn=rn_mkpartd,    .is.dir=1,
     .maybe.readable = TRUE                                                          },
  { .rtype=RND_PARTNAME,   .nm="<partition>",     .buildfn=rn_mkpart,     .is.dir=1,
     .maybe.readable = TRUE,                                                          
     .attr_desc.table=partinfodesc_tab                                               },

  { .rtype=RND_NODES,      .nm="nodes",           .buildfn=rn_mknoded,    .is.dir=1,
    .maybe.readable = TRUE                                                           },
  { .rtype=RND_NODENAME,   .nm="<node>",          .buildfn=rn_mknode,     .is.dir=1,
      .maybe.readable = TRUE,                                                         
      .attr_desc.table=nodeinfodesc_tab                                              },

  { .rtype=RND_JOBS,       .nm="jobs",            .buildfn=rn_mkjobd,     .is.dir=1,
      .maybe.readable = TRUE                                                         },
  { .rtype=RND_JOBID,      .nm="<jobid>",         .buildfn=rn_mkjobid,    .is.dir=1,
    .ctl={{.rtype=RNF_SIGNATURE, .nm="sign",        .fn=rn_ctljobid_sign  },
          {.rtype=RNF_KNOB,      .nm="read",        .fn=rn_ctljobid_read  },
          {.rtype=RNF_KNOB,      .nm="write",       .fn=rn_ctljobid_write },
          {.nm=NULL}                                                          }/*ctl*/,
      .maybe.readable = TRUE,
      .attr_desc.table=jobinfodesc_tab                                               },

  { .rtype=RND_JOBSTEPS,   .nm="jobsteps",        .buildfn=rn_mkjobstepd, .is.dir=1,
      .maybe.readable = TRUE                                                         },
  { .rtype=RND_JOBSTEPID,  .nm="<jobstepid>",     .buildfn=rn_mkjobstep,  .is.dir=1,
      .maybe.readable = TRUE,                                                        
      .attr_desc.table=stepinfodesc_tab                                              },

  { .rtype=RND_NODESTATE,  .nm="state",           .buildfn=rn_mknstate,   .is.dir=1,
      .maybe.readable = TRUE                                                         },                                                        
  { .rtype=RNF_ALLOCJOB,   .nm="<jobidref>",      .buildfn=rn_mkallocjob,
      .maybe.readable = TRUE,                                                        
#ifdef _XXX_symlink_ALLOCJOB
    /*XXXALTIMP RND_ALLOCJOB & is_bindmount=1 to point to RND_JOBID*/     .is.link=1 }, 
#else    
                                                                          .is.file=1 },
#endif  
  { .rtype=RND_ATTRIBUTES, .nm="attributes",      .buildfn=rn_mkattrd,    .is.dir=1,
      .maybe.readable=1                                                              },
  { .rtype=RNF_ATTRIBUTE,  .nm="<attribute>",     .buildfn=rn_mkattr,     .is.file=1,
      .maybe.readable=1                                                              },

  { .rtype=RND_CONTROL,    .nm="control",         .buildfn=rn_mkctld,     .is.dir=1,
      .maybe.controllable=1, .maybe.readable=1                                       },
  { .rtype=RNF_CONTEXT,    .nm="context",         .buildfn=rn_mkattr,     .is.file=1,
      .maybe.writable=1, .maybe.readable=1                                           },
  { .rtype=RNF_KNOB,       .nm="<knob>",          .buildfn=rn_mkattr,     .is.file=1,
      .maybe.execable=1, .maybe.controllable=1, .maybe.readable=1                    },
#ifdef SLURM_1905
  { .rtype=RNF_SIGNATURE,  .nm="signature",       .buildfn=rn_mkattr,   .is.file=1,
      .maybe.readable=1                                                              },
#endif
  
  { .rtype=RND_PARAMS,     .nm="rn_parameters", .buildfn=NULL/*XXXFUTURE*/, .is.dir=1,
      .maybe.readable=1                                                              },

  { .rtype=RN_LAST,        .nm="raze",            .buildfn=rn_raze                   }
};

/*
 * conversion table from configuration parameter types to rnode types
 *
 * although most map to plain attributes, tabilifying is more flexible
 * for future arbitrary types
 *
 * modifying this requires modifying rmfs_types.h and the name table,
 * ptyp2pname_tab
 * XXX split into (3, 4, 6?,  separate tables: opaque, alpha, fsvis,
 * XXX                                         host, numeric, special
 */

pt2rn_t p2r_typ_convtab[] = {
  { .ptyp=PTYP_NONE,           .rtyp=RN_NONE       },
  { .ptyp=PTYP_OPAQUE,         .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_ALPHANUM,       .rtyp=RNF_ATTRIBUTE },  
  { .ptyp=PTYP_ALPHA,          .rtyp=RNF_ATTRIBUTE },  
  { .ptyp=PTYP_NUMERICHAR,     .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_ALPHA_P2P,      .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_XATTR,          .rtyp=RNF_ATTRIBUTE }, /*XXX */
  { .ptyp=PTYP_PATH,           .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_FILE,           .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_FILEXIST,       .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_SYM,            .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_DIREXIST,       .rtyp=RNF_ATTRIBUTE }, /*XXX */
  { .ptyp=PTYP_DIR,            .rtyp=RNF_ATTRIBUTE }, /*XXX */
  { .ptyp=PTYP_HOSTNAME,       .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_NUMERIC,        .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_NUMERICTIME,    .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_NUMTIM_SECS,    .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_NUMSIGNED,      .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_UNSIGNED_INT,   .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_UNSIGNED_INT16, .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_UNSIGNED_INT32, .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_UID,            .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_PID,            .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_TRILENE,        .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_BOOLEAN,        .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_SIGNATURE,      .rtyp=RNF_SIGNATURE },
  { .ptyp=PTYP_CONTEXT,        .rtyp=RNF_CONTEXT   },
  { .ptyp=PTYP_NONE }
};

pt2rn_t p2r_typ_rm_convtab[] = {
  { .ptyp=PTYP_CLUSTER,      .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_CNTRLMACH,    .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_PARTITION,    .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_NODE,         .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_JOB,          .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_STEP,         .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_ALLOCJOB,     .rtyp=RNF_ALLOCJOB  }, /* XXX */
  { .ptyp=PTYP_NODESTATE,    .rtyp=RNF_ATTRIBUTE }, /* XXX */
  { .ptyp=PTYP_SLURMVERSION, .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_SLURMUID,     .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_SLURMTMOUT,   .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_SPANKENV,     .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_SPANKENVSIZE, .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_JSUB_PLUGIN,  .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_GRES_PLUGIN,  .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_SLURMUNAME,   .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_SLURMDUNAME,  .rtyp=RNF_ATTRIBUTE },
  { .ptyp=PTYP_NONE }
};
