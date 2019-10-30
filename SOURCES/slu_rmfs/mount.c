
#include "rmfs.h"



/*
 * XXXaltimp convert to using BackingStore (/dev/shm) rather than /var/run/...
 */

FILE *
already_mounted(void) {
  config_param_t *mpfile_cp, *pidfile_cp;
  FILE           *prev_ctlfile;
  char           *prev_mp;
  size_t          sz;
  char            prev_mp_ctl[_POSIX_PATH_MAX];

  mpfile_cp = getconfig_fromnm("mountpointfile");
  pidfile_cp = getconfig_fromnm("pidfile");

  if (!mpfile_cp) {
    ErrExit(ErrExit_INTERNAL, "already_mounted: !mpfile_cp");
  }
  if (!pidfile_cp) {
    ErrExit(ErrExit_INTERNAL, "already_mounted: !pidfile_cp");
  }

  if (!mpfile_cp->val.pd.fullpath) {
    return NULL;
  }
  if (!pidfile_cp->val.pd.fullpath) {
    return NULL;
  }

  /*
   * should have been setup when originally tried to set mountpointfile and pidfile
   */
  if (!mpfile_cp->val.pd.fstr) {
    ErrExit(ErrExit_INTERNAL, "already_mounted: mpfile: no fstr");
  }
  if (!pidfile_cp->val.pd.fstr) {
    ErrExit(ErrExit_INTERNAL, "already_mounted: pidfile: no fstr");
  }

  sz = fscanf(mpfile_cp->val.pd.fstr, "%as", &prev_mp);
  if (sz == EOF || sz == 0) {
    ErrExit(ErrExit_INTERNAL, "already_mounted: prev_mp fscanf");
  }

  (void) strlcpy(prev_mp_ctl, prev_mp, _POSIX_PATH_MAX);  /*bsd/string.h*/
  sz = strlcat(prev_mp_ctl, "/control", _POSIX_PATH_MAX); /*bsd/string.h*/
  if (sz <= 0) {
    ErrExit(ErrExit_INTERNAL, "already_mounted: strlcat prev_mp_ctl");
  }
  
  if (!(prev_ctlfile = fopen(prev_mp_ctl, "r+"))) {
    ErrExit(ErrExit_INTERNAL, "already_mounted: fopen(prev_mp_ctrl)");
  }
  setvbuf(prev_ctlfile, NULL, _IONBF, BUFSIZ); /*no buffering*/
  return prev_ctlfile;
}


void
reclaim_files() {
  config_param_t *mpfile_cp, *pidfile_cp;
  int             varrun_opencp(config_param_t *); /*collect_os.c*/

  pidfile_cp = getconfig_fromnm("pidfile");
  mpfile_cp = getconfig_fromnm("mountpointfile");

  if (!pidfile_cp) {
    ErrExit(ErrExit_INTERNAL, "reclaim_files: !pidfile_cp");
  }
  if (!mpfile_cp) {
    ErrExit(ErrExit_INTERNAL, "reclaim_files: !mpfile_cp");
  }
  if (varrun_opencp(pidfile_cp) < 0) {
    ErrExit(ErrExit_INTERNAL, "reclaim_files: varrun_opencp(pidfile_cp)");
  }
  if (varrun_opencp(mpfile_cp) < 0) {
    ErrExit(ErrExit_INTERNAL, "reclaim_files: varrun_opencp(mpfile_cp)");
  }
  return;
}

tri_t
request_unmount(FILE *prev_ctlfile, long waited_for) {
  config_param_t *unmountwait_cp;
  int             w, rc;
  extern int      errno;

  if (!prev_ctlfile) {
    return FALSE;
  }

  unmountwait_cp = getconfig_fromnm("unmountwait");

  if ((rc = fprintf(prev_ctlfile, "umount")) < 0) {
    int e = errno;
    if (e != ENOENT && e != EACCES) {
      ErrExit(ErrExit_STUCK, "request_unmount: fprintf(prev_mp_ctl, \"umount\")");
    }
  }

  /*
   * if there is an unmount timeout, and we haven't timed out, try again
   */
  w = unmountwait_cp->val.ue.l;
  if ((rc < 0) && (w > 0) && (waited_for < w)) {

    ErrExit(ErrExit_WARN, "request_unmount: waiting for predecessor");
    sleep(w);
    request_unmount(prev_ctlfile, w);

    if (already_mounted()) {
      ErrExit(ErrExit_STUCK, "request_unmount: waited too long, still mounted");
    }
  }
  reclaim_files();
  return TRUE;
}

