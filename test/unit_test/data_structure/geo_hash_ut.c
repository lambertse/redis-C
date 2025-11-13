/**
 * @file geohash_test.c
 * @brief Unit tests for GeoHash encoder, decoder, and neighbor functions
 * 
 * Tests the core functionality of geohash encoding, decoding, and
 * computing adjacent geohashes in cardinal directions.
 */

#define CTEST_IMPLEMENTATION
#include "ctest.h"
#include "data_structure/geo_hash.h"
#include <math.h>
#include <string.h>

/* ============================================================================
 * Test Case Structures
 * ============================================================================ */

/**
 * @struct EncodeTestCase
 * @brief Test case for geohash encoding
 */
typedef struct {
    GeoPoint point;          /**< Input coordinate */
    const char* expected;    /**< Expected geohash prefix */
    size_t precision;        /**< Geohash precision */
} EncodeTestCase;

/**
 * @struct DecodeTestCase
 * @brief Test case for geohash decoding
 */
typedef struct {
    const char* hash;        /**< Input geohash */
    GeoPoint expected;       /**< Expected coordinate */
    double tolerance;        /**< Acceptable error margin */
} DecodeTestCase;

/**
 * @struct AdjacentTestCase
 * @brief Test case for adjacent geohash computation
 */
typedef struct {
    const char* hash;        /**< Base geohash */
    GeoDirection direction;  /**< Direction to neighbor */
    const char* expected;    /**< Expected neighbor geohash */
} AdjacentTestCase;

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/**
 * @brief Compare two strings up to a maximum length
 * 
 * @param str1 First string
 * @param str2 Second string
 * @param len  Maximum characters to compare
 * 
 * @return 0 if equal, non-zero otherwise
 */
static int strncmp_safe(const char* str1, const char* str2, size_t len) {
    if (str1 == NULL || str2 == NULL) {
        return (str1 == str2) ? 0 : -1;
    }
    return strncmp(str1, str2, len);
}

/**
 * @brief Check if two floating-point values are approximately equal
 * 
 * @param actual   Actual value
 * @param expected Expected value
 * @param tolerance Acceptable difference
 * 
 * @return 1 if within tolerance, 0 otherwise
 */
static int approx_equal(double actual, double expected, double tolerance) {
    return fabs(actual - expected) <= tolerance;
}

/**
 * @brief Print GeoPoint for debugging
 */
static void print_geopoint(const char* label, GeoPoint point) {
    printf("%s: (%.4f, %.4f)\n", label, point.latitude, point.longitude);
}

/**
 * @brief Create a GeoHash from a string for testing
 */
static GeoHash create_test_geohash(const char* hash_str) {
    GeoHash hash;
    hash.length = strlen(hash_str);
    hash.hash = (char*)malloc(hash.length + 1);
    strcpy(hash.hash, hash_str);
    return hash;
}

/* ============================================================================
 * Encode Tests
 * ============================================================================ */

/**
 * @brief Test encoding of known geographic coordinates
 * 
 * Verifies that geographic coordinates are correctly encoded into
 * their corresponding geohash representations.
 */
TEST(GeoHashUT, EncodeBasic) {
    EncodeTestCase testCases[] = {
        {geopoint_create(37.7749, -122.4194), "9q8yy", 5},    /* San Francisco */
        {geopoint_create(40.7128, -74.0060), "dr5re", 5},     /* New York */
        {geopoint_create(51.5074, -0.1278), "gcpvj", 5},      /* London */
        {geopoint_create(-33.8688, 151.2093), "r3gx2", 5},    /* Sydney */
        {geopoint_create(35.6895, 139.6917), "xn774", 5},     /* Tokyo */
    };

    int num_cases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_cases; ++i) {
        GeoHash result = geohash_create();
        GeoHashError error = geohash_encode(&testCases[i].point, 
                                            testCases[i].precision, 
                                            &result);

        EXPECT_EQ(error, GEOHASH_OK);
        EXPECT_NE(result.hash, NULL);
        EXPECT_EQ(strncmp_safe(result.hash, testCases[i].expected, 
                               strlen(testCases[i].expected)), 0);
        
        geohash_free(&result);
    }
}

