

#ifndef RMFS_H_
#define RMFS_H_

/*
 * This version of the "resource manager file system" aka rmfs implements
 * a dependency on the slurm resource manager and is implemented as a fuse
 * file system. See the rmfs_conf.c/slurmfs_config table for manifest constants.
 */

#include <sys/types.h>
#include <assert.h>
#include <linux/xattr.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <limits.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <bsd/string.h>
#include <sysexits.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "linkage/fuse.h"
#include "linkage/selinux.h"
#include "linkage/slurm.h"

#include "rmfs/types.h"
#include "rmfs/param.h"
#include "rmfs/backingstore.h"
#include "rmfs/conf.h"
#include "rmfs/rnode.h"
#include "rmfs/misc.h"

typedef enum ingest_cycles {
  INGEST_PREVSTATE = -1,
  INGEST_COLDSTART = 0,
  INGEST_1         = INGEST_COLDSTART + 1,
  INGEST_2         = INGEST_1 + 1,
  INGEST_3         = INGEST_1 + 2,
  INGEST_MAX       = INGEST_3
} ingest_cycle_t;

#define NXT_ingest_cycle(_pass_no)  ((_pass_no) >= 0 && (_pass_no) <= (INGEST_MAX))? (_pass_no) + 1: ((INGEST_MAX)+1))

#endif
