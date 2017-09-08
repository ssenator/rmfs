
#include "rmfs.h"


/*
 * mkdir_varrun()
 *  as it is named, creates the dir specified in the varrun config parameter
 */
int
mkdir_varrun(config_param_t *varrun_cp) {
  int          rc, e;
  extern int   errno;
  extern tri_t typ_dir(rmfs_param_t *);
  
  if (!varrun_cp) {
    return -1;
  }
  if (!varrun_cp->val.ue.pathnm) {
    return -1;
  }
  if (FALSE == typ_dir(&varrun_cp->val)) {
    return -1;
  }
  rc = mkdir(varrun_cp->val.ue.pathnm, S_IRWXU|S_IRGRP|S_IXGRP|S_IXOTH|S_IROTH);
  if (-1 == rc) {
    e = errno;
    if (e != EEXIST) {
      ErrExit(ErrExit_NOPERM, "mkdir_varrun: mkdir");
      ;
    }
  }
  return rc; 
}

/*
 * open one of the files in our /var/run space,
 *  and record the file stream for it
 *  get the pid if anyone else claims these
 *
 * see also: collect_predecessor()
 */
int
varrun_opencp(config_param_t *file_cp) {
  char           *fullpath;
  int             fd, e;
  extern int      errno;
  FILE           *fstr;
  config_param_t *varrun_cp, *pid_cp;
  mode_t          mode;
  pid_t           owner;

  if (!file_cp) {
        ErrExit(ErrExit_ASSERT, "varrun_opencp: !file_cp");
	return 0;
  }
  varrun_cp = getconfig_fromnm("varrun");
  if (!varrun_cp) {
    ErrExit(ErrExit_ASSERT, "varrun_opencp: !varrun_cp");
    return 0;
  }
  pid_cp = getconfig_fromnm("pid");
  if (!pid_cp) {
    ErrExit(ErrExit_ASSERT, "varrun_opencp: !pid_cp");
    return 0;
  }
  
  /*
   * if called to open the file in file_cp
   *  attempt to use the saved fullpathname, if it exists
   *
   * if there is no saved fullpathname, allocate it
   *  ...also create a dotfile for the pathname, if the pathname already exists
   */
  fullpath = file_cp->val.pd.fullpath;
  if (!file_cp->val.pd.fullpath) {
    fullpath = calloc(_POSIX_PATH_MAX, sizeof(char));
    if (!fullpath) {
      ErrExit(ErrExit_ASSERT, "varrun_opencp: calloc fullpath NULL");
      return -1;
    }
    if (snprintf(fullpath, _POSIX_PATH_MAX-1, "%s/%s",
		 varrun_cp->val.ue.pathnm, file_cp->val.ue.pathnm) < 0) {
    
      ErrExit(ErrExit_ASSERT, "varrun_opencp: construct fullpath");
      return -1;
    }
    file_cp->val.pd.fullpath = fullpath;
  }

  fd = file_cp->val.pd.fd;
  if (fd <= 0) {
    mode = S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH;
    if ((fd = open(fullpath, O_RDWR, mode)) < 0) {
      e = errno;
      if (e == ENOENT) {
	if ((fd = open(fullpath, O_RDWR | O_CREAT, mode)) < 0) {
	  ErrExit(ErrExit_ASSERT, "varrun_opencp: cannot create fullname");
	  return -1;
	}
      }
      ErrExit(ErrExit_ASSERT, "varrun_opencp: fullname open fail");
      return -1;
    }
    file_cp->val.pd.fd = fd;
  }

  fstr = file_cp->val.pd.fstr;
  if (!fstr) {
    if (!(fstr = fdopen(fd, "r+"))) {
      ErrExit(ErrExit_ASSERT, "varrun_opencp: fdopen(fullname) fail");
      return -1;
    }
    file_cp->val.pd.fstr = fstr;
    file_cp->val.pd.is_filestr = TRUE;
    file_cp->val.pd.is_mmapped = FALSE;
  }

  owner = file_cp->val.pd.owner;
  if (owner == 0) {
    owner = fcntl(fd, F_GETOWN, /*ignored*/ 0);
  }
  if (owner == 0 || owner != pid_cp->val.ue.pid) {
    
    /* F_SETOWN expires when the file is closed or when a pid exits */
    if (fcntl(fd, F_SETOWN, pid_cp->val.ue.pid) != -1) {
      owner                 = pid_cp->val.ue.pid;
      file_cp->val.pd.ours  = TRUE;
      file_cp->val.pd.owner = owner;
      
    } else {
      e = errno;
      if (e != EPERM) {
	ErrExit(ErrExit_ASSERT, "varrun_opencp: F_SETOWN fail");
	return -1;
      }
    }
  }
  return 0;
}

param_source_t
collectos_pid(config_param_t *p_cp) {
  char t[_POSIX_PATH_MAX];

  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "collectos_pid: !p_cp");
    return PSRC_NONE;
  }
  set_val_pid(p_cp, getpid());
  if (typ_copyout(p_cp, t, _POSIX_PATH_MAX) <= 0) {
    ErrExit(ErrExit_ASSERT, "collectos_pid: typ_copyout <= 0");
    return PSRC_NONE;
  }
  p_cp->val.size = internal_strlen(t)+1;
  
  return PSRC_DERIVED;
}

