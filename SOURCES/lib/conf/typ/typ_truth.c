
#include "rmfs.h"

tri_t
typ_trilene(rmfs_param_t *p_val) {
  
  if (!p_val) {
    ErrExit(ErrExit_ASSERT, "typ_trilene: !p_val");
    return FALSE;
  }

  if (UNSET == p_val->ue.truth ||
      FALSE == p_val->ue.truth ||
      TRUE  == p_val->ue.truth) {
    return TRUE;
  }
  return FALSE;
}

static struct truth_hash {
  char *str;
  tri_t truth;
  int   h;
} truth_hash_tab[] =
{
  { .str = "T",     .truth = TRUE, .h = 0 },
  { .str = "TRUE",  .truth = TRUE, .h = 0 },
  { .str = "t",     .truth = TRUE, .h = 0 },
  { .str = "truth", .truth = TRUE, .h = 0 },
  { .str = "1",     .truth = TRUE, .h = 0 },
  { .str = "set",   .truth = TRUE, .h = 0 },
  { .str = "on",    .truth = TRUE, .h = 0 },
  
  { .str = "F",     .truth = FALSE, .h = 0 },
  { .str = "FALSE", .truth = FALSE, .h = 0 },
  { .str = "f",     .truth = FALSE, .h = 0 },
  { .str = "false", .truth = FALSE, .h = 0 },
  { .str = "0",     .truth = FALSE, .h = 0 },
  { .str = "off",   .truth = FALSE, .h = 0 },
  { .str = "clear", .truth = FALSE, .h = 0 },

  { .str = NULL,    .truth = UNSET, .h = ~0 }
};

tri_t
typ_boolean(rmfs_param_t *p_val) {
  tri_t              t;
  int                h;
  struct truth_hash *p_th;
  
  t = UNSET;
  if (!p_val) {
    ErrExit(ErrExit_ASSERT, "typ_boolean: !p_val");
    return t;
  }
  t = (p_val->ue.truth == FALSE || p_val->ue.truth == TRUE);
  if (t) {
    return t;
  }

  for (p_th = &truth_hash_tab[0];
           p_th->str && t != TRUE && t != FALSE;
               p_th++) {
    
    if (!IS_VALID_HASH(p_th->h)) {
      p_th->h = djb_strtohash(p_th->str);
      if (!IS_VALID_HASH(p_th->h)) {
	ErrExit(ErrExit_ASSERT, "typ_boolean: djb_strtohash(truth_hash) !INVALID_HASH");
	break;
      }
    }

    h = djb_strtohash(p_val->ue.str);
    if (!IS_VALID_HASH(h)) {
      ErrExit(ErrExit_ASSERT, "typ_boolean: INVALID_HASH(h)");
      break;
    }
    if (p_th->h == h) {
      t = TRUE;
      p_val->ue.truth = p_th->truth;
    }
  }
  return t;
}

char *
tri2str[] = { "unset", "false", "true" };
char *
bool2str[] = { "false", "true" };

char *
b2boolstr(bool_t btruth) {
  switch (btruth) {
  case FALSE:
    return bool2str[0];
    break;
  case TRUE:
    return bool2str[1];
    break;
  default:
    ErrExit(ErrExit_ASSERT, "b2boolstr: boolean logic has > 2 states");
    break;
  }
  return "b2boolstr: <cannot happen>";
}

char *
t2tristr(tri_t truth) {
  switch (truth) {
  case UNSET:
    return tri2str[0];
    break;
  case FALSE:
    return tri2str[1];
    break;
  case TRUE:
    return tri2str[2];
    break;
  default:
    ErrExit(ErrExit_ASSERT, "t2tristr: trinary logic has > 3 states");
    break;
  }
  return "t2tristr: <cannot happen>";
}
