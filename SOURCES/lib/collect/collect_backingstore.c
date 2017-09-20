/*
 * routines that manipulate backingstore, and collect parameters from there
 */
#include "rmfs.h"

#if defined(PORTING_TO_SLURMv17)
#error The backing store mechanism should be revisited given that the commercial \
        version of slurm has a more robust slurm data base backend capability.	 \
	Re-evaluate its utility before resurrecting this code.
#else

tri_t claim_BackingStore(config_param_t *p_backingstore_cp, config_param_t *p_pid_cp, claim_t request) {
	return FALSE;
}

tri_t open_BackingStore(config_param_t *p_backingstore_cp, int rdwr) {
	return FALSE;
}

tri_t merge_BackingStore(config_param_t *p_src_cp, merge_bs_source_t bs_mergefrom) {
	return FALSE;
}

tri_t requestWrite_BackingStore(char *mountpoint) {
	return FALSE;
}

tri_t write_modifiedrnode_toBackingStore() {
	return FALSE;
}

tri_t spawn_BackingStorelistener(config_param_t *p_bs_cp) {
	return FALSE;
}
#endif

#if defined(PORTING_TO_SLURMv17)
/*
 * prune_BackingStore()
 *  mark any jobid records (infinite expiration or with an expiration time) as empty
 *   if they don't currently exist
 */
tri_t
prune_Backingstore(config_param_t *p_bs_cp) {
  time_t                      t;
  int                         j, n_j;
  bs_record_t                 p_bs;
  struct backingstore_record *p_bsr;

  if (!p_bs_cp) {
    ErrExit(ErrExit_ASSERT, "prune_BackingStore(): !p_bs_cp");
    return FALSE;
  }
  if (!p_bs_cp->val.pd.is_mmapped) {
    ErrExit(ErrExit_ASSERT, "prune_BackingStore(): backing store is not mapped");
    return FALSE;
  }
  if (!p_bs_cp->val.ue.bs) {
    ErrExit(ErrExit_ASSERT, "prune_BackingStore(): !p_bs_cp->val.ue.bs");
    return FALSE;
  }
  
  time(&t);
  for (j = 0, p_bs = p_bs_base, n_j = 0; j < n_rec; j++, p_bs++) {

    p_bsr = p_bs->_u._r;
    if (p_bsr->htyp.header) {
      continue;
    }
    if (p_bsr->htyp.trailer) {
      continue;
    }
    if (p_bsr->htyp.barrier) {
      continue;
    }
    if (p_bsr->htyp.infinite_expiration) {
      n_j++;
    } else if (p_bsr->node_delta && p_bsr->tm_exp < t) {
      n_j++;
    }
  }
  if (n_j == 0) {
    ErrExit(ErrExit_WARN, "prune_backingstore(): no jobs state to merge");
    return FALSE;
  }

  /*
   * walk p_backingstore_cp->p_bs,
   *  removing any infinite_expiration and node_delta records for inactive jobs
   *  and node_deltas for jobs that are in the past
   *
   * XXXfuture if infinite_expiration space is full, enlarge it
   */ 

  for (j = 0, p_bs = p_bs_base, n_j = 0, p_bs = p_bs->_u._r;
           j < n_rec;
               j++, p_bs++, p_bsr = p_bs->_u._r) {
    
    if (p_bsr->htyp.header) {
      continue;
    }
    if (p_bsr->htyp.trailer) {
      continue;
    }
    if (p_bsr->htyp.barrier) {
      continue;
    }
    
    if (p_bsr->htyp.infinite_expiration || p_bsr->htyp.node_delta) {
      if (rn_jobid_match(p_bsr->job_id, p_bsr->rn_sig)) {
	if (p_bsr->htyp.node_delta && p_bsr->tm_exp < t) {
	  p_bsr->htyp.node_delta = FALSE;
	}
      }
    }
  }

  return TRUE;
}

/*
 * claim_BackingStore()
 * XXXfuture revalidate BackingStore and
 *   claim backingstore for our own
 */

