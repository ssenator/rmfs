
/*
 * usage()
 *  print proper usage message and exits
 */

#include "rmfs.h"

char   *pusage_msg;  /* constructed from the fuse_opt table */

/*
 * various ways to exit
 */

void
Usage(int err, char *msg) {

  ErrExit (err? err: ErrExit_ARGPARSE, msg? msg: "usage error");
  /*NOTREACHED*/
}


/*
 * ErrExit() can return, not exit, in the following circumstance:
 *  It is called with ErrExit_ASSERT AND Debug is set and caught_in_a_loop == 0
 */
static int caught_in_a_loop = 0;

void
ErrExit(int err, char *msg) {
  config_param_t *p_cp;

  if (msg) {
    write(2, msg, strlen(msg));
    write(2, "\n", 1);
    fsync(2);
  }
  switch (err) {
  case ErrExit_ASSERT:
    p_cp = getconfig_fromnm_nohash("Debug");
    if (caught_in_a_loop == 0 && p_cp && p_cp->val.ue.ul == 0) {
      caught_in_a_loop++;
      return;
    }
    break;
    
  case ErrExit_WARN:  
    return;
    break;
  }
  
  exit (err);
}

void
CleanExit() {
  ErrExit(Exit_OK, NULL);
}
