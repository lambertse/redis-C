/*
MIT License

Copyright (c) 2024 Tomasz Stanisławski

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef CTEST_H
#define CTEST_H __FILE__

#include <stdbool.h>

void ctest_fail_test(void);
void ctest_drop_test(const char *fpath, int line);
void ctest_skip_test(void);
int ctest_failed(void);
int ctest_main(int * argc, char * argv[]);
void ctest_log(const char * fmt, ...) __attribute__ ((format (printf, 1, 2)));

enum ctest_status {
    CTEST_UNKNOWN, // must be first
    CTEST_RUNNING,
    CTEST_SUCCESS,
    CTEST_SKIPPED,
    CTEST_FAILURE,
};

typedef struct ctest {
    const char * name;
    void (*_init)(void);
    void (*_exec)(void);
    void (*_drop)(void);
    void  *_data;
    struct ctest * _next;
    enum ctest_status _status;
} ctest;

void ctest_register(ctest *);

/**
 * @brief Add a test case within a test suite. Parameters must be expanded.
 */
#define CTEST__TEST(tsuite, tcase) \
    static void tsuite ## tcase(void);            \
    __attribute__((constructor))                  \
    static void tsuite ## tcase ## __ctor(void) { \
        static ctest instance = {                 \
            .name = #tsuite "." #tcase,           \
            ._exec = tsuite ## tcase,             \
        };                                        \
        ctest_register(&instance);                \
    }                                             \
    static void tsuite ## tcase(void)

/**
 * @brief Add a test case within a test suite. Parameters can be macros.
 *
 * @param tsuite a name of the test suite
 * @param tcase a name of the test case with a suite
 */
#define CTEST_TEST(test_suite, test_case) \
    CTEST__TEST(test_suite, test_case)

/**
 * @brief Add a test case within a test fixture. Parameters must be expanded.
 */
#define CTEST__TEST_F(tfixture, tcase) \
    static tfixture tfixture ## __data;             \
    /* tentative declarations */                    \
    static void (*tfixture ## __init)(tfixture*);   \
    static void (*tfixture ## __drop)(tfixture*);   \
                                                    \
    static void tfixture ## tcase ## __init(void) { \
        if (tfixture ## __init)                     \
            tfixture ## __init(&tfixture ## __data);\
    }                                               \
    static void tfixture ## tcase(tfixture *);      \
    static void tfixture ## tcase ## __exec(void) { \
        tfixture ## tcase(&tfixture ## __data);     \
    }                                               \
    static void tfixture ## tcase ## __drop(void) { \
        if (tfixture ## __drop)                     \
            tfixture ## __drop(&tfixture ## __data);\
    }                                               \
    __attribute__((constructor))                    \
    static void tfixture ## tcase ## __ctor(void) { \
        static ctest instance = {                   \
            .name = #tfixture "." #tcase,           \
            ._init = tfixture ## tcase ## __init,   \
            ._exec = tfixture ## tcase ## __exec,   \
            ._drop = tfixture ## tcase ## __drop,   \
        };                                          \
        ctest_register(&instance);                  \
    }                                               \
    static void tfixture ## tcase(tfixture * self __attribute__((unused)))