tri_t
claim_BackingStore(config_param_t *p_backingstore_cp,
		   config_param_t *p_pid_cp,
		   claim_t request) {

  bs_hdrtyp_t *p_bs_hdr;

  if (!p_backingstore_cp) {
    ErrExit(ErrExit_ASSERT, "claim_backingstore(): !p_backingstore_cp");
    return FALSE;
  }
  if (!p_pid_cp) {
    ErrExit(ErrExit_ASSERT, "claim_backingstore(): !p_pid_cp");
    return FALSE;
  }
  if (!p_backingstore_cp->val.ue.bs) {
    ErrExit(ErrExit_ASSERT, "claim_backingstore(): backing store is unopened? !val.ue.bs");
    return FALSE;
  }
  if (p_pid_cp->val.ue.pid <= 2) {
    ErrExit(ErrExit_ASSERT, "claim_backingstore(): p_pid_cp->val.ue.pid implausible");
    return FALSE;
  }
  
  /*once we have acquired a lock, do not just return without tidying*/

  /*XXXlock get_bs_rwlock(READ)*/
  p_bs_hdr = p_backingstore_cp->val.ue.bs;
  if (p_bs_hdr->owner == p_pid_cp->val.ue.pid) {
    goto tidy;
  }

  if (request == CL_TEST) {
    goto tidy;
  }
  
  /*XXXlock get_bs_rwlock(WRITE)*/
  if (p_backingstore_cp->val.is_mmapped) {
    p_bs_hdr->owner = p_pid_cp->val.ue.pid;

  } else {
    ErrExit(ErrExit_ASSERT, "claim_backingstore(): bs !is_mapped");
    goto tidy;
  }

tidy:
  /*XXXlock release_bs_rwlock()*/

  if (request != CL_TEST) {
    prune_BackingStore();
  }

  return TRUE;
}

/*
 * open a socket so that our sucessors may contact us for current state
 *  this is a fallback for rendezvous through the file system
 *
 *  XXXFUTURE: ONC RPC, XDR especially
 */
void
spawn_exporter(config_param_t *p_bs_cp) {

  if (!p_bs_cp) {
    ErrExit(ErrExit_ASSERT, "spawn_exporter: !p_bs_cp");
    return FALSE;
  }

  if (snprintf(export_cmd, _POSIX_PATH_MAX-1, "/usr/bin/exportfs -i %s", p_bs_cp->val.ue.pathnm) <= 0) {
    ErrExit(ErrExit_ASSERT, "spawn_exporter: snprintf");
    return FALSE;
  }
  
  if ((cpid = vfork()) < 0) {
    ErrExit(ErrExit_ASSERT, "spawn_exporter: vfork");
    return FALSE;

  } else if (cpid != 0) {
    execlp("/usr/sbin/exportfs", "exportfs", "-i", p_bs_cp->pathnm, NULL);
    ErrExit(ErrExit_ASSERT, "spawn_exporter: execlp(\"exportfs\")");
    exit (-1);
  }
  return;
}

void
bs_processreq(int req_fd, config_param_t *p_bs_cp) {

  char                        req[_POSIX_PATH_MAX];
  FILE                       *str;
  bs_hdr_t                   *p_bsh;
  bs_record_t                *p_bs, *p_bs_base;
  struct backingstore_record *p_bsr;
  int                         n;

  if (!p_bs_cp) {
    ErrExit(ErrExit_ASSERT, "bs_processreq: !p_bs_cp()");
    exit (-1);
  }
  if (memset(req, 0, _POSIX_PATH_MAX) != req) {
    ErrExit(ErrExit_ASSERT, "bs_processreq: !memset()");
    exit (-1);
  }
  if (!(str = fdopen(req_fd, "r+"))) {
    ErrExit(ErrExit_ASSERT, "bs_processreq: !fdopen()");
    exit (-1);
  }
  if (fscanf(str, "%s", &req) <= 0) {
    ErrExit(ErrExit_ASSERT, "bs_processreq: !fscanf()");
    exit (-1);
  }
  
  if (strcmp(req, "n_rec") == 0) {
    if (fwrite(&p_bs_cp->val.ue.i, sizeof(int), 1, str) < 1) {
      ErrExit(ErrExit_ASSERT, "bs_processreq: !fwrite(\"n_rec\"))");
      exit (-1);
    }
    
  } else if (strcmp(req, "dump") == 0) {
    if (!p_bs_cp->val.ue.bs) {
      ErrExit(ErrExit_ASSERT, "bs_processreq: dump, but !p_bs_cp->val.ue.bs");
      exit (-1);
    }
    if (fwrite(p_bs, sizeof(bs_record_t), p_bs_cp->val.ue.bs->_u._r.size, str) != p_bs_cp->val.ue.bs->_u._r.size) {
      ErrExit(ErrExit_ASSERT, "bs_processreq: dump, fwrite() => !p_bs_cp->val.ue.bs->_u._r.size");
      exit (-1);
    }
  }
  exit (0);
}