/**
 * @brief Test encoding of pole coordinates
 * 
 * Verifies correct encoding of extreme latitude values (poles)
 * and extreme longitude values.
 */
TEST(GeoHashUT, EncodePoles) {
    EncodeTestCase testCases[] = {
        {geopoint_create(90, -180), "bpbpb", 5},              /* North Pole, Western Hemisphere */
        {geopoint_create(-90, 180), "pbpbp", 5},              /* South Pole, Eastern Hemisphere */
        {geopoint_create(90, 180), "zzzzz", 5},               /* North Pole, Eastern Hemisphere */
        {geopoint_create(-90, -180), "00000", 5},             /* South Pole, Western Hemisphere */
    };

    int num_cases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_cases; ++i) {
        GeoHash result = geohash_create();
        GeoHashError error = geohash_encode(&testCases[i].point, 
                                            testCases[i].precision, 
                                            &result);

        EXPECT_EQ(error, GEOHASH_OK);
        EXPECT_NE(result.hash, NULL);
        EXPECT_EQ(strncmp_safe(result.hash, testCases[i].expected,
                               strlen(testCases[i].expected)), 0);
        
        geohash_free(&result);
    }
}

/**
 * @brief Test encoding with invalid coordinates
 */
TEST(GeoHashUT, EncodeInvalidCoordinates) {
    GeoPoint invalid_points[] = {
        geopoint_create(100.0, 0.0),      /* Latitude > 90 */
        geopoint_create(-100.0, 0.0),     /* Latitude < -90 */
        geopoint_create(0.0, 200.0),      /* Longitude > 180 */
        geopoint_create(0.0, -200.0),     /* Longitude < -180 */
    };

    int num_cases = sizeof(invalid_points) / sizeof(invalid_points[0]);

    for (int i = 0; i < num_cases; ++i) {
        GeoHash result = geohash_create();
        GeoHashError error = geohash_encode(&invalid_points[i], 9, &result);

        EXPECT_EQ(error, GEOHASH_ERROR_INVALID_POINT);
        
        geohash_free(&result);
    }
}

/**
 * @brief Test encoding with various precision levels
 */
TEST(GeoHashUT, EncodePrecision) {
    GeoPoint point = geopoint_create(37.7749, -122.4194);
    size_t precisions[] = {1, 3, 5, 9, 12};
    int num_precisions = sizeof(precisions) / sizeof(precisions[0]);

    for (int i = 0; i < num_precisions; ++i) {
        GeoHash result = geohash_create();
        GeoHashError error = geohash_encode(&point, precisions[i], &result);

        EXPECT_EQ(error, GEOHASH_OK);
        EXPECT_EQ(result.length, precisions[i]);
        
        geohash_free(&result);
    }
}

/* ============================================================================
 * Decode Tests
 * ============================================================================ */

/**
 * @brief Test decoding of known geohashes
 * 
 * Verifies that geohash strings are correctly decoded into their
 * corresponding geographic coordinates.
 */
TEST(GeoHashUT, DecodeBasic) {
    DecodeTestCase testCases[] = {
        {"9q8yy", geopoint_create(37.7749, -122.4194), 0.1},    /* San Francisco */
        {"dr5re", geopoint_create(40.7128, -74.0060), 0.1},     /* New York */
        {"gcpvj", geopoint_create(51.5074, -0.1278), 0.1},      /* London */
        {"r3gx2", geopoint_create(-33.8688, 151.2093), 0.1},    /* Sydney */
        {"xn774", geopoint_create(35.6895, 139.6917), 0.1},     /* Tokyo */
    };

    int num_cases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_cases; ++i) {
        GeoHash hash = create_test_geohash(testCases[i].hash);
        GeoPoint actual;
        GeoHashError error = geohash_decode(&hash, &actual);

        EXPECT_EQ(error, GEOHASH_OK);
        EXPECT_TRUE(approx_equal(actual.latitude, testCases[i].expected.latitude,
                                testCases[i].tolerance));
        EXPECT_TRUE(approx_equal(actual.longitude, testCases[i].expected.longitude,
                                testCases[i].tolerance));
        
        geohash_free(&hash);
    }
}

