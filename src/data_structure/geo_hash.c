#include "geo_hash.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Constants */
#define MAX_LONGITUDE 180.0
#define MIN_LONGITUDE -180.0
#define MAX_LATITUDE 90.0
#define MIN_LATITUDE -90.0
#define BASE32_ALPHABET "0123456789bcdefghjkmnpqrstuvwxyz"
#define BASE32_SIZE 32
#define BITS_PER_CHAR 5

/* Macros */
#define GET_BIT(num, idx) (((num) >> (idx)) & 1)

/* Lookup tables for adjacent cells */
#define NUM_DIRECTIONS 4
#define NUM_PARITY_TYPES 2

static const char *adjacent_map[NUM_DIRECTIONS][NUM_PARITY_TYPES] = {
    {"p0r21436x8zb9dcf5h7kjnmqesgutwvy", "bc01fg45238967deuvhjyznpkmstqrwx"}, /* North */
    {"14365h7k9dcfesgujnmqp0r2twvyx8zb", "238967debc01fg45kmstqrwxuvhjyznp"}, /* South */
    {"bc01fg45238967deuvhjyznpkmstqrwx", "p0r21436x8zb9dcf5h7kjnmqesgutwvy"}, /* East */
    {"238967debc01fg45kmstqrwxuvhjyznp", "14365h7k9dcfesgujnmqp0r2twvyx8zb"}  /* West */
};

static const char *border_map[NUM_DIRECTIONS][NUM_PARITY_TYPES] = {
    {"prxz", "bcfguvyz"},     /* North */
    {"028b", "0145hjnp"},     /* South */
    {"bcfguvyz", "prxz"},     /* East */
    {"0145hjnp", "028b"}      /* West */
};

/* Internal helper functions */
static int find_base32_index(char c) {
    for (int i = 0; i < BASE32_SIZE; i++) {
        if (BASE32_ALPHABET[i] == c) {
            return i;
        }
    }
    return -1;
}

static int find_char_index(char c, const char *str) {
    const char *pos = strchr(str, c);
    return pos ? (int)(pos - str) : -1;
}

static bool is_valid_geohash_string(const char *hash, size_t length) {
    if (!hash || length == 0 || length > GEOHASH_MAX_PRECISION) {
        return false;
    }
    
    for (size_t i = 0; i < length; i++) {
        if (find_base32_index(hash[i]) == -1) {
            return false;
        }
    }
    return true;
}

/* GeoPoint functions */
GeoPoint geopoint_create(double latitude, double longitude) {
    GeoPoint point = {latitude, longitude};
    return point;
}

bool geopoint_is_valid(const GeoPoint *point) {
    if (!point) return false;
    return (point->latitude >= MIN_LATITUDE && point->latitude <= MAX_LATITUDE &&
            point->longitude >= MIN_LONGITUDE && point->longitude <= MAX_LONGITUDE);
}

/* GeoHash utility functions */
GeoHash geohash_create(void) {
    GeoHash hash = {NULL, 0};
    return hash;
}

void geohash_free(GeoHash *hash) {
    if (hash && hash->hash) {
        free(hash->hash);
        hash->hash = NULL;
        hash->length = 0;
    }
}

