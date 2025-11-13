#ifndef GEO_HASH_H__
#define GEO_HASH_H__

#include <stddef.h>
#include <stdbool.h>

/* Constants */
#define GEOHASH_MAX_PRECISION 12
#define GEOHASH_DEFAULT_PRECISION 9

/* Error codes */
typedef enum {
    GEOHASH_OK = 0,
    GEOHASH_ERROR_INVALID_POINT = -1,
    GEOHASH_ERROR_INVALID_HASH = -2,
    GEOHASH_ERROR_ALLOCATION = -3,
    GEOHASH_ERROR_INVALID_PRECISION = -4
} GeoHashError;

/* Structures */
typedef struct {
    double latitude;
    double longitude;
} GeoPoint;

typedef struct {
    char *hash;
    size_t length;
} GeoHash;

typedef enum { 
    GEOHASH_NORTH = 0, 
    GEOHASH_SOUTH, 
    GEOHASH_EAST, 
    GEOHASH_WEST 
} GeoDirection;

typedef struct {
    GeoHash north;
    GeoHash south;
    GeoHash east;
    GeoHash west;
    GeoHash northeast;
    GeoHash northwest;
    GeoHash southeast;
    GeoHash southwest;
} GeoAdjacent;

typedef struct {
    double min_latitude;
    double max_latitude;
    double min_longitude;
    double max_longitude;
} GeoBounds;

/* GeoPoint functions */
GeoPoint geopoint_create(double latitude, double longitude);
bool geopoint_is_valid(const GeoPoint *point);

/* GeoHash encoding/decoding */
GeoHashError geohash_encode(const GeoPoint *point, size_t precision, GeoHash *result);
GeoHashError geohash_encode_default(const GeoPoint *point, GeoHash *result);
GeoHashError geohash_decode(const GeoHash *hash, GeoPoint *result);

/* GeoHash bounds */
GeoHashError geohash_get_bounds(const GeoHash *hash, GeoBounds *bounds);

/* GeoHash adjacency */
GeoHashError geohash_get_adjacent(const GeoHash *hash, GeoDirection direction, GeoHash *result);
GeoHashError geohash_get_all_adjacent(const GeoHash *hash, GeoAdjacent *result);

/* GeoHash utility functions */
GeoHash geohash_create(void);
void geohash_free(GeoHash *hash);
GeoHashError geohash_copy(const GeoHash *src, GeoHash *dest);
const char* geohash_error_string(GeoHashError error);

/* GeoAdjacent utility functions */
void geoadjacent_free(GeoAdjacent *adjacent);

#endif /* GEO_HASH_H__ */