tri_t
bs_handlelistenrequests(config_param_t *p_bs_cp) {
  config_param_t    *p_mp;
  int                sfd;
  struct sockaddr_in listen_s    = { 0 };
  struct sockaddr_in requestor_s = { 0 };
  char               req_buf[_POSIX_PATH_MAX];
  FILE              *req_str;
  pid_t              child_handler;

  
  if ((sfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    ErrExit(ErrExit_ASSERT, "bs_handlelistenrequests: !socket()");
    return FALSE;
  }

  listen_s.sin_family      = AF_INET;
  listen_s.sin_addr.s_addr = INADDR_ANY;
  listen_s.sin_port        = htons(BS_PORT);

  if (bind(sfd, (struct sockaddr *) &listen_s, sizeof(listen_s)) < 0) {
    ErrExit(ErrExit_ASSERT, "bs_handlelistenrequests: !bind()");
    return FALSE;
  }
  if (listen(sfd, 5) < 0) {
    ErrExit(ErrExit_ASSERT, "bs_handlelistenrequests: !listen()");
    return FALSE;
  }
  req_len = sizeof(requestor_s);

  for ( ;; ) {
    if ((req_fd = accept(sfd, (struct sockaddr *) &requestor_s, &req_len)) < 0) {
      ErrExit(ErrExit_ASSERT, "bs_handlelistenrequests: !accept()");
      continue;
    }
    if ((child_handler = fork()) < 0) {
      ErrExit(ErrExit_ASSERT, "bs_handlelistenrequests: !fork()");
      continue;
    } else if (child_handler == 0) {
      close(sfd);
      bs_processreq(req_fd, p_bs_cp);
      exit(0);
    } else if (child_handler > 0) {
      close(req_fd);
    }
  }
  return FALSE;
}
/*
 *
 * spawn_..()
 *
 * XXXuseclone instead of fork() and call export_fn() or listen_fn()
 * XXXuseclone merge spawn_{exporter, listener}
 */

void
spawn_listener(config_param_t *p_bs_cp) {

  if (!p_bs_cp) {
    ErrExit(ErrExit_ASSERT, "spawn_listener: !p_bs_cp");
    return FALSE;
  }

  /*
   * fork a child to respond to requests from others & export the backingstore
   * XXXclone
   */
  if ((lpid = vfork()) < 0) {
    ErrExit(ErrExit_ASSERT, "spawn_listener: vfork");
    return FALSE;
    
  } else if (lpid != 0) {
    bs_handlelistenrequests(p_bs_cp);
    ErrExit(ErrExit_ASSERT, "spawn_listener: bs_handlelistenrequests returned");
    exit (-1);
  }
  return TRUE;
}

tri_t
spawn_BackingStorelistener(config_param_t *p_bs_cp){
  rn_param_t *p_rn_paramtab;
  rnode_t    *p_fsroot;
  char        cbuf[_POSIX_PATH_MAX];

  spawn_exporter(p_bs_cp);
  spawn_listener(p_bs_cp);
  
  p_rn_paramtab = get_rn_params(/*needlock*/ FALSE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_ASSERT, "open_BackingStore(): !p_rn_paramtab");
    return FALSE;
  }
  

  return TRUE;
}

/*
 * merge_BackingStore(bs_src)
 *  reads from bs_src, and obtains records corresponding to current jobs
 *  if these records match a current job (signatures are equal)
 *   collect context from BackingStore and put into current state
 *    can be called as bs_mergefrom = MS_PREDECESSOR
 *                                    MS_BACKINGSTORE
 *    p_src_cp describes the source
 *
 */