/**
 * @brief Test decoding of extreme geohashes
 * 
 * Verifies decoding of geohashes representing pole coordinates.
 */
TEST(GeoHashUT, DecodePoles) {
    DecodeTestCase testCases[] = {
        {"bpbpb", geopoint_create(90, -180), 1.0},
        {"pbpbp", geopoint_create(-90, 180), 1.0},
        {"zzzzz", geopoint_create(90, 180), 1.0},
        {"00000", geopoint_create(-90, -180), 1.0},
    };

    int num_cases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_cases; ++i) {
        GeoHash hash = create_test_geohash(testCases[i].hash);
        GeoPoint actual;
        GeoHashError error = geohash_decode(&hash, &actual);

        EXPECT_EQ(error, GEOHASH_OK);
        EXPECT_TRUE(approx_equal(actual.latitude, testCases[i].expected.latitude,
                                testCases[i].tolerance));
        EXPECT_TRUE(approx_equal(actual.longitude, testCases[i].expected.longitude,
                                testCases[i].tolerance));
        
        geohash_free(&hash);
    }
}

/**
 * @brief Test encode-decode roundtrip
 */
TEST(GeoHashUT, EncodeDecodeRoundtrip) {
    GeoPoint original = geopoint_create(37.7749, -122.4194);
    GeoHash encoded = geohash_create();
    GeoPoint decoded;

    GeoHashError error = geohash_encode(&original, 9, &encoded);
    EXPECT_EQ(error, GEOHASH_OK);

    error = geohash_decode(&encoded, &decoded);
    EXPECT_EQ(error, GEOHASH_OK);

    /* Should be close but not exact due to geohash precision */
    EXPECT_TRUE(approx_equal(decoded.latitude, original.latitude, 0.001));
    EXPECT_TRUE(approx_equal(decoded.longitude, original.longitude, 0.001));

    geohash_free(&encoded);
}

/* ============================================================================
 * Adjacent Tests
 * ============================================================================ */

/**
 * @brief Test adjacent geohash computation returning all 8 neighbors
 * 
 * Verifies that geohash_get_all_adjacent() correctly returns a GeoAdjacent structure
 * containing all 8 neighbor geohashes (4 cardinal + 4 diagonal directions).
 */
TEST(GeoHashUT, AdjacentAllDirections) {
    const char* base_hash_str = "9q8yy";
    GeoHash base_hash = create_test_geohash(base_hash_str);
    GeoAdjacent neighbors;

    GeoHashError error = geohash_get_all_adjacent(&base_hash, &neighbors);
    EXPECT_EQ(error, GEOHASH_OK);

    /* Verify all neighbors are allocated and not NULL */
    EXPECT_NE(neighbors.north.hash, NULL);
    EXPECT_NE(neighbors.south.hash, NULL);
    EXPECT_NE(neighbors.east.hash, NULL);
    EXPECT_NE(neighbors.west.hash, NULL);
    EXPECT_NE(neighbors.northeast.hash, NULL);
    EXPECT_NE(neighbors.northwest.hash, NULL);
    EXPECT_NE(neighbors.southeast.hash, NULL);
    EXPECT_NE(neighbors.southwest.hash, NULL);

    /* Verify all neighbors are different from base hash */
    EXPECT_NE(strcmp(neighbors.north.hash, base_hash_str), 0);
    EXPECT_NE(strcmp(neighbors.south.hash, base_hash_str), 0);
    EXPECT_NE(strcmp(neighbors.east.hash, base_hash_str), 0);
    EXPECT_NE(strcmp(neighbors.west.hash, base_hash_str), 0);

    geoadjacent_free(&neighbors);
    geohash_free(&base_hash);
}

/**
 * @brief Test single direction geohash_get_adjacent function
 * 
 * Verifies that geohash_get_adjacent() correctly computes a single neighbor
 * in the specified cardinal direction.
 */
