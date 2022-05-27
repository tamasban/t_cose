/*
*  t_cose_mini_sign_test.c
*
* Copyright 2022, Laurence Lundblade
*
* SPDX-License-Identifier: BSD-3-Clause
*
* See BSD-3-Clause license in README.md
*/

#include "t_cose_austere_test.h"
#include "t_cose_make_test_pub_key.h"

#include "t_cose/t_cose_mini_sign.h"
#include "t_cose/t_cose_sign1_verify.h"

const uint8_t payload[] = {
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03,
    0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x02, 0x03
};


int32_t austere_test(void) {

    enum t_cose_err_t               err;
    MakeUsefulBufOnStack(           output, sizeof(payload) + T_COSE_MINI_SIGN_SIZE_OVERHEAD_ES256);
    struct q_useful_buf_c           cose_sign1;
    struct t_cose_key               key_pair;
    struct t_cose_sign1_verify_ctx  verify_ctx;
    struct q_useful_buf_c           verified_payload;


    err = make_ecdsa_key_pair(T_COSE_ALGORITHM_ES256, &key_pair);
    if(err) {
        return 10;
    }

    err = t_cose_mini_sign(Q_USEFUL_BUF_FROM_BYTE_ARRAY_LITERAL(payload),
                           key_pair,
                           output,
                          &cose_sign1);
    if(err) {
        return 20;
    }


    t_cose_sign1_verify_init(&verify_ctx, 0);

    t_cose_sign1_set_verification_key(&verify_ctx, key_pair);

    err = t_cose_sign1_verify(&verify_ctx, cose_sign1, &verified_payload, NULL);
    if(err) {
        return 30;
    }

    return 0;
}


// TODO: test for output buffer too small
