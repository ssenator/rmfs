
#include "rmfs.h"

/*
 * routines to communicate to, from and with predecessors
 */


/*
 * predecessor_alive()
 *  detect predecessor is still alive
 *
 * search for pidfile and mountpointfile in well-known locations
 * determine if that pid is live and that mountpoint contains a mount instance
 */

tri_t
predecessor_alive(config_param_t *p_pid_cp,
		  config_param_t *p_mp_cp) {

  config_param_t *p_pidfile_cp, *p_mpfile_cp;
  pid_t           pred_pid;
  int             pred_ctlpid_fd;
  FILE           *pred_ctlpid_fstr;
  struct stat     st;
  char            pbuf[_POSIX_PATH_MAX];


  p_pidfile_cp = getconfig_fromnm("pidfile");
  p_mpfile_cp  = getconfig_fromnm("mountpointfile");
  if (!p_pidfile_cp) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: !getconfig_fromnm(\"pidfile\")");
    return FALSE;
  }
  if (!p_mpfile_cp) {
    ErrExit(ErrExit_INTERNAL, "predecessor_alive: !getconfig_fromnm(\"mountpointfile\")");
    return FALSE;
  }
  
  if (!p_pidfile_cp->val.pd.fullpath) {
    ErrExit(ErrExit_INTERNAL, "predecessor_alive: !pidfile->cp->val.pd.fullpath");
    return FALSE;
  }
  if (!p_mpfile_cp->val.pd.fullpath) {
    ErrExit(ErrExit_INTERNAL, "predecessor_alive: !mpfile->cp->val.pd.fullpath");
    return FALSE;
  }

  if (stat(p_mpfile_cp->val.pd.fullpath, &st) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: !mpfile->cp->val.pd.fullpath");
    return FALSE;
  }

  /*if (!path in p_mpfile does not refer to a fuse mount) => FALSE*/

  if (memset(pbuf, 0, _POSIX_PATH_MAX) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: memset(pbuf) mpfile");
    return FALSE;
  }

  if (!p_mpfile_cp->val.pd.is_filestr) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: mpfile !is_filestr");
    return FALSE;
  }
  if (fscanf(p_mpfile_cp->val.pd.fstr, "%s", pbuf) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: memset(pbuf) mpfile");
    return FALSE;
  }
  
  if (stat(pbuf, &st) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: !stat(mpfile->pbuf)");
    return FALSE;
  }

  if (!S_ISDIR(st.st_mode)) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: !ISDIR(mpfile->pbuf)");
    return FALSE;
  }

  if (snprintf(pbuf, _POSIX_PATH_MAX-1, "%s/attributes/pid", pbuf) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: snprintf(mpfile->pbuf/attributes/pid)");
    return FALSE;
  }
  
  if (access(pbuf, R_OK) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: access(mpfile->cp->val.pd.fullpath/attributes/pid)");
    return FALSE;
  }

  if ((pred_ctlpid_fd = open(pbuf, R_OK)) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: open(mpfile->cp->val.pd.fullpath/attributes/pid) < 0");
    return FALSE;
  }
  if ((pred_ctlpid_fstr = fdopen(pred_ctlpid_fd, "r")) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: fdopen(mpfile->cp->val.pd.fullpath/attributes/pid) < 0");
    return FALSE;
  }

  if (fscanf(pred_ctlpid_fstr, "%ld", (long int *) &pred_pid) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: fscanf(mpfile->cp->val.pd.fullpath/attributes/pid) < 0");
    return FALSE;
  }
  fclose (pred_ctlpid_fstr); /* closes the underlying fd, pred_ctlpid_fd */
  pred_ctlpid_fd = -1;

  if (pred_pid <= 1) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: predecessor/attributes/pid <= 1");
    return FALSE;
  }
  
  if (memset(pbuf, 0, _POSIX_PATH_MAX) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: memset(pbuf-2) mpfile");
    return FALSE;
  }
  if (fscanf(p_mpfile_cp->val.pd.fstr, "%s", pbuf) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: memset(pbuf) mpfile");
    return FALSE;
  }

  if (kill(pred_pid, 0) == 0) {
    return FALSE;
  }

  requestWrite_BackingStore(pbuf); /*collect_backingstore.c, but => rn_ctlroot.c*/

  p_pid_cp = p_pidfile_cp;
  p_mp_cp  = p_mpfile_cp;

  return TRUE;
}

/*
 * claim_varrun()
 *  leave our breadcrumbs in /var/run: pidfile and mountpointfile
 *  clean up predecessors breadcrumbs
 */
tri_t
claim_varrun(config_param_t *p_pred_pid_cp, config_param_t *p_pred_mp_cp) {
  ErrExit(ErrExit_INTERNAL, "claim_varrun() ENOXIST");
  return FALSE;
}