TEST(GeoHashUT, AdjacentSingleDirection) {
    const char* base_hash_str = "9q8yy";
    GeoHash base_hash = create_test_geohash(base_hash_str);
    
    GeoHash north = geohash_create();
    GeoHash south = geohash_create();
    GeoHash east = geohash_create();
    GeoHash west = geohash_create();

    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_NORTH, &north), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_SOUTH, &south), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_EAST, &east), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_WEST, &west), GEOHASH_OK);

    /* Verify neighbors are not NULL */
    EXPECT_NE(north.hash, NULL);
    EXPECT_NE(south.hash, NULL);
    EXPECT_NE(east.hash, NULL);
    EXPECT_NE(west.hash, NULL);

    /* Verify neighbors are different from base */
    EXPECT_NE(strcmp(north.hash, base_hash_str), 0);
    EXPECT_NE(strcmp(south.hash, base_hash_str), 0);
    EXPECT_NE(strcmp(east.hash, base_hash_str), 0);
    EXPECT_NE(strcmp(west.hash, base_hash_str), 0);

    geohash_free(&north);
    geohash_free(&south);
    geohash_free(&east);
    geohash_free(&west);
    geohash_free(&base_hash);
}

/**
 * @brief Test consistency between single and all-directions functions
 * 
 * Verifies that geohash_get_adjacent() and geohash_get_all_adjacent() produce
 * consistent results for the same direction.
 */
TEST(GeoHashUT, AdjacentConsistency) {
    const char* base_hash_str = "9q8yy";
    GeoHash base_hash = create_test_geohash(base_hash_str);
    
    /* Get neighbors using all-directions function */
    GeoAdjacent all_neighbors;
    EXPECT_EQ(geohash_get_all_adjacent(&base_hash, &all_neighbors), GEOHASH_OK);
    
    /* Get neighbors using single-direction function */
    GeoHash north = geohash_create();
    GeoHash south = geohash_create();
    GeoHash east = geohash_create();
    GeoHash west = geohash_create();

    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_NORTH, &north), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_SOUTH, &south), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_EAST, &east), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_WEST, &west), GEOHASH_OK);

    /* Verify consistency */
    EXPECT_EQ(strcmp(north.hash, all_neighbors.north.hash), 0);
    EXPECT_EQ(strcmp(south.hash, all_neighbors.south.hash), 0);
    EXPECT_EQ(strcmp(east.hash, all_neighbors.east.hash), 0);
    EXPECT_EQ(strcmp(west.hash, all_neighbors.west.hash), 0);

    geohash_free(&north);
    geohash_free(&south);
    geohash_free(&east);
    geohash_free(&west);
    geoadjacent_free(&all_neighbors);
    geohash_free(&base_hash);
}

/**
 * @brief Test diagonal neighbors composition
 * 
 * Verifies that diagonal neighbors are correctly computed as combinations
 * of cardinal directions.
 */
