#include "count_min_sketch.h"
#include <inttypes.h> /* PRIu64 */
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOG_TWO 0.6931471805599453

/* private functions */
static int __setup_cms(CountMinSketch *cms, uint32_t width, uint32_t depth,
                       double error_rate, double confidence);
static uint64_t *__default_hash(unsigned int num_hashes, const char *key);
static uint64_t __fnv_1a(const char *key, int seed);
static int __compare(const void *a, const void *b);
static int32_t __safe_add(int32_t a, uint32_t b);
static int32_t __safe_sub(int32_t a, uint32_t b);
static int32_t __safe_add_2(int32_t a, int32_t b);

// Compatibility with non-clang compilers
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

int cms_init_by_dim(CountMinSketch *cms, uint32_t width, uint32_t depth) {
  if (depth < 1 || width < 1) {
    fprintf(stderr, "Unable to initialize the count-min sketch since either "
                    "width or depth is 0!\n");
    return CMS_ERROR;
  }
  double confidence = 1 - (1 / pow(2, depth));
  double error_rate = 2 / (double)width;
  return __setup_cms(cms, width, depth, error_rate, confidence);
}

int cms_init_by_prob(CountMinSketch *cms, double error_rate, double confidence){
  // Validate input parameters
  if (error_rate <= 0 || error_rate >= 1) {
    fprintf(stderr, "Error rate must be between 0 and 1 (exclusive)\n");
    return CMS_ERROR;
  }

  if (confidence <= 0 || confidence >= 1) {
    fprintf(stderr, "Confidence must be between 0 and 1 (exclusive)\n");
    return CMS_ERROR;
  }

  uint32_t width = (uint32_t)ceil(2.0 / error_rate);
  uint32_t depth = (uint32_t)ceil(log2(1.0 / (1.0 - confidence)));

  // Ensure minimum dimensions
  if (width < 1)
    width = 1;
  if (depth < 1)
    depth = 1;

  return __setup_cms(cms, width, depth, error_rate, confidence);
}

int cms_destroy(CountMinSketch *cms) {
  free(cms->bins);
  cms->width = 0;
  cms->depth = 0;
  cms->confidence = 0.0;
  cms->error_rate = 0.0;
  cms->elements_added = 0;
  cms->hash_function = NULL;
  cms->bins = NULL;

  return CMS_SUCCESS;
}

uint64_t *cms_get_hashes(CountMinSketch *cms, const char *key) {
  return cms->hash_function(cms->depth, key);
}

int32_t cms_add_inc_alt(CountMinSketch *cms, uint64_t *hashes,
                        unsigned int num_hashes, uint32_t x) {
  if (num_hashes < cms->depth) {
    fprintf(stderr, "Insufficient hashes to complete the addition of the "
                    "element to the count-min sketch!");
    return CMS_ERROR;
  }
  int num_add = INT32_MAX;
  for (unsigned int i = 0; i < cms->depth; ++i) {
    uint64_t bin = (hashes[i] % cms->width) + (i * cms->width);
    cms->bins[bin] = __safe_add(cms->bins[bin], x);
    /* currently a standard min strategy */
    if (cms->bins[bin] < num_add) {
      num_add = cms->bins[bin];
    }
  }
  cms->elements_added += x;
  return num_add;
}

int32_t cms_add_inc(CountMinSketch *cms, const char *key, unsigned int x) {
  uint64_t *hashes = cms_get_hashes(cms, key);
  int32_t num_add = cms_add_inc_alt(cms, hashes, cms->depth, x);
  free(hashes);
  return num_add;
}

int32_t cms_remove_inc_alt(CountMinSketch *cms, uint64_t *hashes,
                           unsigned int num_hashes, unsigned int x) {
  if (num_hashes < cms->depth) {
    fprintf(stderr, "Insufficient hashes to complete the removal of the "
                    "element to the count-min sketch!");
    return CMS_ERROR;
  }
  int32_t num_add = INT32_MAX;
  for (unsigned int i = 0; i < cms->depth; ++i) {
    uint32_t bin = (hashes[i] % cms->width) + (i * cms->width);
    cms->bins[bin] = __safe_sub(cms->bins[bin], x);
    if (cms->bins[bin] < num_add) {
      num_add = cms->bins[bin];
    }
  }
  cms->elements_added -= x;
  return num_add;
}

int32_t cms_remove_inc(CountMinSketch *cms, const char *key, uint32_t x) {
  uint64_t *hashes = cms_get_hashes(cms, key);
  int32_t num_add = cms_remove_inc_alt(cms, hashes, cms->depth, x);
  free(hashes);
  return num_add;
}