tri_t
predecessor_umount(config_param_t *p_pred_pid_cp, config_param_t *p_pred_mp_cp) {
  char pbuf[_POSIX_PATH_MAX];
  int  pred_ctlpid_fd;
  
  if (!p_pred_pid_cp) {
    ErrExit(ErrExit_ASSERT, "predecessor_umount: !p_pred_pid_cp");
    return FALSE;
  }
  if (!p_pred_mp_cp) {
    ErrExit(ErrExit_ASSERT, "predecessor_umount: !p_pred_mp_cp");
    return FALSE;
  }
  
  if (memset(pbuf, 0, _POSIX_PATH_MAX) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_umount: memset(pbuf) mpfile");
    return FALSE;
  }

  if (snprintf(pbuf, _POSIX_PATH_MAX-1, "%s/control/umount", pbuf) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_umount: snprintf(mpfile->pbuf/control/umount)");
    return FALSE;
  }
  
  if (access(pbuf, R_OK) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_umount: access(mpfile->cp->val.pd.fullpath/control/umount)");
    return FALSE;
  }
  
  if ((pred_ctlpid_fd = open(pbuf, R_OK)) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_umount: open(mpfile->cp->val.pd.fullpath/control/umount) < 0");
    return FALSE;
  }

  if (write(pred_ctlpid_fd, "umount", internal_strlen("umount")) < 0) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: write(mpfile->cp->val.pd.fullpath/control/umount, ) < 0");
    return FALSE;
  }

  if (close(pred_ctlpid_fd)) {
    ErrExit(ErrExit_ASSERT, "predecessor_alive: close(mpfile->cp->val.pd.fullpath/control/umount) < 0");
    return FALSE;
  }
  return TRUE;
}


/*
 * collects state from previously running instance, if it is still running
 *  not all state transfers from previous instances; only the dirty rnodes
 */

tri_t
collect_predstate(void) {
  config_param_t *p_bs_cp       = NULL;
  config_param_t *p_pred_pid_cp = NULL;
  config_param_t *p_pred_mp_cp  = NULL;
  config_param_t *p_pid_cp      = NULL;
  int             merged;
  tri_t           ctlr;

  ctlr = isCtlr();
  if (ctlr) {
    if (open_BackingStore(p_bs_cp, /*rdwr*/ ctlr) == FALSE) {
#if defined(PORTING_TO_SLURMv17)	    
      ErrExit(ErrExit_ASSERT, "collect_predstate: !open_BackingStore(isCtlr)");
#else      
      ErrExit(ErrExit_WARN, "collect_predstate: !open_BackingStore(isCtlr)");
#endif      
      return FALSE;
    }
    if (predecessor_alive(p_pred_pid_cp, p_pred_mp_cp)) { /*sets pred_mp_cp, pred_pid_cp*/
      merged = 0;

      if (merge_BackingStore(p_pred_mp_cp, MS_PREDECESSOR) == FALSE) {
	ErrExit(ErrExit_ASSERT, "merge_state[Controller]: !PREDECESSOR");
	/*FALLTHROUGH*/
      } else {
	merged++;
      }
      if (merge_BackingStore(p_bs_cp, MS_BACKINGSTORE) == FALSE) {
	ErrExit(ErrExit_ASSERT, "merge_state[Controller]: !BACKINGSTORE");
	/*FALLTHROUGH*/
      } else {
	merged++;
      }
      if (merged == 0) {
	ErrExit(ErrExit_ASSERT, "collect_predstate: !merge_state()");
	return FALSE;
      }
      if (predecessor_umount(p_pred_pid_cp, p_pred_mp_cp) == FALSE) {
	ErrExit(ErrExit_STUCK, "collect_predstate: !predecessor_umount()");
	return FALSE;
      }
      if (predecessor_alive(p_pred_pid_cp, p_pred_mp_cp) == FALSE) {
	ErrExit(ErrExit_STUCK, "collect_predstate: predecessor refusing to relinquish");
	return FALSE;
      }
    }

    if (!(p_pid_cp = getconfig_fromnm("pid"))) {
	ErrExit(ErrExit_STUCK, "collect_predstate: NULL p_pid_cp");
	return FALSE;
    }
    if (claim_BackingStore(p_bs_cp, p_pid_cp, CL_CLAIM) == FALSE) {
      ErrExit(ErrExit_STUCK, "collect_predstate: !claim_BackingStore()");
      return FALSE;
    }
    if (spawn_BackingStorelistener(p_bs_cp) == FALSE) {
      ErrExit(ErrExit_STUCK, "collect_predstate: spawn_BackingStorelistener");
      return FALSE;
    }
  } else if (!isCtlr()) {
    if (open_BackingStore(p_bs_cp, !ctlr) == FALSE) {
#if defined(PORTING_TO_SLURMv17)	    
      ErrExit(ErrExit_STUCK, "collect_predstate[!ctlr]: open_BackingStore(!isCtlr)");
#else
      ErrExit(ErrExit_WARN, "collect_predstate[!ctlr]: open_BackingStore(!isCtlr)");
#endif      
      return FALSE;
    }
    if (merge_BackingStore(p_pred_mp_cp, MS_CONTROLLER) == FALSE) {
      ErrExit(ErrExit_STUCK, "collect_predstate[!ctlr]: merge_state(CONTROLLER)");
      return FALSE;
    }
  }
  return claim_varrun(p_pred_pid_cp, p_pred_mp_cp);
}
