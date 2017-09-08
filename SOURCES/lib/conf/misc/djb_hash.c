
#include <limits.h>
#include <string.h>


/*
 * Dan J Bernstein's hash algorithm as documented in comp.lang.c and elsewhere
 *   http://eternallyconfuzzled.com/tuts/algorithms/jsw_tut_hashing.aspx
 *   http://en.literateprograms.org/Hash_function_comparison_(C,_sh)
 *   http://www.cse.yorku.ca/~oz/hash.html
 *
 * This implementation does not have a large number of structures,
 * so hash benefits may be minimal. However, by using hashes throughout
 * and not directly manipulating strings, we attempt to isolate the
 * string manipulation routines to a much smaller footprint, anticipating
 * code vulnerability audits.
 *
 * _POSIX_PATH_MAX is somewhat arbitrary upper bound, but we can control
 * most of our input, so it is not an unreasonable upper bound for a filesystem
 * to prevent fuzz't input
 */
int
djb_strtohash(char *str)
{
  char *p, *p_term;
  int   hash, i, len;

  p_term  = strchr(str, '\0');
  len     = p_term - str;
  p       = str;
  hash    = 0;

  if (len <= 0 || len > _POSIX_PATH_MAX) {
    return (~0); /*to flag as invalid*/
  }

  for (i = 0; i < len; i++) {
    hash = 33 * hash ^ p[i];
  }
  return hash;
}

int
djb_accumulate(int prev_hash, int key) {
  int hash = prev_hash;

  if (key == 0) {
    return (~0);
  }
  
  hash = 33 * hash ^ key;
  return hash;
}