#define CTEST_TEST_F_INIT(tfixture) \
    static void tfixture ## __init_function(tfixture*); \
    static void (*tfixture ## __init)(tfixture*) =      \
        tfixture ## __init_function;                    \
    static void tfixture ## __init_function(tfixture * self __attribute__((unused)))

#define CTEST_TEST_F_DROP(tfixture) \
    static void tfixture ## __drop_function(tfixture*); \
    static void (*tfixture ## __drop)(tfixture*) =      \
        tfixture ## __drop_function;                    \
    static void tfixture ## __drop_function(tfixture * self __attribute__((unused)))

/**
 * @brief Add a test case within a test fixture. Parameters can be macros.
 *
 * @param tfixture a type name of the test fixture
 * @param tcase a name of the test case with a fixture
 */
#define CTEST_TEST_F(test_fixture, test_case) \
    CTEST__TEST_F(test_fixture, test_case)

#define CTEST_FAIL() ctest_drop_test(__FILE__, __LINE__)
#define CTEST_SKIP() ctest_skip_test()

void ctest__cleanup(const int *);

#define ASSERT__WRAP(...) \
    for(int _armed __attribute__((cleanup(ctest__cleanup))) = 0; \
        !_armed && !(__VA_ARGS__);                               \
        _armed = 1)

#define EXPECT__WRAP(...) \
    if (__VA_ARGS__); else

#define CTEST_MAIN() \
int main(int argc, char *argv[]) { \
    return ctest_main(&argc, argv); \
}
#define CTEST_LOG(...) ctest_log(__VA_ARGS__)

enum ctest__cmp {
    CTEST__CMP_EQ,
    CTEST__CMP_NE,
    CTEST__CMP_LT,
    CTEST__CMP_LE,
    CTEST__CMP_GT,
    CTEST__CMP_GE,
};

int ctest__cmp_signed(const char *, int, long long signed, const char *,
                       enum ctest__cmp, long long signed, const char *);
int ctest__cmp_unsigned(const char *, int, long long unsigned, const char *,
                       enum ctest__cmp, long long unsigned, const char *);
int ctest__cmp_double(const char *, int, double, const char *,
                       enum ctest__cmp, double, const char *);
int ctest__cmp_str(const char *, int, const char *, const char *,
                       enum ctest__cmp, const char *, const char *);
int ctest__cmp_ptr(const char *, int, const volatile void *, const char *,
                       enum ctest__cmp, const volatile void *, const char *);
int ctest__check_bool(const char *, int, _Bool, const char *, _Bool);
int ctest__check_near(const char *, int, double, const char *, double, const char *, double);

#define CTEST_ASSERT_TRUE(pred) \
    ASSERT__WRAP(ctest__check_bool(__FILE__, __LINE__, (pred), #pred, 1))
#define CTEST_ASSERT_FALSE(pred) \
    ASSERT__WRAP(ctest__check_bool(__FILE__, __LINE__, (pred), #pred, 0))
#define CTEST_EXPECT_TRUE(pred) \
    EXPECT__WRAP(ctest__check_bool(__FILE__, __LINE__, (pred), #pred, 1))
#define CTEST_EXPECT_FALSE(pred) \
    EXPECT__WRAP(ctest__check_bool(__FILE__, __LINE__, (pred), #pred, 0))

#define CTEST__CMP(a, cmp, b)                 \
_Generic(1 ? (a) : (b)                        \
    , _Bool: ctest__cmp_unsigned              \
    , char: ctest__cmp_signed                 \
    , signed char: ctest__cmp_signed          \
    , short: ctest__cmp_signed                \
    , int: ctest__cmp_signed                  \
    , long: ctest__cmp_signed                 \
    , long long: ctest__cmp_signed            \
    , unsigned char: ctest__cmp_unsigned      \
    , unsigned short: ctest__cmp_unsigned     \
    , unsigned int: ctest__cmp_unsigned       \
    , unsigned long: ctest__cmp_unsigned      \
    , unsigned long long: ctest__cmp_unsigned \
    , float: ctest__cmp_double                \
    , double: ctest__cmp_double               \
    , default: ctest__cmp_ptr                 \
)(__FILE__, __LINE__, a, #a, CTEST__CMP_ ## cmp, b, #b)

#define CTEST_EXPECT_EQ(a, b) EXPECT__WRAP(CTEST__CMP(a, EQ, b))
#define CTEST_ASSERT_EQ(a, b) ASSERT__WRAP(CTEST__CMP(a, EQ, b))
#define CTEST_EXPECT_NE(a, b) EXPECT__WRAP(CTEST__CMP(a, NE, b))
#define CTEST_ASSERT_NE(a, b) ASSERT__WRAP(CTEST__CMP(a, NE, b))
#define CTEST_EXPECT_LT(a, b) EXPECT__WRAP(CTEST__CMP(a, LT, b))
#define CTEST_ASSERT_LT(a, b) ASSERT__WRAP(CTEST__CMP(a, LT, b))
#define CTEST_EXPECT_LE(a, b) EXPECT__WRAP(CTEST__CMP(a, LE, b))
#define CTEST_ASSERT_LE(a, b) ASSERT__WRAP(CTEST__CMP(a, LE, b))
#define CTEST_EXPECT_GT(a, b) EXPECT__WRAP(CTEST__CMP(a, GT, b))
#define CTEST_ASSERT_GT(a, b) ASSERT__WRAP(CTEST__CMP(a, GT, b))
#define CTEST_EXPECT_GE(a, b) EXPECT__WRAP(CTEST__CMP(a, GE, b))
#define CTEST_ASSERT_GE(a, b) ASSERT__WRAP(CTEST__CMP(a, GE, b))

#define CTEST__STR_CMP(a, cmp, b) \
    ctest__cmp_str(__FILE__, __LINE__, a, #a, CTEST__CMP_ ## cmp, b, #b)

#define CTEST_EXPECT_STR_EQ(a, b) EXPECT__WRAP(CTEST__STR_CMP(a, EQ, b))
#define CTEST_ASSERT_STR_EQ(a, b) ASSERT__WRAP(CTEST__STR_CMP(a, EQ, b))
#define CTEST_EXPECT_STR_NE(a, b) EXPECT__WRAP(CTEST__STR_CMP(a, NE, b))
#define CTEST_ASSERT_STR_NE(a, b) ASSERT__WRAP(CTEST__STR_CMP(a, NE, b))
#define CTEST_EXPECT_STR_LT(a, b) EXPECT__WRAP(CTEST__STR_CMP(a, LT, b))
#define CTEST_ASSERT_STR_LT(a, b) ASSERT__WRAP(CTEST__STR_CMP(a, LT, b))
#define CTEST_EXPECT_STR_LE(a, b) EXPECT__WRAP(CTEST__STR_CMP(a, LE, b))
#define CTEST_ASSERT_STR_LE(a, b) ASSERT__WRAP(CTEST__STR_CMP(a, LE, b))
#define CTEST_EXPECT_STR_GT(a, b) EXPECT__WRAP(CTEST__STR_CMP(a, GT, b))
#define CTEST_ASSERT_STR_GT(a, b) ASSERT__WRAP(CTEST__STR_CMP(a, GT, b))
#define CTEST_EXPECT_STR_GE(a, b) EXPECT__WRAP(CTEST__STR_CMP(a, GE, b))
#define CTEST_ASSERT_STR_GE(a, b) ASSERT__WRAP(CTEST__STR_CMP(a, GE, b))

#define CTEST__NEAR(a, b, absdiff) \
    ctest__check_near(__FILE__, __LINE__, a, #a, b, #b, absdiff)
#define CTEST_EXPECT_NEAR(a, b, absdiff) \
    EXPECT__WRAP(CTEST__NEAR(a, b, absdiff))
#define CTEST_ASSERT_NEAR(a, b, absdiff) \
    ASSERT__WRAP(CTEST__NEAR(a, b, absdiff))

#ifndef CTEST_NO_SHORT_NAMES
#  define ASSERT_TRUE     CTEST_ASSERT_TRUE
#  define EXPECT_TRUE     CTEST_EXPECT_TRUE
#  define ASSERT_FALSE    CTEST_ASSERT_FALSE
#  define EXPECT_FALSE    CTEST_EXPECT_FALSE
#  define ASSERT_EQ       CTEST_ASSERT_EQ
#  define EXPECT_EQ       CTEST_EXPECT_EQ
#  define ASSERT_NE       CTEST_ASSERT_NE
#  define EXPECT_NE       CTEST_EXPECT_NE
#  define ASSERT_LT       CTEST_ASSERT_LT
#  define EXPECT_LT       CTEST_EXPECT_LT
#  define ASSERT_LE       CTEST_ASSERT_LE
#  define EXPECT_LE       CTEST_EXPECT_LE
#  define ASSERT_GT       CTEST_ASSERT_GT
#  define EXPECT_GT       CTEST_EXPECT_GT
#  define ASSERT_GE       CTEST_ASSERT_GE
#  define EXPECT_GE       CTEST_EXPECT_GE
#  define ASSERT_STR_EQ   CTEST_ASSERT_STR_EQ
#  define EXPECT_STR_EQ   CTEST_EXPECT_STR_EQ
#  define ASSERT_STR_NE   CTEST_ASSERT_STR_NE
#  define EXPECT_STR_NE   CTEST_EXPECT_STR_NE
#  define ASSERT_STR_LT   CTEST_ASSERT_STR_LT
#  define EXPECT_STR_LT   CTEST_EXPECT_STR_LT
#  define ASSERT_STR_LE   CTEST_ASSERT_STR_LE
#  define EXPECT_STR_LE   CTEST_EXPECT_STR_LE
#  define ASSERT_STR_GT   CTEST_ASSERT_STR_GT
#  define EXPECT_STR_GT   CTEST_EXPECT_STR_GT
#  define ASSERT_STR_GE   CTEST_ASSERT_STR_GE
#  define EXPECT_STR_GE   CTEST_EXPECT_STR_GE
#  define ASSERT_STR_EQ   CTEST_ASSERT_STR_EQ
#  define EXPECT_STR_EQ   CTEST_EXPECT_STR_EQ
#  define ASSERT_NEAR     CTEST_ASSERT_NEAR
#  define EXPECT_NEAR     CTEST_EXPECT_NEAR
#  define FAIL            CTEST_FAIL
#  define SKIP            CTEST_SKIP
#  define TEST            CTEST_TEST
#  define TEST_F          CTEST_TEST_F
#  define TEST_F_INIT     CTEST_TEST_F_INIT
#  define TEST_F_DROP     CTEST_TEST_F_DROP
#  define LOG             CTEST_LOG
#endif

#endif // CTEST_H


#ifdef CTEST_IMPLEMENTATION

#include <assert.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define CTEST__CMP_XMACRO(X) \
    X(EQ, ==) \
    X(NE, !=) \
    X(LT,  <) \
    X(LE, <=) \
    X(GT,  >) \
    X(GE, >=) \

static inline const char *ctest__cmp_to_str(enum ctest__cmp cmp) {
    #define X(OP,STR) if (cmp == CTEST__CMP_ ## OP) return #STR;
    CTEST__CMP_XMACRO(X)
    #undef X
    assert(!"Invalid ctest__cmp");
}

int ctest__check_bool(
    const char *fpath, int lineno,
    _Bool a, const char * a_str,
    _Bool b
) {
    if (a == b) return 1;
    fprintf(stdout, "%s:%d: Failure\n", fpath, lineno);
    fprintf(stdout, "Expected: (%s) to be %s\n",
        a_str, b ? "true" : "false");
    ctest_fail_test();
    return 0;
}

int ctest__check_near(
    const char *fpath, int lineno,
    double a, const char * a_str,
    double b, const char * b_str,
    double absdiff
) {
    double diff = a > b ? a - b : b - a;
    if (diff <= absdiff) return 1;
    fprintf(stdout, "%s:%d: Failure\n", fpath, lineno);
    fprintf(stdout, "The difference between %s and %s is %g"
                    ", which exceeds %g\n", a_str, b_str, diff, absdiff);
    fprintf(stdout, "  %s evaluates to %.15lf.\n", a_str, a);
    fprintf(stdout, "  %s evaluates to %.15lf.\n", b_str, b);
    ctest_fail_test();
    return 0;
}

#define CTEST__CMP_FUNC_IMPL(FNAME, TYPE, FMT, X) \
int FNAME(                                        \
    const char *fpath, int lineno,                \
    TYPE a, const char * a_str,                   \
    enum ctest__cmp cmp,                          \
    TYPE b, const char * b_str                    \
) {                                               \
    CTEST__CMP_XMACRO(X)                          \
    fprintf(stdout, "%s:%d: Failure\n", fpath, lineno); \
    fprintf(stdout, "Expected: %s %s %s, got\n",  \
        a_str, ctest__cmp_to_str(cmp), b_str);    \
    fprintf(stdout, "  lhs = " FMT "\n", a);      \
    fprintf(stdout, "  rhs = " FMT "\n", b);      \
    fprintf(stdout, "\n");                        \
    ctest_fail_test();                            \
    return 0;                                     \
}

#define X(name, op) if (cmp == CTEST__CMP_ ## name && a op b) return 1;
CTEST__CMP_FUNC_IMPL(ctest__cmp_signed,     long long signed, "%lld", X)
CTEST__CMP_FUNC_IMPL(ctest__cmp_unsigned, long long unsigned, "%llu", X)
CTEST__CMP_FUNC_IMPL(ctest__cmp_double,               double,   "%g", X)
CTEST__CMP_FUNC_IMPL(ctest__cmp_ptr,   const volatile void *,   "%p", X)
#undef X
#define X(name, op) if (cmp == CTEST__CMP_ ## name && strcmp(a,b) op 0) return 1;
CTEST__CMP_FUNC_IMPL(ctest__cmp_str, const char *, "\"%s\"", X)
#undef X


static volatile enum ctest_status ctest_status;
static jmp_buf ctest_longjmp_env;

void ctest__cleanup(const int * armed) {
    if (*armed) longjmp(ctest_longjmp_env, 1);
}

void ctest_fail_test(void) {
    ctest_status = CTEST_FAILURE;
}

void ctest_drop_test(const char *fpath, int line) {
    fprintf(stdout, "%s:%d: Failure\n", fpath, line);
    ctest_status = CTEST_FAILURE;
    longjmp(ctest_longjmp_env, 1);
}

void ctest_skip_test(void) {
    if (ctest_status != CTEST_FAILURE)
        ctest_status = CTEST_SKIPPED;
    longjmp(ctest_longjmp_env, 1);
}

int ctest_failed(void) {
    return ctest_status == CTEST_FAILURE;
}

void ctest_log(const char * fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    (void)vfprintf(stdout, fmt, ap);
    va_end(ap);
}

#define CTEST_COLOR_RED     "\033[0;31m"
#define CTEST_COLOR_GREEN   "\033[0;32m"
#define CTEST_COLOR_YELLOW  "\033[0;33m"
#define CTEST_COLOR_DEFAULT "\033[0m"

static const char *ctest_status_color_string[] = {
    [CTEST_RUNNING] = CTEST_COLOR_GREEN  "[ RUNNING    ]" CTEST_COLOR_DEFAULT,
    [CTEST_SKIPPED] = CTEST_COLOR_YELLOW "[    SKIPPED ]" CTEST_COLOR_DEFAULT,
    [CTEST_SUCCESS] = CTEST_COLOR_GREEN  "[    SUCCESS ]" CTEST_COLOR_DEFAULT,
    [CTEST_FAILURE] = CTEST_COLOR_RED    "[    FAILURE ]" CTEST_COLOR_DEFAULT,
};

static const char *ctest_status_mono_string[] = {
    [CTEST_RUNNING] = "[ RUNNING    ]",
    [CTEST_SKIPPED] = "[    SKIPPED ]",
    [CTEST_SUCCESS] = "[    SUCCESS ]",
    [CTEST_FAILURE] = "[    FAILURE ]",
};

static const char **ctest_status_string = 0;

static ctest * ctest_head;
static ctest ** ctest_tail_p = &ctest_head;

void ctest_register(ctest * test) {
    assert(ctest_tail_p && "cannot ctest_register after running tests");
    test->_next = 0;
    *ctest_tail_p = test;
    ctest_tail_p = &test->_next;
}

#define CTEST_FOR_EACH(n) \
    for (ctest * n = ctest_head; n; n = n->_next)

static void ctest_run(ctest * t) {
    ctest_status = CTEST_RUNNING;
    fprintf(stdout, "%s: %s\n", ctest_status_string[ctest_status], t->name);

    if (t->_init)
        if (setjmp(ctest_longjmp_env) == 0)
            t->_init();

    if (ctest_status == CTEST_RUNNING) {
        if (setjmp(ctest_longjmp_env) == 0)
            t->_exec();
        if (ctest_status == CTEST_RUNNING)
            ctest_status = CTEST_SUCCESS;
        if (t->_drop)
            t->_drop();
    }

    t->_status = ctest_status;
    fprintf(stdout, "%s: %s\n", ctest_status_string[ctest_status], t->name);
}

static void ctest_list_results(enum ctest_status status, _Bool list) {
    size_t count = 0;
    CTEST_FOR_EACH(node)
        if (node->_status == status)
            ++count;

    if (count == 0)
        return;

    const char * status_str = ctest_status_string[status];
    fprintf(stdout, "%s %zu tests.\n", status_str, count);

    if (list) {
        CTEST_FOR_EACH(node)
            if (node->_status == status)
                fprintf(stdout, "%s %s\n", status_str, node->name);
    }
}

static int ctest_is_disabled(struct ctest * t) {
    return strstr(t->name, ".DISABLED_") != 0;
}

struct ctest_config {
    int list_tests;
    int repeat;
    int also_run_disabled_tests;
    int show_help;
    int shuffle;
    int random_seed;
    int is_correct;
    int color;
    char * filter;
};

static int ctest_parse_int(const char * str, int * dst) {
    return sscanf(str, "%d%c", dst, (char[1]){0}) != 1;
}

static struct ctest_config ctest_get_config(int * argc_p, char ** argv) {
    struct ctest_config cfg = { .random_seed = (int)time(0) };
    int argc = *argc_p;

    int non_ctest_opts = 1;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            cfg.show_help = 1;
        } else if (strcmp(argv[i], "--ctest_list_tests") == 0) {
            cfg.list_tests = 1;
        } else if (strcmp(argv[i], "--ctest_also_run_disabled_tests") == 0) {
            cfg.also_run_disabled_tests = 1;
        } else if (strcmp(argv[i], "--ctest_repeat") == 0) {
            if (ctest_parse_int(argv[++i], &cfg.repeat) != 0)
                return cfg;
        } else if (strncmp(argv[i], "--ctest_repeat=", 15) == 0) {
            if (ctest_parse_int(argv[i] + 15, &cfg.repeat) != 0)
                return cfg;
        } else if (strcmp(argv[i], "--ctest_shuffle") == 0) {
            cfg.shuffle = 1;
        } else if (strcmp(argv[i], "--ctest_filter") == 0) {
            cfg.filter = argv[++i];
        } else if (strncmp(argv[i], "--ctest_filter=", 15) == 0) {
            cfg.filter = argv[i] + 15;
        } else if (strcmp(argv[i], "--ctest_random_seed") == 0) {
            if (ctest_parse_int(argv[++i], &cfg.random_seed) != 0)
                return cfg;
        } else if (strncmp(argv[i], "--ctest_random_seed=", 20) == 0) {
            if (ctest_parse_int(argv[i] + 20, &cfg.random_seed) != 0)
                return cfg;
        } else if (strcmp(argv[i], "--ctest_color") == 0) {
            if (ctest_parse_int(argv[++i], &cfg.color) != 0)
                return cfg;
        } else if (strncmp(argv[i], "--ctest_color=", 14) == 0) {
            if (ctest_parse_int(argv[i] + 14, &cfg.color) != 0)
                return cfg;
        } else if (strncmp(argv[i], "--ctest_", 8) == 0) {
            // unknown option
            return cfg;
        } else {
            argv[non_ctest_opts++] = argv[i];
        }
    }

    argv[non_ctest_opts] = 0;
    *argc_p = non_ctest_opts;

    if (cfg.color == 0)
        cfg.color = isatty(STDOUT_FILENO) ? 1 : -1;
    cfg.is_correct = 1;

    return cfg;
}

static void ctest_show_help(void) {
    fprintf(stderr,
        "This program contains tests created using CTest framework. "
        "The behavior can be controlled with following options:"
        "\n\n"
        "--ctest_also_run_disabled_tests\n\tRun disabled tests.\n"
        "--ctest_color=INTEGER\n\t< 0 - no, 0 - auto, > 0 - yes.\n"
        "--ctest_filter=PATTERN\n\tUse filter to select tests.\n"
        "--ctest_list_tests\n\tLists all tests.\n"
        "--ctest_repeat=INTEGER\n\tRepeat tests given times.\n"
        "--ctest_shuffle\n\tShuffle tests at each iteration.\n"
        "--ctest_random_seed\n\tRandom seed for shuffling.\n"
    );
}

static int ctest_match(const char * str, const char * rex) {
    int rlen = strlen(rex);
    int slen = strlen(str);

    _Bool data[4096], *match = data + 1;
    if (rlen + 1 > (int)sizeof data)
        return 0; // too long

    memset(data, 0, rlen + 1);

    for (int spos = -1; spos < slen; ++spos) {
        _Bool prev = (spos == -1);
        for (int rpos =  0; rpos < rlen; ++rpos) {
            int s = (spos < 0 ? 0 : str[spos]);
            int r = rex[rpos];

            _Bool curr;

            if      (r == ':')
                curr = (s == 0);
            else if (r == '*')
                curr = prev || match[rpos];
            else if (r == '?')
                curr = match[rpos - 1];
            else if (r == '\"' || r == '"')
                curr = prev;
            else
                curr = match[rpos - 1] && (r == s);

            match[rpos - 1] = prev;
            prev = curr;
        }
        match[rlen - 1] = prev;
    }

    for (int rpos =  0; rpos <= rlen; ++rpos)
        if ((rex[rpos] == ':' || rex[rpos] == 0) && match[rpos - 1])
            return 1;

    return 0;
}

static void ctest_select_tests(struct ctest_config cfg) {
    ctest ** prev = &ctest_head;
    CTEST_FOR_EACH(node)
        if (!cfg.filter || ctest_match(node->name, cfg.filter)) {
            *prev = node;
            prev = &node->_next;
        }
    *prev = 0;
}

static ctest * ctest_shuffle_run_list(ctest * head) {
    // empty or singular list need no shuffling
    if (!head || !head->_next)
        return head;

    // split odd ane even nodes to separate lists
    ctest * list[2] = {0, 0};
    int list_cnt[2] = {0, 0};
    int id = 0;
    for (ctest * node = head, *next; node; node = next) {
        next = node->_next;
        node->_next = list[id];
        list[id] = node;
        ++list_cnt[id];
        id = 1 - id;
    }

    // shuffle lists recursively
    list[0] = ctest_shuffle_run_list(list[0]);
    list[1] = ctest_shuffle_run_list(list[1]);

    // merge lists
    head = 0;
    while (list_cnt[0] + list_cnt[1] > 0) {
        int id = (rand() % (list_cnt[0] + list_cnt[1]) >= list_cnt[0]);
        assert(list[id]);
        struct ctest * next = list[id]->_next;
        list[id]->_next = head;
        head = list[id];
        list[id] = next;
        --list_cnt[id];
    }

    return head;
}

static size_t ctest_run_tests(struct ctest_config cfg) {
    size_t disabled_cnt = 0;
    size_t  failure_cnt = 0;

    CTEST_FOR_EACH(node)
        if (!ctest_is_disabled(node) || cfg.also_run_disabled_tests) {
            ctest_run(node);
            failure_cnt += (node->_status == CTEST_FAILURE);
        } else {
            ++disabled_cnt;
        }

    fprintf(stdout, "\n=== SUMMARY ===\n\n");
    ctest_list_results(CTEST_SUCCESS, 0);
    ctest_list_results(CTEST_SKIPPED, 1);
    ctest_list_results(CTEST_FAILURE, 1);

    if (failure_cnt == 0) {
        fprintf(stdout, "\nAll tests passed.\n");
    } else {
        fprintf(stdout, "\n%zu tests FAILED.\n", failure_cnt);
    }

    if (disabled_cnt > 0) {
        fprintf(stdout, "    %s%zu test%s DISABLED.\n%s",
                cfg.color > 0 ? CTEST_COLOR_YELLOW : "",
                disabled_cnt, disabled_cnt == 1 ? " is" : "s are",
                cfg.color > 0 ? CTEST_COLOR_DEFAULT : "");
    }

    return failure_cnt;
}

int ctest_main(int * argc_p, char *argv[]) {
    struct ctest_config cfg = ctest_get_config(argc_p, argv);
    ctest_tail_p = 0; // freeze tests

    if (!cfg.is_correct || cfg.show_help) {
        ctest_show_help();
        return EXIT_FAILURE;
    }

    ctest_status_string = cfg.color > 0 ? ctest_status_color_string
                                        : ctest_status_mono_string;

    ctest_select_tests(cfg);

    if (cfg.list_tests) {
        CTEST_FOR_EACH(t)
            fprintf(stdout, "%s\n", t->name);
        return EXIT_SUCCESS;
    }

    if (cfg.shuffle) {
        fprintf(stdout, "Random seed is %d.\n", cfg.random_seed);
        srand(cfg.random_seed);
    }

    size_t failure_cnt = 0;
    for (int rep = 0; rep <= cfg.repeat; ++rep) {
        if (cfg.shuffle) {
            ctest_head = ctest_shuffle_run_list(ctest_head);
        }
        if (rep > 0)
            fprintf(stdout, "\nRepeating test, iteration %d ...\n\n", rep);
        failure_cnt += ctest_run_tests(cfg);
    }

    return failure_cnt == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif /* CTEST_IMPLEMENTATION */