tri_t
merge_BackingStore(config_param_t *p_src_cp,
		   merge_bs_source_t bs_mergefrom) {
  int                         n_rec, n_j, h_ctx, h;
  tri_t                       rc = FALSE;
  time_t                      t;
  config_param_t             *p_rnodepool_cp;
  bs_record_t                *p_bs, *p_bs_pred, p_bs_base;
  bs_hdr_t                   *p_bsh;
  struct backingstore_record *p_bsr;

  if (bs_mergefrom == MS_PREDECESSOR) {
    if (!p_src_cp->pd.fstr) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) !fstr");
      return FALSE;
    }
    if (fwrite("n_rec", 1, internal_strlen("n_rec"), p_src_cp->pd.fstr) <= 0) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) !fwrite(fstr, \"n_rec\")");
      return FALSE;
    }
    if (fread(&n_rec, sizeof(int), 1, p_src_cp->pd.fstr) <= 0) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) !fread(fstr, \"n_rec\")");
      return FALSE;
    }
    if (!(p_rnodepool_cp = getconfig_fromnm("rnodepool"))) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) !p_rnodepool_cp");
      return FALSE;
    }
    if (n_rec <= 0) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) n_rec <= 0");
      return FALSE;
    }
    if ((p_rnodepool_cp->val.ue.l - n_rec) > (p_rnodepool_cp->val.ue.l/10)) {  /*  heuristic */
      ErrExit(ErrExit_WARN, "merge_backingstore(PREDECESSOR): pred backingstore is >10% different from rnodepoolsize");
      /*FALLTHROUGH*/
    }
    if (!(p_bs_pred = calloc(n_rec+1, sizeof(bs_record_t)))) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) !p_bs_pred");
      return FALSE;
    }
    if (fwrite("dump", 1, internal_strlen("dump"), p_src_cp->pd.fstr) <= 0) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) !fwrite(fstr, \"dump\")");
      return FALSE;
    }
    if (fread(p_bs_pred, sizeof(bs_record_t), n_rec, p_src_cp->pd.fstr) != (n_rec * sizeof(bs_record_t))) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) !fread");
      return FALSE;
    }
    p_bs_base = p_bs_pred;

  } else if (MS_BACKINGSTORE) {
    if (!p_src_cp->per_src.rmfs.backingstore) {
      ErrExit(ErrExit_ASSERT, "merge_backingstore(BACKINGSTORE) but !p_src_cp->...backingstore");
      return FALSE;
    }
    p_bs_base = p_src_cp->val.ue.bs;
  }
  if (!ck_bs(p_bs)) {
    ErrExit(ErrExit_ASSERT, "merge_backingstore(PREDECESSOR) !ck_bs()");
    return FALSE;
  }
  time(&t);
  bsh = p_bs_base;
  n_j = bsh->size;

  if (!(p_jn = calloc(n_j, sizeof(jnode_t)))) {
    ErrExit(ErrExit_ASSERT, "merge_backingstore() calloc(job node)");
    return FALSE;
  }
  
  h_ctx = djb_strtohash(CTX_XATTR_NM);

  /*
   * for all jobids in backing store
   *  if they are unexpired
   *   and the backing store record for this job contains an xattr of a CTX_ATTR_NM
   *    store the value from the backing store into the rnode
   */
  for (j = 0, p_bsr = p_bs = p_bs_base, n_j = 0;
           !p_bsr->htyp.trailer;
               j++, p_bsr = (p_bs++)->_u._r) {
    
    if (p_bsr->htyp.infinite_expiration || p_bsr->htyp.node_delta) {

      if (rn_jobid_matchsig(p_bsr->job_id, p_bsr->rn_sig)) {
	if (p_bsr->htyp.infinite_expiration ||
	    (p_bsr->htyp.node_delta && p_bsr->tm_exp >= t)
	   ) {
	  h = djb_strtohash(p_bsr->xattr.nm);
	  
	  if (h_ctx == h) {
	    if (!rn_jobid_addxattr(p_bsr->job_id, p_bsr->xattr.nm, p_bsr->xattr.ctx)) { /*rn_ctljobid.c*/
	      ErrExit(ErrExit_ASSERT, "merge_backingstore() rn_jobid_addxattr(jobid)");
	      return FALSE;
	    }
	    rc = TRUE;
	  }
	}
      }
    }
  }
  return rc;
}

/*
 * open_BackingStore()
 *  - opens BackingStore
 *   if rdwr == TRUE
 *    - creates it if necessary
 *      - setting its size to a reasonable size for this cluster
 *  - if not in controller mode rdwr = FALSE
 *    opens backing store in read-only mode
 */
