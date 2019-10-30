/*==============================================================================
 run_tests.c -- test aggregator and results reporting

 Copyright (c) 2018-2019, Laurence Lundblade. All rights reserved.

 SPDX-License-Identifier: BSD-3-Clause

 See BSD-3-Clause license in README.md

 Created on 9/30/18
 ==============================================================================*/

#include "run_tests.h"
#include "UsefulBuf.h"
#include <stdbool.h>

#include "t_cose_test.h"
#include "t_cose_openssl_test.h"
#include "t_cose_sign_verify_test.h"


/*
 Test configuration
 */

typedef int (test_fun_t)(void);
typedef const char * (test_fun2_t)(void);


#define TEST_ENTRY(test_name)  {#test_name, test_name, true}
#define TEST_ENTRY_DISABLED(test_name)  {#test_name, test_name, false}

typedef struct {
    const char  *szTestName;
    test_fun_t  *test_fun;
    bool         bEnabled;
} test_entry;

#ifdef STRING_RETURNING_TESTS
typedef struct {
    const char *szTestName;
    test_fun2_t  *test_fun;
    bool         bEnabled;
} test_entry2;


static test_entry2 s_tests2[] = {
};
#endif

static test_entry s_tests[] = {
#ifndef T_COSE_DISABLE_SIGN_VERIFY_TESTS
    /* Many tests can be run without a crypto library integration and provide
     * good test coverage of everything but the signing and verification. These
     * tests can't be run with signing and verification short circuited */
    TEST_ENTRY(sign_verify_basic_test),
    TEST_ENTRY(sign_verify_make_cwt_test),
    TEST_ENTRY(sign_verify_sig_fail_test),
#endif
    TEST_ENTRY(sign1_structure_decode_test),
    TEST_ENTRY(content_type_test),
    TEST_ENTRY(all_headers_test),
    TEST_ENTRY(cose_example_test),
    TEST_ENTRY(critical_headers_test),
    TEST_ENTRY(bad_headers_test),
    TEST_ENTRY(short_circuit_no_parse_test),
    TEST_ENTRY(short_circuit_make_cwt_test),
    TEST_ENTRY(short_circuit_signing_error_conditions_test),
    TEST_ENTRY(short_circuit_verify_fail_test),
    TEST_ENTRY(short_circuit_self_test),

#ifdef T_COSE_ENABLE_HASH_FAIL_TEST
    TEST_ENTRY(short_circuit_hash_fail_test),
#endif /* T_COSE_DISABLE_HASH_FAIL_TEST */
};



/*
 Convert a number up to 999999999 to a string. This is so sprintf doesn't
 have to be linked in so as to minimized dependencies even in test code.

 StringMem should be 12 bytes long, 9 for digits, 1 for minus and
 1 for \0 termination.
 */
static const char *NumToString(int32_t nNum, UsefulBuf StringMem)
{
   const int32_t nMax = 1000000000;

   UsefulOutBuf OutBuf;
   UsefulOutBuf_Init(&OutBuf, StringMem);

   if(nNum < 0) {
      UsefulOutBuf_AppendByte(&OutBuf, '-');
      nNum = -nNum;
   }
   if(nNum > nMax-1) {
      return "XXX";
   }

   bool bDidSomeOutput = false;
   for(int n = nMax; n > 0; n/=10) {
      int x = nNum/n;
      if(x || bDidSomeOutput){
         bDidSomeOutput = true;
         UsefulOutBuf_AppendByte(&OutBuf, '0' + x);
         nNum -= x * n;
      }
   }
   if(!bDidSomeOutput){
      UsefulOutBuf_AppendByte(&OutBuf, '0');
   }
   UsefulOutBuf_AppendByte(&OutBuf, '\0');

   return UsefulOutBuf_GetError(&OutBuf) ? "" : StringMem.ptr;
}


/*
 Public function. See run_test.h.
 */