TEST(GeoHashUT, AdjacentDiagonalComposition) {
    const char* base_hash_str = "9q8yy";
    GeoHash base_hash = create_test_geohash(base_hash_str);
    GeoAdjacent neighbors;
    EXPECT_EQ(geohash_get_all_adjacent(&base_hash, &neighbors), GEOHASH_OK);

    /* Get intermediate neighbors */
    GeoHash north = geohash_create();
    GeoHash south = geohash_create();
    GeoHash east = geohash_create();
    GeoHash west = geohash_create();

    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_NORTH, &north), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_SOUTH, &south), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_EAST, &east), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_WEST, &west), GEOHASH_OK);

    /* Compute diagonals using composition */
    GeoHash northeast_via_north = geohash_create();
    GeoHash northeast_via_east = geohash_create();
    GeoHash southeast_via_south = geohash_create();
    GeoHash southeast_via_east = geohash_create();
    GeoHash southwest_via_south = geohash_create();
    GeoHash southwest_via_west = geohash_create();
    GeoHash northwest_via_north = geohash_create();
    GeoHash northwest_via_west = geohash_create();

    EXPECT_EQ(geohash_get_adjacent(&north, GEOHASH_EAST, &northeast_via_north), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&east, GEOHASH_NORTH, &northeast_via_east), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&south, GEOHASH_EAST, &southeast_via_south), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&east, GEOHASH_SOUTH, &southeast_via_east), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&south, GEOHASH_WEST, &southwest_via_south), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&west, GEOHASH_SOUTH, &southwest_via_west), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&north, GEOHASH_WEST, &northwest_via_north), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&west, GEOHASH_NORTH, &northwest_via_west), GEOHASH_OK);

    /* Verify diagonal neighbors match both composition methods */
    EXPECT_EQ(strcmp(neighbors.northeast.hash, northeast_via_north.hash), 0);
    EXPECT_EQ(strcmp(northeast_via_north.hash, northeast_via_east.hash), 0);

    EXPECT_EQ(strcmp(neighbors.southeast.hash, southeast_via_south.hash), 0);
    EXPECT_EQ(strcmp(southeast_via_south.hash, southeast_via_east.hash), 0);

    EXPECT_EQ(strcmp(neighbors.southwest.hash, southwest_via_south.hash), 0);
    EXPECT_EQ(strcmp(southwest_via_south.hash, southwest_via_west.hash), 0);

    EXPECT_EQ(strcmp(neighbors.northwest.hash, northwest_via_north.hash), 0);
    EXPECT_EQ(strcmp(northwest_via_north.hash, northwest_via_west.hash), 0);

    /* Cleanup */
    geohash_free(&north);
    geohash_free(&south);
    geohash_free(&east);
    geohash_free(&west);
    geohash_free(&northeast_via_north);
    geohash_free(&northeast_via_east);
    geohash_free(&southeast_via_south);
    geohash_free(&southeast_via_east);
    geohash_free(&southwest_via_south);
    geohash_free(&southwest_via_west);
    geohash_free(&northwest_via_north);
    geohash_free(&northwest_via_west);
    geoadjacent_free(&neighbors);
    geohash_free(&base_hash);
}

/**
 * @brief Test commutativity of adjacent operations
 * 
 * Verifies that combining directions in different orders produces
 * the same result (commutativity).
 */
TEST(GeoHashUT, AdjacentCommutativity) {
    const char* base_hash_str = "9q8yy";
    GeoHash base_hash = create_test_geohash(base_hash_str);

    GeoHash north = geohash_create();
    GeoHash east = geohash_create();
    GeoHash south = geohash_create();
    GeoHash west = geohash_create();

    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_NORTH, &north), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_EAST, &east), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_SOUTH, &south), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_WEST, &west), GEOHASH_OK);

    /* North-East vs East-North */
    GeoHash ne_via_north = geohash_create();
    GeoHash ne_via_east = geohash_create();
    EXPECT_EQ(geohash_get_adjacent(&north, GEOHASH_EAST, &ne_via_north), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&east, GEOHASH_NORTH, &ne_via_east), GEOHASH_OK);
    EXPECT_EQ(strcmp(ne_via_north.hash, ne_via_east.hash), 0);

    /* South-West vs West-South */
    GeoHash sw_via_south = geohash_create();
    GeoHash sw_via_west = geohash_create();
    EXPECT_EQ(geohash_get_adjacent(&south, GEOHASH_WEST, &sw_via_south), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&west, GEOHASH_SOUTH, &sw_via_west), GEOHASH_OK);
    EXPECT_EQ(strcmp(sw_via_south.hash, sw_via_west.hash), 0);

    /* Cleanup */
    geohash_free(&north);
    geohash_free(&east);
    geohash_free(&south);
    geohash_free(&west);
    geohash_free(&ne_via_north);
    geohash_free(&ne_via_east);
    geohash_free(&sw_via_south);
    geohash_free(&sw_via_west);
    geohash_free(&base_hash);
}

/**
 * @brief Test roundtrip: opposite neighbors cancel out
 * 
 * Verifies that moving to an adjacent cell and then moving in the
 * opposite direction returns to the original geohash.
 */
