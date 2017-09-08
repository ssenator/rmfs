
#include "rmfs.h"

/*
 * (slu)rmfs provides a file system view into the resource manager state namespace
 *  allowing some objects to have additional state associated with the file system node
 *
 * This implementation utilizes fuse a file system implementation.
 * A production implementation would require a 9p, NFS or VNFS implementation.
 */

extern void mk_fs(void);
extern void ingest_config(int);

int
main(int ac, char **av) {
  
  extern struct fuse_operations rmfs_file_ops; /*rmfs_fuseops.c*/
  extern struct fuse_args       f_args;        /* linkage/fuse.h */

  /* = FUSE_ARGS_INIT(ac, av), consumed by fuse_opt_parse */
  f_args.argc      = ac;
  f_args.argv      = av;
  f_args.allocated = 0;
  
  ingest_config(INGEST_COLDSTART);
  mk_fs();
  
  ingest_config(INGEST_PREVSTATE);

  fuse_main(ac, av, &rmfs_file_ops, /*userdata*/ NULL);
  /* FALLTHROUGH */

  /*XXX fuse_request_unmount();*/
  CleanExit();
  return ExitOK;
}
