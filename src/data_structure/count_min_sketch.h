/**
 * @file count_min_sketch.h
 * @brief Count-Min Sketch probabilistic data structure for frequency estimation
 * 
 * A Count-Min Sketch is a probabilistic data structure that estimates the frequency
 * of elements in a stream. It provides configurable error and confidence bounds while
 * using constant memory regardless of the number of unique elements.
 * 
 * @author Redis C Library
 * @version 1.0
 */

#ifndef REDIS_C_COUNT_MIN_SKETCH_H__
#define REDIS_C_COUNT_MIN_SKETCH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @defgroup CMS_Status Status Codes
 * @{
 */
#define CMS_SUCCESS   0          /**< Operation completed successfully */
#define CMS_ERROR     INT32_MIN  /**< Operation failed */
/** @} */

/**
 * @defgroup CMS_Compiler Compiler-specific Definitions
 * @{
 */
#ifndef __GNUC__
#define __inline__ inline
#endif
/** @} */

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

typedef uint64_t* (*cms_hash_function)(unsigned int num_hashes, const char* key);

/**
 * @struct CountMinSketch
 * @brief Count-Min Sketch data structure
 * 
 * Represents a probabilistic frequency counter using a 2D matrix of bins.
 * The structure maintains `depth` independent hash functions, each mapping
 * to `width` counter bins.
 */
typedef struct {
    uint32_t depth;              /**< Number of hash functions (rows) */
    uint32_t width;              /**< Number of bins per hash function (columns) */
    int64_t elements_added;      /**< Total number of elements added/removed */
    double confidence;           /**< Confidence level (1 - error probability) */
    double error_rate;           /**< Maximum error rate per estimate */
    cms_hash_function hash_function; /**< Custom hash function (NULL for default) */
    int32_t* bins;               /**< Flat array of counters (size: depth * width) */
} CountMinSketch;

/**
 * @typedef count_min_sketch
 * @brief Alias for CountMinSketch structure
 */
typedef CountMinSketch count_min_sketch;

/* ============================================================================
 * Initialization and Destruction
 * ============================================================================
 */
int cms_init(CountMinSketch *cms, unsigned int width, unsigned int depth);
int cms_destroy(CountMinSketch *cms);

/* ============================================================================
 * Hash Operations
 * ============================================================================
 */

uint64_t *cms_get_hashes(CountMinSketch *cms,
                                           const char *key);

/* ============================================================================
 * Element Addition Operations
 * ============================================================================
 */
int32_t cms_add_inc(CountMinSketch *cms, const char *key, uint32_t x);
int32_t cms_add_inc_alt(CountMinSketch *cms, uint64_t *hashes,
                        unsigned int num_hashes, uint32_t x);
static __inline__ int32_t cms_add(CountMinSketch *cms, const char *key) {
  return cms_add_inc(cms, key, 1);
}
static __inline__ int32_t cms_add_alt(CountMinSketch *cms, uint64_t *hashes,
                                      unsigned int num_hashes) {
  return cms_add_inc_alt(cms, hashes, num_hashes, 1);
}

/* ============================================================================
 * Element Removal Operations
 * ============================================================================
 */

int32_t cms_remove_inc(CountMinSketch *cms, const char *key, uint32_t x);
static __inline__ int32_t cms_remove(CountMinSketch *cms, const char *key) {
  return cms_remove_inc(cms, key, 1);
}

/* ============================================================================
 * Query Operations
 * ============================================================================
 */

int32_t cms_check(CountMinSketch *cms, const char *key);
static __inline__ int32_t cms_check_min(CountMinSketch *cms, const char *key) {
  return cms_check(cms, key);
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif // REDIS_C__COUNT_MIN_SKETCH_H__