TEST(GeoHashUT, AdjacentRoundtrip) {
    const char* base_hash_str = "9q8yy";
    GeoHash base_hash = create_test_geohash(base_hash_str);

    /* North then South */
    GeoHash north = geohash_create();
    GeoHash back_south = geohash_create();
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_NORTH, &north), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&north, GEOHASH_SOUTH, &back_south), GEOHASH_OK);
    EXPECT_EQ(strcmp(back_south.hash, base_hash_str), 0);

    /* East then West */
    GeoHash east = geohash_create();
    GeoHash back_west = geohash_create();
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_EAST, &east), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&east, GEOHASH_WEST, &back_west), GEOHASH_OK);
    EXPECT_EQ(strcmp(back_west.hash, base_hash_str), 0);

    /* South then North */
    GeoHash south = geohash_create();
    GeoHash back_north = geohash_create();
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_SOUTH, &south), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&south, GEOHASH_NORTH, &back_north), GEOHASH_OK);
    EXPECT_EQ(strcmp(back_north.hash, base_hash_str), 0);

    /* West then East */
    GeoHash west = geohash_create();
    GeoHash back_east = geohash_create();
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_WEST, &west), GEOHASH_OK);
    EXPECT_EQ(geohash_get_adjacent(&west, GEOHASH_EAST, &back_east), GEOHASH_OK);
    EXPECT_EQ(strcmp(back_east.hash, base_hash_str), 0);

    /* Cleanup */
    geohash_free(&north);
    geohash_free(&back_south);
    geohash_free(&east);
    geohash_free(&back_west);
    geohash_free(&south);
    geohash_free(&back_north);
    geohash_free(&west);
    geohash_free(&back_east);
    geohash_free(&base_hash);
}

/**
 * @brief Test adjacent with various geohash precisions
 * 
 * Verifies that adjacent computation maintains geohash length
 * across different precision levels.
 */
TEST(GeoHashUT, AdjacentPrecision) {
    const char* hashes[] = {"9", "9q", "9q8", "9q8y", "9q8yy"};
    int num_hashes = sizeof(hashes) / sizeof(hashes[0]);

    for (int i = 0; i < num_hashes; ++i) {
        GeoHash hash = create_test_geohash(hashes[i]);
        GeoAdjacent neighbors;
        
        EXPECT_EQ(geohash_get_all_adjacent(&hash, &neighbors), GEOHASH_OK);
        
        size_t expected_len = strlen(hashes[i]);

        EXPECT_EQ(neighbors.north.length, expected_len);
        EXPECT_EQ(neighbors.south.length, expected_len);
        EXPECT_EQ(neighbors.east.length, expected_len);
        EXPECT_EQ(neighbors.west.length, expected_len);
        EXPECT_EQ(neighbors.northeast.length, expected_len);

        geoadjacent_free(&neighbors);
        geohash_free(&hash);
    }
}

/* ============================================================================
 * Bounds Tests
 * ============================================================================ */

/**
 * @brief Test getting bounds from a geohash
 */
TEST(GeoHashUT, GetBounds) {
    const char* hash_str = "9q8yy";
    GeoHash hash = create_test_geohash(hash_str);
    GeoBounds bounds;

    GeoHashError error = geohash_get_bounds(&hash, &bounds);
    EXPECT_EQ(error, GEOHASH_OK);

    /* Bounds should be valid */
    EXPECT_TRUE(bounds.min_latitude <= bounds.max_latitude);
    EXPECT_TRUE(bounds.min_longitude <= bounds.max_longitude);

    /* San Francisco should be within these bounds */
    EXPECT_TRUE(37.7749 >= bounds.min_latitude);
    EXPECT_TRUE(37.7749 <= bounds.max_latitude);
    EXPECT_TRUE(-122.4194 >= bounds.min_longitude);
    EXPECT_TRUE(-122.4194 <= bounds.max_longitude);

    geohash_free(&hash);
}

/* ============================================================================
 * Error Handling Tests
 * ============================================================================ */

/**
 * @brief Test error handling for NULL pointers
 */
