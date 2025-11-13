#define CTEST_IMPLEMENTATION
#include "ctest.h"
#include "data_structure/count_min_sketch.h"

#include <stdlib.h>
#include <string.h>

TEST(CountMinSketch, Init) {
  CountMinSketch cms;

  // Basic initialization
  int result = cms_init(&cms, 100, 5);
  EXPECT_EQ(result, CMS_SUCCESS);
  EXPECT_EQ(cms.width, 100);
  EXPECT_EQ(cms.depth, 5);
  EXPECT_EQ(cms.elements_added, 0);

  cms_destroy(&cms);
}

TEST(CountMinSketch, Destroy) {
  CountMinSketch cms;

  cms_init(&cms, 100, 5);
  int result = cms_destroy(&cms);
  EXPECT_EQ(result, CMS_SUCCESS);

  // After destruction, bins should be NULL
  EXPECT_EQ(cms.bins, NULL);
}

TEST(CountMinSketch, Add) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Basic add (single increment)
  int32_t result = cms_add(&cms, "test_key");
  EXPECT_NE(result, CMS_ERROR);
  EXPECT_EQ(cms.elements_added, 1);

  // Add same key again
  result = cms_add(&cms, "test_key");
  EXPECT_NE(result, CMS_ERROR);
  EXPECT_EQ(cms.elements_added, 2);

  // Add different key
  result = cms_add(&cms, "another_key");
  EXPECT_NE(result, CMS_ERROR);
  EXPECT_EQ(cms.elements_added, 3);

  cms_destroy(&cms);
}

TEST(CountMinSketch, AddInc) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Add with custom increment
  int32_t result = cms_add_inc(&cms, "key1", 5);
  EXPECT_NE(result, CMS_ERROR);
  EXPECT_EQ(cms.elements_added, 5);

  // Add another increment to same key
  result = cms_add_inc(&cms, "key1", 3);
  EXPECT_NE(result, CMS_ERROR);
  EXPECT_EQ(cms.elements_added, 8);

  // Zero increment
  result = cms_add_inc(&cms, "key2", 0);
  EXPECT_NE(result, CMS_ERROR);
  EXPECT_EQ(cms.elements_added, 8);

  // Large increment
  result = cms_add_inc(&cms, "key3", 1000);
  EXPECT_NE(result, CMS_ERROR);
  EXPECT_EQ(cms.elements_added, 1008);

  cms_destroy(&cms);
}

TEST(CountMinSketch, Check) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Check non-existent key
  int32_t count = cms_check(&cms, "nonexistent");
  EXPECT_EQ(count, 0);

  // Add key and check
  cms_add(&cms, "key1");
  count = cms_check(&cms, "key1");
  EXPECT_EQ(count, 1);

  // Add multiple times and check
  cms_add(&cms, "key1");
  cms_add(&cms, "key1");
  count = cms_check(&cms, "key1");
  EXPECT_EQ(count, 3);

  // Check different key
  cms_add(&cms, "key2");
  count = cms_check(&cms, "key2");
  EXPECT_EQ(count, 1);

  cms_destroy(&cms);
}

TEST(CountMinSketch, CheckMin) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Check min is same as check
  cms_add(&cms, "key1");
  cms_add(&cms, "key1");

  int32_t count1 = cms_check(&cms, "key1");
  int32_t count2 = cms_check_min(&cms, "key1");
  EXPECT_EQ(count1, count2);

  cms_destroy(&cms);
}

TEST(CountMinSketch, CheckWithIncrement) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Add with increment and check
  cms_add_inc(&cms, "key1", 10);
  int32_t count = cms_check(&cms, "key1");
  EXPECT_EQ(count, 10);

  // Add more
  cms_add_inc(&cms, "key1", 5);
  count = cms_check(&cms, "key1");
  EXPECT_EQ(count, 15);

  cms_destroy(&cms);
}

TEST(CountMinSketch, Remove) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Add and then remove
  cms_add(&cms, "key1");
  cms_add(&cms, "key1");
  int32_t result = cms_remove(&cms, "key1");
  EXPECT_NE(result, CMS_ERROR);

  int32_t count = cms_check(&cms, "key1");
  EXPECT_EQ(count, 1);

  cms_destroy(&cms);
}

TEST(CountMinSketch, RemoveInc) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Add with increment and remove with increment
  cms_add_inc(&cms, "key1", 10);
  int32_t result = cms_remove_inc(&cms, "key1", 3);
  EXPECT_NE(result, CMS_ERROR);

  int32_t count = cms_check(&cms, "key1");
  EXPECT_EQ(count, 7);

  cms_destroy(&cms);
}

TEST(CountMinSketch, MultipleKeys) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Add multiple different keys
  cms_add(&cms, "key1");
  cms_add(&cms, "key2");
  cms_add(&cms, "key3");
  cms_add(&cms, "key1");
  cms_add(&cms, "key2");

  EXPECT_EQ(cms_check(&cms, "key1"), 2);
  EXPECT_EQ(cms_check(&cms, "key2"), 2);
  EXPECT_EQ(cms_check(&cms, "key3"), 1);
  EXPECT_EQ(cms_check(&cms, "nonexistent"), 0);

  cms_destroy(&cms);
}

TEST(CountMinSketch, EdgeCases) {
  CountMinSketch cms;
  cms_init(&cms, 100, 5);

  // Empty string key
  cms_add(&cms, "");
  EXPECT_EQ(cms_check(&cms, ""), 1);

  // Very long key
  char long_key[1024];
  memset(long_key, 'a', 1023);
  long_key[1023] = '\0';
  cms_add(&cms, long_key);
  EXPECT_EQ(cms_check(&cms, long_key), 1);

  // Keys with special characters
  cms_add(&cms, "key\n\t\r");
  EXPECT_EQ(cms_check(&cms, "key\n\t\r"), 1);

  cms_destroy(&cms);
}

TEST(CountMinSketch, LargeScale) {
  CountMinSketch cms;
  cms_init(&cms, 1000, 10);

  // Add many elements
  for (int i = 0; i < 100; i++) {
    char key[32];
    sprintf(key, "key_%d", i);
    cms_add(&cms, key);
  }

  EXPECT_EQ(cms.elements_added, 100);

  // Verify some keys
  char key[32];
  sprintf(key, "key_%d", 50);
  EXPECT_EQ(cms_check(&cms, key), 1);

  cms_destroy(&cms);
}

CTEST_MAIN()