int32_t cms_check_alt(CountMinSketch *cms, uint64_t *hashes,
                      unsigned int num_hashes) {
  if (num_hashes < cms->depth) {
    fprintf(stderr, "Insufficient hashes to complete the min lookup of the "
                    "element to the count-min sketch!");
    return CMS_ERROR;
  }
  int32_t num_add = INT32_MAX;
  for (unsigned int i = 0; i < cms->depth; ++i) {
    uint32_t bin = (hashes[i] % cms->width) + (i * cms->width);
    if (cms->bins[bin] < num_add) {
      num_add = cms->bins[bin];
    }
  }
  return num_add;
}

int32_t cms_check(CountMinSketch *cms, const char *key) {
  uint64_t *hashes = cms_get_hashes(cms, key);
  int32_t num_add = cms_check_alt(cms, hashes, cms->depth);
  free(hashes);
  return num_add;
}

int32_t cms_check_mean_alt(CountMinSketch *cms, uint64_t *hashes,
                           unsigned int num_hashes) {
  if (num_hashes < cms->depth) {
    fprintf(stderr, "Insufficient hashes to complete the mean lookup of the "
                    "element to the count-min sketch!");
    return CMS_ERROR;
  }
  int32_t num_add = 0;
  for (unsigned int i = 0; i < cms->depth; ++i) {
    uint32_t bin = (hashes[i] % cms->width) + (i * cms->width);
    num_add += cms->bins[bin];
  }
  return num_add / cms->depth;
}

int32_t cms_check_mean(CountMinSketch *cms, const char *key) {
  uint64_t *hashes = cms_get_hashes(cms, key);
  int32_t num_add = cms_check_mean_alt(cms, hashes, cms->depth);
  free(hashes);
  return num_add;
}

/*******************************************************************************
 *    PRIVATE FUNCTIONS
 *******************************************************************************/
static int __setup_cms(CountMinSketch *cms, unsigned int width,
                       unsigned int depth, double error_rate,
                       double confidence) {
  cms->width = width;
  cms->depth = depth;
  cms->confidence = confidence;
  cms->error_rate = error_rate;
  cms->elements_added = 0;
  cms->bins = (int32_t *)calloc((width * depth), sizeof(int32_t));
  cms->hash_function = __default_hash; /* TODO: custom hash function */

  if (NULL == cms->bins) {
    fprintf(stderr, "Failed to allocate %zu bytes for bins!",
            ((width * depth) * sizeof(int32_t)));
    return CMS_ERROR;
  }
  return CMS_SUCCESS;
}

/* NOTE: The caller will free the results */
static uint64_t *__default_hash(unsigned int num_hashes, const char *str) {
  uint64_t *results = (uint64_t *)calloc(num_hashes, sizeof(uint64_t));
  int i;
  for (i = 0; i < num_hashes; ++i) {
    results[i] = __fnv_1a(str, i);
  }
  return results;
}

static uint64_t __fnv_1a(const char *key, int seed) {
  // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
  int i, len = strlen(key);
  uint64_t h = 14695981039346656037ULL +
               (31 * seed); // FNV_OFFSET 64 bit with magic number seed
  for (i = 0; i < len; ++i) {
    h = h ^ (unsigned char)key[i];
    h = h * 1099511628211ULL; // FNV_PRIME 64 bit
  }
  return h;
}

static int __compare(const void *a, const void *b) {
  return (*(int64_t *)a - *(int64_t *)b);
}

static int32_t __safe_add(int32_t a, uint32_t b) {
  if (a == INT32_MAX || a == INT32_MIN) {
    return a;
  }

  /* use the gcc macro if compiling with GCC, otherwise, simple overflow check
   */
  int32_t c = 0;
#if (__has_builtin(__builtin_add_overflow)) ||                                 \
    (defined(__GNUC__) && __GNUC__ >= 5)
  bool bl = __builtin_add_overflow(a, b, &c);
  if (bl) {
    c = INT32_MAX;
  }
#else
  c = ((int64_t)a + b > INT32_MAX) ? INT32_MAX : (a + b);
#endif

  return c;
}

static int32_t __safe_sub(int32_t a, uint32_t b) {
  if (a == INT32_MAX || a == INT32_MIN) {
    return a;
  }

  /* use the gcc macro if compiling with GCC, otherwise, simple overflow check
   */
  int32_t c = 0;
#if (__has_builtin(__builtin_sub_overflow)) ||                                 \
    (defined(__GNUC__) && __GNUC__ >= 5)
  bool bl = __builtin_sub_overflow(a, b, &c);
  if (bl) {
    c = INT32_MIN;
  }
#else
  c = ((int64_t)b - a < INT32_MIN) ? INT32_MAX : (a - b);
#endif

  return c;
}

static int32_t __safe_add_2(int32_t a, int32_t b) {
  if (a == INT32_MAX || a == INT32_MIN) {
    return a;
  }

  /* use the gcc macro if compiling with GCC, otherwise, simple overflow check
   */
  int64_t c = (int64_t)a + (int64_t)b;
  if (c <= INT32_MIN)
    return INT32_MIN;
  else if (c >= INT32_MAX)
    return INT32_MAX;
  return (int32_t)c;
}