TEST(GeoHashUT, ErrorHandlingNullPointers) {
    GeoPoint point = geopoint_create(37.7749, -122.4194);
    GeoHash hash = geohash_create();

    /* Encode with NULL result */
    EXPECT_EQ(geohash_encode(&point, 9, NULL), GEOHASH_ERROR_INVALID_POINT);

    /* Encode with NULL point */
    EXPECT_EQ(geohash_encode(NULL, 9, &hash), GEOHASH_ERROR_INVALID_POINT);

    /* Decode with NULL hash */
    GeoPoint decoded_point;
    EXPECT_EQ(geohash_decode(NULL, &decoded_point), GEOHASH_ERROR_INVALID_HASH);

    /* Decode with NULL result */
    GeoHash valid_hash = create_test_geohash("9q8yy");
    EXPECT_EQ(geohash_decode(&valid_hash, NULL), GEOHASH_ERROR_INVALID_POINT);

    geohash_free(&valid_hash);
    geohash_free(&hash);
}

/**
 * @brief Test error handling for invalid precision
 */
TEST(GeoHashUT, ErrorHandlingInvalidPrecision) {
    GeoPoint point = geopoint_create(37.7749, -122.4194);
    GeoHash hash = geohash_create();

    /* Precision 0 */
    EXPECT_EQ(geohash_encode(&point, 0, &hash), GEOHASH_ERROR_INVALID_PRECISION);

    /* Precision too large */
    EXPECT_EQ(geohash_encode(&point, 20, &hash), GEOHASH_ERROR_INVALID_PRECISION);

    geohash_free(&hash);
}

/**
 * @brief Test error strings
 */
TEST(GeoHashUT, ErrorStrings) {
    EXPECT_NE(geohash_error_string(GEOHASH_OK), NULL);
    EXPECT_NE(geohash_error_string(GEOHASH_ERROR_INVALID_POINT), NULL);
    EXPECT_NE(geohash_error_string(GEOHASH_ERROR_INVALID_HASH), NULL);
    EXPECT_NE(geohash_error_string(GEOHASH_ERROR_ALLOCATION), NULL);
    EXPECT_NE(geohash_error_string(GEOHASH_ERROR_INVALID_PRECISION), NULL);
}

/* ============================================================================
 * Utility Function Tests
 * ============================================================================ */

/**
 * @brief Test GeoPoint validation
 */
TEST(GeoHashUT, GeoPointValidation) {
    /* Valid points */
    GeoPoint valid1 = geopoint_create(0.0, 0.0);
    GeoPoint valid2 = geopoint_create(90.0, 180.0);
    GeoPoint valid3 = geopoint_create(-90.0, -180.0);

    EXPECT_TRUE(geopoint_is_valid(&valid1));
    EXPECT_TRUE(geopoint_is_valid(&valid2));
    EXPECT_TRUE(geopoint_is_valid(&valid3));

    /* Invalid points */
    GeoPoint invalid1 = geopoint_create(91.0, 0.0);
    GeoPoint invalid2 = geopoint_create(-91.0, 0.0);
    GeoPoint invalid3 = geopoint_create(0.0, 181.0);
    GeoPoint invalid4 = geopoint_create(0.0, -181.0);

    EXPECT_FALSE(geopoint_is_valid(&invalid1));
    EXPECT_FALSE(geopoint_is_valid(&invalid2));
    EXPECT_FALSE(geopoint_is_valid(&invalid3));
    EXPECT_FALSE(geopoint_is_valid(&invalid4));

    /* NULL pointer */
    EXPECT_FALSE(geopoint_is_valid(NULL));
}

/**
 * @brief Test GeoHash copy function
 */
TEST(GeoHashUT, GeoHashCopy) {
    GeoHash original = create_test_geohash("9q8yy");
    GeoHash copy = geohash_create();

    GeoHashError error = geohash_copy(&original, &copy);
    EXPECT_EQ(error, GEOHASH_OK);

    /* Verify copy matches original */
    EXPECT_EQ(copy.length, original.length);
    EXPECT_EQ(strcmp(copy.hash, original.hash), 0);

    /* Verify it's a real copy (different memory) */
    EXPECT_NE(copy.hash, original.hash);

    geohash_free(&original);
    geohash_free(&copy);
}

/**
 * @brief Test default precision encoding
 */