tri_t
open_BackingStore(config_param_t *p_backingstore_cp, int rdwr) {
  rn_param_t     *p_rn_paramtab;
  int             needToCreate  = 0;
  int             exists        = 0;
  int             i, n_inf, bs_fd;
  FILE           *bs_fstr;
  unsigned long   bs_size;
  struct stat     st;
  bs_record_t    *p_bs, *p_hdr, *p_barrier, *p_trailer;
  bs_record_t     bs_rec, bs_zero;
  config_param_t *p_pid_cp, *p_hostname_cp;
  time_t          t;

  if (!p_backingstore_cp) {
    ErrExit(ErrExit_ASSERT, "open_BackingStore(): !p_backingstore_cp");
    return FALSE;
  }

  p_rn_paramtab = get_rn_params(/*needlock*/ FALSE);
  if (!p_rn_paramtab) {
    ErrExit(ErrExit_ASSERT, "open_BackingStore(): !p_rn_paramtab");
    return FALSE;
  }
  bs_size = p_rn_paramtab->rn_maxalloc;

  if (!(p_pid_cp = getconfig_fromnm("pid"))) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !p_pid_cp");
      return FALSE;
  }

  if (!(p_hostname_cp = getconfig_fromnm("hostname"))) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !p_hostname_cp");
      return FALSE;
  }

  if (access(p_backingstore_cp->val.pathnm) < 0) {
    needToCreate++;
  } else {
    exists++;
  }

  if (exists) {
    if ((bs_fd = shm_open(p_cp->val.fullstr, rdwr? O_RDWR: O_RDONLY )) < 0) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !SHM_OPEN(BackingStore)");
      return FALSE;
    }
    if (!(p_bs = mmap64(/*addr*/ NULL, bs_size, rdwr? PROT_READ|PROT_WRITE: PROT_READ,
			rdwr? MAP_SHARED|MAP_LOCKED|MAP_POPULATE:MAP_PRIVATE|MAP_LOCKED|MAP_POPULATE,
			bs_fd, /*offset*/ 0))) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !mmap(existing BackingStore)");
      return FALSE;
    }

    if (rdwr) {
      if (p_bs->size != bs_size) {
	if (p_bs->size < bs_size) {
	  if (ftruncate(bs_fd, bs_size*sizeof(bs_record_t)) < 0) {
	    ErrExit(ErrExit_ASSERT, "open_BackingStore(): !ftruncate(BackingStore)");
	    return FALSE;
	  } else {
	    ErrExit(ErrExit_WARN, "open_BackingStore(): enlarged BackingStore");
	  }
	}
      }
    }
    p_hdr     = p_bs;
    p_trailer = p_bs + bs_size - 1;
  }
  
  if (needToCreate) {
    if ((bs_fd = shm_open(p_cp->val.fullstr, O_RDWR|O_CREAT)) < 0) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !SHM_OPEN(BackingStore, O_CREAT)");
      return FALSE;
    }
    if (ftruncate(bs_fd, bs_size*sizeof(bs_record_t)) < 0) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !ftruncate(BackingStore)");
      return FALSE;
    }
    if (!(p_bs = mmap64(/*addr*/ NULL, bs_size, PROT_READ|PROT_WRITE,
			MAP_SHARED|MAP_LOCKED|MAP_POPULATE, bs_fd, /*offset*/ 0))) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !mmap(new BackingStore)");
      return FALSE;
    }

    if (memset(p_bs, 0, bs_size*sizeof(bs_record_t)) != p_bs) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !memset(0, BackingStore)");
      return FALSE;
    }
    if (time(&t) < 0) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !time()");
      return FALSE;
    }
 
    /* write out header and trailers */
    p_hdr     = p_bs;    
    p_trailer = p_bs + bs_size - 1;

    /*XXXreadability bs_cast(...);*/
    bs_rec.htyp.header           = TRUE;
    bs_rec.time_stamp            = t;
    bs_rec.n_infinite_expiration = n_inf = BS_MIN_INF_EXP_SPACE; /*XXX heuristic*/
    bs_rec.size                  = bs_size;
    bs_rec.owner                 = p_pid_cp->val.ue.pid;
    bs_rec.last_written          = 0;
    bs_rec.format_version        = BS_FORMAT_VERSION;


    if (strlcpy(bs_rec.host, p_hostname_cp->val.charstr, p_hdr->host, p_hostname_cp->val.size) != p_hostname_cp->val.size) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !strlcpy(HEADER, hostname)");
      return FALSE;
    }
    *p_hdr = bs_rec;
    
    bs_rec.htyp.header  = FALSE;
    bs_rec.htyp.trailer = TRUE;
    *p_trailer          = bs_rec;

    if (memset(bs_zero, 0, sizeof(bs_record_t)) != p_bs) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !memset(0, BackingStore)");
      return FALSE;
    }
    /*
     * barrier between sections have the bits set for the sections separated by the barrier
     */
    bs_rec = bs_zero;
    bs_rec.htyp.barrier             = TRUE;
    bs_rec.htyp.header              = TRUE;
    bs_rec.htyp.infinite_expiration = TRUE;
    
    p_barrier = p_hdr + 1;
    *p_barrier = bs_rec;

    /* n_inf infinite_expiration job records */
    bs_rec = bs_zero;
    
    bs_rec.htyp.infinite_expiration = TRUE;
    bs_rec.time_stamp               = t;
    bs_rec.job_id                   = NO_VAL; /*slurm.h*/
    bs_rec.rn_sig                   = 0;

    for (i = 0, p_bs = p_barrier + 1; i < n_inf; i++, p_bs++) {
      *p_bs = bs_rec;
    }
    /* barrier between infinite expiration and node delta records */
    bs_rec = bs_zero;
    bs_rec.htyp.barrier             = TRUE;
    bs_rec.htyp.infinite_expiration = TRUE;
    bs_rec.htyp.node_delta          = TRUE;
    p_barrier  = p_bs;      /* after the n_inf block above */
    *p_barrier = bs_rec;

    /* barrier between node deltas and trailer records */
    bs_rec = bs_zero;
    bs_rec.htyp.barrier    = TRUE;
    bs_rec.htyp.node_delta = TRUE;
    bs_rec.htyp.trailer    = TRUE;
    p_barrier  = p_trailer - 1; /* EOF-2 */
    *p_barrier = bs_rec;
    
    if (msync(p_hdr, bs_size*sizeof(bs_record_t), MS_SYNC|MS_INVALIDATE) < 0) {
      ErrExit(ErrExit_ASSERT, "open_BackingStore(): !msync(initial format)");
      return FALSE;
    }
    p_bs = p_hdr; /* same location as if pre-existing */
  }
  
  p_backingstore_cp->val.ue.bs     = p_hdr;
  p_backingstore_cp->pd.is_filestr = FALSE;
  p_backingstore_cp->pd.is_mmapped = TRUE;

  return p_hdr;
}