#ifndef _HOST_NAME_MAX
#define _HOST_NAME_MAX _POSIX_PATH_MAX-1
#endif

param_source_t
collectos_hostname(config_param_t *p_cp) {
  char *p_hostnm;
  
  if (!p_cp) {
    ErrExit(ErrExit_ASSERT, "collectos_hostname: !p_cp");
    return PSRC_NONE;
  }
  if (!(p_hostnm = calloc(_HOST_NAME_MAX+1, sizeof(char)))) {
    ErrExit(ErrExit_ENOMEM, "collectos_hostname: !calloc p_hostnm");
  }
  /*strictly speaking gethostname() need not have a terminating '\0'*/
  if (gethostname(p_hostnm, _HOST_NAME_MAX) == -1) {
    ErrExit(ErrExit_TEMPFAIL, "collectos_hostname: gethostname");
  }
  set_val_charptr(p_cp, p_hostnm);
  
  return PSRC_DERIVED;
}


/*
 * collectos_pidfile()
 *  determines and sets the fullpath into the config_param_t
 *  opens the file
 *  attempts to set this PID as the owner of the file
 *  if this succeeds, it records the PID into the file
 *  if this fails, it records the owner's PID into the config param
 *
 *  see also: collect_predecessor()
 */
param_source_t
collectos_pidfile(config_param_t *pidfile_cp) {
  config_param_t *pid_cp, *varrun_cp;

  if (!pidfile_cp) {
    ErrExit(ErrExit_ASSERT, "collectos_pidfile: !p_cp");
    return PSRC_NONE;
  }
  
  pid_cp = getconfig_fromnm("pid");
  if (!pid_cp) {
    ErrExit(ErrExit_ASSERT, "collectos_pidfile: !pid_cp");
    return PSRC_NONE;
  }
  varrun_cp = getconfig_fromnm("varrun");
  if (!varrun_cp) {
    ErrExit(ErrExit_ASSERT, "collectos_pidfile: !varrun_cp failed");
    return PSRC_NONE;
  }
  
  if (mkdir_varrun(varrun_cp) < 0) {
    ErrExit(ErrExit_ASSERT, "collectos_pidfile: mkdir_varrun failed");
    return PSRC_NONE;
  }
  if (varrun_opencp(pidfile_cp) < 0) {
    ErrExit(ErrExit_ASSERT, "collectos_pidfile: varrun_opencp failed");
    return PSRC_NONE;
  }
  
  if (!pidfile_cp->val.pd.ours) {
    /*
     * if the previous instance of ourselves exists,
     * do not record ourselves until later calls to varrun_opencp
     */
    ErrExit(ErrExit_WARN, "collectos_pidfile: not owner of mpfile");
    /*FALLTHROUGH*/
  } else {
    fprintf(pidfile_cp->val.pd.fstr, "%d\n", pid_cp->val.ue.pid);
    fflush(pidfile_cp->val.pd.fstr);
    fsync(pidfile_cp->val.pd.fd);
  }
  return PSRC_DERIVED;
}

/*
 * record_mountpoint() is the partner of collectos_pidfile()
 * if it is able to do so, it records the new mountpoint in mpfile_cp
 * if not it takes note in the mountpointfile config_param_t for later action
 */

param_source_t
record_mountpoint(config_param_t *mpfile_cp) {
  config_param_t *mp_cp, *varrun_cp, *pid_cp;

  if (!mpfile_cp) {
    ErrExit(ErrExit_ASSERT, "record_mountpoint: !mpfile_cp");
    return PSRC_NONE;
  }
  mp_cp = getconfig_fromnm("MountPoint");
  if (!mp_cp) {
    ErrExit(ErrExit_ASSERT, "record_mountpoint: !mp_cp");
    return PSRC_NONE;
  }
  varrun_cp = getconfig_fromnm("varrun");
  if (!varrun_cp) {
    ErrExit(ErrExit_ASSERT, "record_mountpoint: !varrun_cp failed");
    return PSRC_NONE;
  }
  pid_cp = getconfig_fromnm("pid");
  if (!pid_cp) {
    ErrExit(ErrExit_ASSERT, "collectos_pidfile: !pid_cp");
    return PSRC_NONE;
  }
  if (mkdir_varrun(varrun_cp) < 0) {
    ErrExit(ErrExit_ASSERT, "record_mountpoint: !mkdir_varrun()");
    return PSRC_NONE;
  }
  if (varrun_opencp(mpfile_cp) < 0) {
    ErrExit(ErrExit_ASSERT, "record_mountpoint: !varrun_opencp(mpfile_cp)");
    return PSRC_NONE;
  }
  /* (FILE *) fstr remains open */
  if (mpfile_cp->val.pd.ours) {
      fprintf(mpfile_cp->val.pd.fstr, "%s\n", mp_cp->val.ue.pathnm);
      fflush(mpfile_cp->val.pd.fstr);
      fsync(mpfile_cp->val.pd.fd);
      
  } else {
    ErrExit(ErrExit_ASSERT, "record_mountpoint: not owner of mpfile");
    /*FALLTHROUGH*/
  }
  return PSRC_DERIVED;
}