TEST(GeoHashUT, EncodeDefaultPrecision) {
    GeoPoint point = geopoint_create(37.7749, -122.4194);
    GeoHash hash = geohash_create();

    GeoHashError error = geohash_encode_default(&point, &hash);
    EXPECT_EQ(error, GEOHASH_OK);
    EXPECT_EQ(hash.length, GEOHASH_DEFAULT_PRECISION);

    geohash_free(&hash);
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

/**
 * @brief Test complete workflow: encode -> get neighbors -> decode
 */
TEST(GeoHashUT, IntegrationWorkflow) {
    /* Start with a point */
    GeoPoint original = geopoint_create(37.7749, -122.4194);

    /* Encode it */
    GeoHash encoded = geohash_create();
    EXPECT_EQ(geohash_encode(&original, 9, &encoded), GEOHASH_OK);

    /* Get all neighbors */
    GeoAdjacent neighbors;
    EXPECT_EQ(geohash_get_all_adjacent(&encoded, &neighbors), GEOHASH_OK);

    /* Decode all neighbors */
    GeoPoint north_point, south_point, east_point, west_point;
    EXPECT_EQ(geohash_decode(&neighbors.north, &north_point), GEOHASH_OK);
    EXPECT_EQ(geohash_decode(&neighbors.south, &south_point), GEOHASH_OK);
    EXPECT_EQ(geohash_decode(&neighbors.east, &east_point), GEOHASH_OK);
    EXPECT_EQ(geohash_decode(&neighbors.west, &west_point), GEOHASH_OK);

    /* North should have higher latitude */
    EXPECT_TRUE(north_point.latitude > original.latitude);

    /* South should have lower latitude */
    EXPECT_TRUE(south_point.latitude < original.latitude);

    /* East should have higher longitude */
    EXPECT_TRUE(east_point.longitude > original.longitude);

    /* West should have lower longitude */
    EXPECT_TRUE(west_point.longitude < original.longitude);

    /* Cleanup */
    geohash_free(&encoded);
    geoadjacent_free(&neighbors);
}

/**
 * @brief Test memory management with multiple operations
 */
TEST(GeoHashUT, MemoryManagement) {
    /* Create and destroy many geohashes */
    for (int i = 0; i < 100; i++) {
        GeoPoint point = geopoint_create(37.7749 + i * 0.01, -122.4194 + i * 0.01);
        GeoHash hash = geohash_create();
        
        EXPECT_EQ(geohash_encode(&point, 9, &hash), GEOHASH_OK);
        
        GeoAdjacent neighbors;
        EXPECT_EQ(geohash_get_all_adjacent(&hash, &neighbors), GEOHASH_OK);
        
        geoadjacent_free(&neighbors);
        geohash_free(&hash);
    }
}

/**
 * @brief Test that adjacent cells share borders
 */
TEST(GeoHashUT, AdjacentCellsShareBorders) {
    const char* base_hash_str = "9q8yy";
    GeoHash base_hash = create_test_geohash(base_hash_str);
    
    /* Get base bounds */
    GeoBounds base_bounds;
    EXPECT_EQ(geohash_get_bounds(&base_hash, &base_bounds), GEOHASH_OK);
    
    /* Get north neighbor */
    GeoHash north = geohash_create();
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_NORTH, &north), GEOHASH_OK);
    
    GeoBounds north_bounds;
    EXPECT_EQ(geohash_get_bounds(&north, &north_bounds), GEOHASH_OK);
    
    /* North neighbor's southern edge should match base's northern edge */
    EXPECT_TRUE(approx_equal(north_bounds.min_latitude, 
                             base_bounds.max_latitude, 0.000001));
    
    /* Get east neighbor */
    GeoHash east = geohash_create();
    EXPECT_EQ(geohash_get_adjacent(&base_hash, GEOHASH_EAST, &east), GEOHASH_OK);
    
    GeoBounds east_bounds;
    EXPECT_EQ(geohash_get_bounds(&east, &east_bounds), GEOHASH_OK);
    
    /* East neighbor's western edge should match base's eastern edge */
    EXPECT_TRUE(approx_equal(east_bounds.min_longitude, 
                             base_bounds.max_longitude, 0.000001));
    
    /* Cleanup */
    geohash_free(&base_hash);
    geohash_free(&north);
    geohash_free(&east);
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */

CTEST_MAIN()