/*
 * requestWrite_BackingStore() 
 * - use the fs interface so that it can handle requests to both ourselves and predecessors
 * - authorization is handled at the fs layer
 */
tri_t
requestWrite_BackingStore(char *mountpoint) {
  int   fd;
  FILE *fstr;
  char  pbuf[_POSIX_PATH_MAX];
  
  if (!mountpoint) {
    ErrExit(ErrExit_ASSERT, "requestWrite_BackingStore: !mountpoint");
    return FALSE;
  }
  /*
   * request a write to the backing store, given a mountpoint
   */
  if (snprintf(pbuf, _POSIX_PATH_MAX-1, "%s/control/write", mountpoint) < 0) {
    ErrExit(ErrExit_ASSERT, "requestWrite_BackingStore: snprintf(pbuf/control/write)");
    return FALSE;
  }
  if ((fd = open(pbuf, W_OK)) < 0) {
    ErrExit(ErrExit_ASSERT, "requestWrite_BackingStore: open(pbuf/control/write) < 0");
    return FALSE;
  }
  if (!(fstr = fdopen(fd, "w+"))) {
    ErrExit(ErrExit_ASSERT, "requestWrite_BackingStore: fdopen(pbuf/control/write) < 0");
    return FALSE;
  }
  if (fprintf(fstr, "write\n") <= 0) {
    ErrExit(ErrExit_ASSERT, "requestWrite_BackingStore: fprintf(pbuf/control/write) < 0");
    return FALSE;
  }
  fclose(fstr); /* => close(fd) */
  return TRUE;
}
#endif /*PORTING_TO_SLURMv17*/
