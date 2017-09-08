
#ifndef RMFS_MISCSUBR_H_
#define RMFS_MISCSUBR_H_

/* used heavily when strings are ingested and compared, defined in lib/conf/djb_hash.c */
extern int djb_strtohash(char *);
extern int djb_accumulate(int, int);
extern int internal_strlen(const char *);

#endif