int RunTests(const char *szTestNames[], OutputStringCB pfOutput, void *poutCtx, int *pNumTestsRun)
{
    int nTestsFailed = 0;
    int nTestsRun = 0;
    UsefulBuf_MAKE_STACK_UB(StringStorage, 12);

#ifdef STRING_RETURNING_TESTS

    test_entry2 *t2;
    const test_entry2 *s_tests2_end = s_tests2 + sizeof(s_tests2)/sizeof(test_entry2);

    for(t2 = s_tests2; t2 < s_tests2_end; t2++) {
        if(szTestNames[0]) {
            // Some tests have been named
            const char **szRequestedNames;
            for(szRequestedNames = szTestNames; *szRequestedNames;  szRequestedNames++) {
                if(!strcmp(t2->szTestName, *szRequestedNames)) {
                    break; // Name matched
                }
            }
            if(*szRequestedNames == NULL) {
                // Didn't match this test
                continue;
            }
        } else {
            // no tests named, but don't run "disabled" tests
            if(!t2->bEnabled) {
                // Don't run disabled tests when all tests are being run
                // as indicated by no specific test names being given
                continue;
            }
        }
        const char * szTestResult = (t2->test_fun)();
        nTestsRun++;
        if(pfOutput) {
            (*pfOutput)(t2->szTestName, poutCtx, 0);
        }

        if(szTestResult) {
            if(pfOutput) {
                (*pfOutput)(" FAILED (returned ", poutCtx, 0);
                (*pfOutput)(szTestResult, poutCtx, 0);
                (*pfOutput)(")", poutCtx, 1);
            }
            nTestsFailed++;
        } else {
            if(pfOutput) {
                (*pfOutput)( " PASSED", poutCtx, 1);
            }
        }
    }
#endif


    test_entry *t;
    const test_entry *s_tests_end = s_tests + sizeof(s_tests)/sizeof(test_entry);

    for(t = s_tests; t < s_tests_end; t++) {
        if(szTestNames[0]) {
            // Some tests have been named
            const char **szRequestedNames;
            for(szRequestedNames = szTestNames; *szRequestedNames;  szRequestedNames++) {
                if(!strcmp(t->szTestName, *szRequestedNames)) {
                    break; // Name matched
                }
            }
            if(*szRequestedNames == NULL) {
                // Didn't match this test
                continue;
            }
        } else {
            // no tests named, but don't run "disabled" tests
            if(!t->bEnabled) {
                // Don't run disabled tests when all tests are being run
                // as indicated by no specific test names being given
                continue;
            }
        }

        int nTestResult = (t->test_fun)();
        nTestsRun++;
        if(pfOutput) {
            (*pfOutput)(t->szTestName, poutCtx, 0);
        }

        if(nTestResult) {
            if(pfOutput) {
                (*pfOutput)(" FAILED (returned ", poutCtx, 0);
                (*pfOutput)(NumToString(nTestResult, StringStorage), poutCtx, 0);
                (*pfOutput)(")", poutCtx, 1);
            }
            nTestsFailed++;
        } else {
            if(pfOutput) {
                (*pfOutput)( " PASSED", poutCtx, 1);
            }
        }
    }

    if(pNumTestsRun) {
        *pNumTestsRun = nTestsRun;
    }

    if(pfOutput) {
        (*pfOutput)( "SUMMARY: ", poutCtx, 0);
        (*pfOutput)( NumToString(nTestsRun, StringStorage), poutCtx, 0);
        (*pfOutput)( " tests run; ", poutCtx, 0);
        (*pfOutput)( NumToString(nTestsFailed, StringStorage), poutCtx, 0);
        (*pfOutput)( " tests failed", poutCtx, 1);
    }

    return nTestsFailed;
}


/*
 Public function. See run_test.h.
 */
static void PrintSize(const char *szWhat, uint32_t uSize, OutputStringCB pfOutput, void *pOutCtx)
{
   UsefulBuf_MAKE_STACK_UB(buffer, 20);

   (*pfOutput)(szWhat, pOutCtx, 0);
   (*pfOutput)(" ", pOutCtx, 0);
   (*pfOutput)(NumToString(uSize, buffer), pOutCtx, 0);
   (*pfOutput)("", pOutCtx, 1);
}


/*
 Public function. See run_test.h.
 */

#include "t_cose_sign1_sign.h" /* For size printing */
#include "t_cose_crypto.h"

void PrintSizes(OutputStringCB pfOutput, void *pOutCtx)
{
   // Type and size of return from sizeof() varies. These will never be large so cast is safe
    PrintSize("sizeof(struct t_cose_sign1_ctx)",
              (uint32_t)sizeof(struct t_cose_sign1_sign_ctx),
              pfOutput, pOutCtx);
    PrintSize("sizeof(struct t_cose_signing_key)",
              (uint32_t)sizeof(struct t_cose_key),
              pfOutput, pOutCtx);
    PrintSize("sizeof(struct t_cose_crypto_hash)",
              (uint32_t)sizeof(struct t_cose_crypto_hash),
              pfOutput, pOutCtx);
   (*pfOutput)("", pOutCtx, 1);
}
