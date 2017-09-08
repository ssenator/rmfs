
#ifndef RMFS_backingstore_H
#define RMFS_backingstore_H

/*
 * backing store is composed of a header and multiple records
 *
 * jobs with an infinite end time are recorded from the beginning of the log up to (n*sizeof(aligned_backingstore_record))
 * jobs with a defined end time are recorded after those, ordered by their timestamps.
 *
 * pre-existing records with an end_time past the present may be cleared or overwritten
 */

/*
 * Records are layed out according to the schema
 *
 *  [header record]
 *
 *  [barrier]               <<-- bits set: barrier, header, infinite expiration
 *
 *  [infinite expiration 1]
 *  [...]
 *  [infinite expiration n] <<-- n = n_infinite_jobs
 *                               grow from header down
 *
 *  [barrier]               <<-- bits set: barrier, infinite expiration, node
 *
 *  [empty]
 *  [empty]
 *  [empty]                 <<-- 0 to many empty records may exist
 *
 *  [node delta]            <<-- last written {index from trailer barrier}
 *  [...]                        grow from trailer up
 *  [node delta]
 *
 *  [barrier]               <<-- bits set: barrier, node, trailer
 *
 *  [trailer record]         <<-- duplicate of header record
 *
 */

#define BS_MIN_INF_EXP_SPACE (8)
#define BS_FORMAT_VERSION    1.0

typedef struct backingstore_header_type {
  unsigned int  header              :1;
  unsigned int  node_delta          :1;
  unsigned int  infinite_expiration :1;
  unsigned int  barrier             :1;
  unsigned int   _unused            :(sizeof(unsigned int) * NBBY) - 5; /* 5 = # bits used */
  unsigned int  trailer             :1;
} bs_hdrtyp_t;

typedef struct backingstore_header {
  bs_hdrtyp_t   htyp;
  int           format_version;
  time_t        time_stamp;
  pid_t         owner;
  int           size;
  int           n_infinite_expiration;
  int           last_written;
  char          host[HOST_NAME_MAX];
  unsigned long marker;
} bs_hdr_t;

typedef struct aligned_backingstore_record {
  union {
    struct backingstore_record {
      bs_hdrtyp_t htyp;
      time_t      tm_exp;
      uint32_t    job_id;
      int         rn_sig;
      struct xattr {
	char      nm[XATTR_NM_MAXLEN];
	char      ctx[XATTR_CTX_MAXLEN];
      } xattr;
    } _r;
    bs_hdr_t      align;
  } _u;
} bs_record_t;

typedef enum claim_action {
  CL_TEST = 0,
  CL_CLAIM = 1
} claim_t;

typedef enum merge_bs_source {
  MS_PREDECESSOR  = -1,
  MS_UNDEFINED    = 0,
  MS_BACKINGSTORE = 1,
  MS_CONTROLLER   = 2
} merge_bs_source_t;

extern tri_t claim_BackingStore(config_param_t *, config_param_t *, claim_t);
extern tri_t open_BackingStore(config_param_t *, int);
extern tri_t merge_BackingStore(config_param_t *, merge_bs_source_t);
extern tri_t requestWrite_BackingStore(char *);
extern tri_t spawn_BackingStorelistener(config_param_t *);

#endif