GeoHashError geohash_copy(const GeoHash *src, GeoHash *dest) {
    if (!src || !dest) {
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    if (!src->hash || src->length == 0) {
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    dest->hash = (char *)malloc(src->length + 1);
    if (!dest->hash) {
        return GEOHASH_ERROR_ALLOCATION;
    }
    
    memcpy(dest->hash, src->hash, src->length);
    dest->hash[src->length] = '\0';
    dest->length = src->length;
    
    return GEOHASH_OK;
}

const char* geohash_error_string(GeoHashError error) {
    switch (error) {
        case GEOHASH_OK: return "Success";
        case GEOHASH_ERROR_INVALID_POINT: return "Invalid geographic point";
        case GEOHASH_ERROR_INVALID_HASH: return "Invalid geohash";
        case GEOHASH_ERROR_ALLOCATION: return "Memory allocation failed";
        case GEOHASH_ERROR_INVALID_PRECISION: return "Invalid precision";
        default: return "Unknown error";
    }
}

/* GeoHash encoding */
GeoHashError geohash_encode(const GeoPoint *point, size_t precision, GeoHash *result) {
    if (!point || !result) {
        return GEOHASH_ERROR_INVALID_POINT;
    }
    
    if (!geopoint_is_valid(point)) {
        return GEOHASH_ERROR_INVALID_POINT;
    }
    
    if (precision == 0 || precision > GEOHASH_MAX_PRECISION) {
        return GEOHASH_ERROR_INVALID_PRECISION;
    }
    
    result->hash = (char *)malloc(precision + 1);
    if (!result->hash) {
        return GEOHASH_ERROR_ALLOCATION;
    }
    
    double min_lon = MIN_LONGITUDE;
    double max_lon = MAX_LONGITUDE;
    double min_lat = MIN_LATITUDE;
    double max_lat = MAX_LATITUDE;
    
    int bit = 0;
    int index = 0;
    size_t char_count = 0;
    bool is_even = true; /* Start with longitude */
    
    while (char_count < precision) {
        if (is_even) {
            /* Process longitude */
            double mid = (min_lon + max_lon) / 2.0;
            if (point->longitude >= mid) {
                index = (index << 1) | 1;
                min_lon = mid;
            } else {
                index = index << 1;
                max_lon = mid;
            }
        } else {
            /* Process latitude */
            double mid = (min_lat + max_lat) / 2.0;
            if (point->latitude >= mid) {
                index = (index << 1) | 1;
                min_lat = mid;
            } else {
                index = index << 1;
                max_lat = mid;
            }
        }
        
        is_even = !is_even;
        
        if (++bit == BITS_PER_CHAR) {
            result->hash[char_count++] = BASE32_ALPHABET[index];
            index = 0;
            bit = 0;
        }
    }
    
    result->hash[precision] = '\0';
    result->length = precision;
    
    return GEOHASH_OK;
}

GeoHashError geohash_encode_default(const GeoPoint *point, GeoHash *result) {
    return geohash_encode(point, GEOHASH_DEFAULT_PRECISION, result);
}

/* GeoHash decoding */
GeoHashError geohash_get_bounds(const GeoHash *hash, GeoBounds *bounds) {
    if (!hash || !hash->hash || !bounds) {
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    if (!is_valid_geohash_string(hash->hash, hash->length)) {
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    double min_lon = MIN_LONGITUDE;
    double max_lon = MAX_LONGITUDE;
    double min_lat = MIN_LATITUDE;
    double max_lat = MAX_LATITUDE;
    bool is_even = true;
    
    for (size_t i = 0; i < hash->length; i++) {
        int char_index = find_base32_index(hash->hash[i]);
        if (char_index == -1) {
            return GEOHASH_ERROR_INVALID_HASH;
        }
        
        for (int bit = BITS_PER_CHAR - 1; bit >= 0; bit--) {
            int bit_value = GET_BIT(char_index, bit);
            
            if (is_even) {
                /* Process longitude */
                double mid = (min_lon + max_lon) / 2.0;
                if (bit_value) {
                    min_lon = mid;
                } else {
                    max_lon = mid;
                }
            } else {
                /* Process latitude */
                double mid = (min_lat + max_lat) / 2.0;
                if (bit_value) {
                    min_lat = mid;
                } else {
                    max_lat = mid;
                }
            }
            is_even = !is_even;
        }
    }
    
    bounds->min_latitude = min_lat;
    bounds->max_latitude = max_lat;
    bounds->min_longitude = min_lon;
    bounds->max_longitude = max_lon;
    
    return GEOHASH_OK;
}

GeoHashError geohash_decode(const GeoHash *hash, GeoPoint *result) {
    if (!result) {
        return GEOHASH_ERROR_INVALID_POINT;
    }
    
    GeoBounds bounds;
    GeoHashError error = geohash_get_bounds(hash, &bounds);
    if (error != GEOHASH_OK) {
        return error;
    }
    
    result->latitude = (bounds.min_latitude + bounds.max_latitude) / 2.0;
    result->longitude = (bounds.min_longitude + bounds.max_longitude) / 2.0;
    
    return GEOHASH_OK;
}

/* GeoHash adjacency */
GeoHashError geohash_get_adjacent(const GeoHash *hash, GeoDirection direction, GeoHash *result) {
    if (!hash || !hash->hash || !result) {
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    if (!is_valid_geohash_string(hash->hash, hash->length)) {
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    if (direction < GEOHASH_NORTH || direction > GEOHASH_WEST) {
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    size_t hash_len = hash->length;
    char last_char = hash->hash[hash_len - 1];
    int parity = hash_len % 2;
    
    /* Allocate memory for result */
    result->hash = (char *)malloc(hash_len + 1);
    if (!result->hash) {
        return GEOHASH_ERROR_ALLOCATION;
    }
    
    /* Copy parent hash (all but last character) */
    if (hash_len > 1) {
        memcpy(result->hash, hash->hash, hash_len - 1);
    }
    result->length = hash_len;
    
    /* Check if last character is on border */
    if (find_char_index(last_char, border_map[direction][parity]) != -1 && hash_len > 1) {
        /* Need to adjust parent */
        GeoHash parent_hash = {NULL, hash_len - 1};
        parent_hash.hash = (char *)malloc(hash_len);
        if (!parent_hash.hash) {
            free(result->hash);
            return GEOHASH_ERROR_ALLOCATION;
        }
        memcpy(parent_hash.hash, hash->hash, hash_len - 1);
        parent_hash.hash[hash_len - 1] = '\0';
        
        GeoHash adjacent_parent = geohash_create();
        GeoHashError error = geohash_get_adjacent(&parent_hash, direction, &adjacent_parent);
        free(parent_hash.hash);
        
        if (error != GEOHASH_OK) {
            free(result->hash);
            return error;
        }
        
        memcpy(result->hash, adjacent_parent.hash, hash_len - 1);
        geohash_free(&adjacent_parent);
    }
    
    /* Find new last character */
    int new_char_index = find_char_index(last_char, adjacent_map[direction][parity]);
    if (new_char_index == -1) {
        free(result->hash);
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    result->hash[hash_len - 1] = BASE32_ALPHABET[new_char_index];
    result->hash[hash_len] = '\0';
    
    return GEOHASH_OK;
}

GeoHashError geohash_get_all_adjacent(const GeoHash *hash, GeoAdjacent *result) {
    if (!hash || !result) {
        return GEOHASH_ERROR_INVALID_HASH;
    }
    
    GeoHashError error;
    
    /* Get cardinal directions */
    if ((error = geohash_get_adjacent(hash, GEOHASH_NORTH, &result->north)) != GEOHASH_OK) return error;
    if ((error = geohash_get_adjacent(hash, GEOHASH_SOUTH, &result->south)) != GEOHASH_OK) goto cleanup_north;
    if ((error = geohash_get_adjacent(hash, GEOHASH_EAST, &result->east)) != GEOHASH_OK) goto cleanup_south;
    if ((error = geohash_get_adjacent(hash, GEOHASH_WEST, &result->west)) != GEOHASH_OK) goto cleanup_east;
    
    /* Get diagonal directions */
    if ((error = geohash_get_adjacent(&result->north, GEOHASH_EAST, &result->northeast)) != GEOHASH_OK) goto cleanup_west;
    if ((error = geohash_get_adjacent(&result->north, GEOHASH_WEST, &result->northwest)) != GEOHASH_OK) goto cleanup_ne;
    if ((error = geohash_get_adjacent(&result->south, GEOHASH_EAST, &result->southeast)) != GEOHASH_OK) goto cleanup_nw;
    if ((error = geohash_get_adjacent(&result->south, GEOHASH_WEST, &result->southwest)) != GEOHASH_OK) goto cleanup_se;
    
    return GEOHASH_OK;

    /* Cleanup on error */
cleanup_se:
    geohash_free(&result->southeast);
cleanup_nw:
    geohash_free(&result->northwest);
cleanup_ne:
    geohash_free(&result->northeast);
cleanup_west:
    geohash_free(&result->west);
cleanup_east:
    geohash_free(&result->east);
cleanup_south:
    geohash_free(&result->south);
cleanup_north:
    geohash_free(&result->north);
    return error;
}

void geoadjacent_free(GeoAdjacent *adjacent) {
    if (!adjacent) return;
    
    geohash_free(&adjacent->north);
    geohash_free(&adjacent->south);
    geohash_free(&adjacent->east);
    geohash_free(&adjacent->west);
    geohash_free(&adjacent->northeast);
    geohash_free(&adjacent->northwest);
    geohash_free(&adjacent->southeast);
    geohash_free(&adjacent->southwest);
}
