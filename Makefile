# Makefile -- UNIX-style make for t_cose
#
# Copyright (c) 2019, Laurence Lundblade. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# See BSD-3-Clause license in README.md
#

# ---- comment ----
# This t_cose makefile is for test crypto
# It has no dependency on any external crypto library, but doesn't
# support real ECDSA signing. The only external code needed is QCBOR.


# ---- QCBOR location ----
# Adjust this to the location of QCBOR in your build environment
QCBOR_INC= -I ../../QCBOR/master/inc
QCBOR_LIB=../../QCBOR/master/libqcbor.a


# ---- crypto configuration -----
# Uses only the internal Brad Conte hash implementation that is bundled with t_cose
CRYPTO_INC=-I crypto_adapters/b_con_hash
CRYPTO_OBJ=crypto_adapters/t_cose_test_crypto.o crypto_adapters/b_con_hash/sha256.o
CRYPTO_LIB=


# ---- compiler configuration -----
C_OPTS=-Os -Wall -pedantic-errors -Wextra -Wshadow -Wparentheses -xc -std=c99


# ---- T_COSE Config and test options ----
CONFIG_OPTS=-DT_COSE_ENABLE_HASH_FAIL_TEST -DT_COSE_USE_B_CON_SHA256 -DT_COSE_DISABLE_SIGN_VERIFY_TESTS
CRYPTO_TEST_OBJ=
TEST_OBJ=test/t_cose_test.o test/run_tests.o test/t_cose_make_test_messages.o $(CRYPTO_TEST_OBJ)


# ---- the main body that is invariant ----
INC=-I inc -I Test  -I src
ALL_INC=$(CRYPTO_INC) $(QCBOR_INC) $(INC) 
CFLAGS=$(ALL_INC) $(C_OPTS) $(CONFIG_OPTS)

SRC_OBJ=src/t_cose_sign1_verify.o src/t_cose_sign1_sign.o src/t_cose_util.o src/t_cose_headers.o

all:	t_cose_test libt_cose.a


libt_cose.a: $(SRC_OBJ) $(CRYPTO_OBJ)
	ar -r $@ $^


t_cose_test: main.o  libt_cose.a $(TEST_OBJ)
	cc -o $@ $^ $(QCBOR_LIB) $(CRYPTO_LIB) libt_cose.a


clean:
	rm -f $(SRC_OBJ) $(TEST_OBJ) $(CRYPTO_OBJ) libt_cose.a t_cose_test


# ---- source dependecies -----
src/t_cose_util.o:	src/t_cose_util.h src/t_cose_standard_constants.h inc/t_cose_common.h src/t_cose_crypto.h
src/t_cose_sign1_verify.o:	inc/t_cose_sign1_verify.h src/t_cose_crypto.h src/t_cose_util.h src/t_cose_headers.h inc/t_cose_common.h src/t_cose_standard_constants.h
src/t_cose_headers.o: src/t_cose_headers.h src/t_cose_standard_constants.h inc/t_cose_sign1_verify.h inc/t_cose_common.h
src/t_cose_sign1_sign.o: inc/t_cose_sign1_sign.h src/t_cose_standard_constants.h src/t_cose_crypto.h src/t_cose_util.h inc/t_cose_common.h 


# ---- test dependencies -----
test/t_cose_test.o: test/t_cose_test.h inc/t_cose_sign1_sign.h inc/t_cose_sign1_verify.h inc/t_cose_common.h test/t_cose_make_test_messages.h src/t_cose_crypto.h
test/t_cose_make_test_messages.o: test/t_cose_make_test_messages.h inc/t_cose_sign1_sign.h inc/t_cose_common.h src/t_cose_standard_constants.h src/t_cose_crypto.h src/t_cose_util.h
test/run_test.o: test/run_test.h test/t_cose_test.h test/t_cose_hash_fail_test.h


# ---- crypto dependencies ----
crypto_adapters/t_cose_test_crypto.o:	src/t_cose_crypto.h inc/t_cose_common.h src/t_cose_standard_constants.h inc/q_useful_buf.h crypto_adapters/b_con_hash/sha256.h
crypto_adapters/b_con_hash/sha256.o:	crypto_adapters/b_con_hash/sha256.h
