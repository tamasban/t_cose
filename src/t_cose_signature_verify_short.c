/*
 * t_cose_signature_verify_short.c
 *
 * Copyright (c) 2022, Laurence Lundblade. All rights reserved.
 * Created by Laurence Lundblade on 7/27/22.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * See BSD-3-Clause license in README.md
 */

#include "t_cose/t_cose_signature_verify_short.h"
#include "t_cose/t_cose_parameters.h"
#include "t_cose_util.h"
#include "qcbor/qcbor_decode.h"
#include "qcbor/qcbor_spiffy_decode.h"
#include "t_cose_crypto.h"
#include "t_cose/t_cose_signature_sign_short.h"

//#define T_COSE_CRYPTO_MAX_HASH_SIZE 300 // TODO: fix this

#ifndef T_COSE_DISABLE_SHORT_CIRCUIT_SIGN


/* Warning: this is still early development. Documentation may be incorrect. */


static enum t_cose_err_t
t_cose_signature_verify1_short(struct t_cose_signature_verify *me_x,
                               const uint32_t                  option_flags,
                               const struct q_useful_buf_c     protected_body_headers,
                               const struct q_useful_buf_c     protected_signature_headers,
                               const struct q_useful_buf_c     payload,
                               const struct q_useful_buf_c     aad,
                               const struct t_cose_parameter  *body_parameters,
                               const struct q_useful_buf_c     signature)
{
    int32_t                     cose_algorithm_id;
    enum t_cose_err_t           return_value;
    struct q_useful_buf_c       kid;
    Q_USEFUL_BUF_MAKE_STACK_UB( buffer_for_tbs_hash, T_COSE_CRYPTO_MAX_HASH_SIZE);
    struct q_useful_buf_c       tbs_hash;
    struct t_cose_key           dummy_key;

    (void)me_x;

    dummy_key = T_COSE_NULL_KEY;


    /* --- Get the parameters values needed here --- */
    cose_algorithm_id = t_cose_find_parameter_alg_id(body_parameters);
    if(!t_cose_algorithm_is_short_circuit(cose_algorithm_id)) {
        return T_COSE_ERR_UNSUPPORTED_SIGNING_ALG;
    }

    kid = t_cose_find_parameter_kid(body_parameters);

    // TODO: unify short-circuit kid 
    if(q_useful_buf_compare(kid, get_short_circuit_kid())) {
        return_value = T_COSE_ERR_KID_UNMATCHED;
        goto Done;
    }

    if(option_flags & T_COSE_OPT_DECODE_ONLY) {
        return T_COSE_SUCCESS;
    }

    /* --- Compute the hash of the to-be-signed bytes -- */
    return_value = create_tbs_hash(cose_algorithm_id,
                                   protected_body_headers,
                                   protected_signature_headers,
                                   aad,
                                   payload,
                                   buffer_for_tbs_hash,
                                   &tbs_hash);
    if(return_value) {
        goto Done;
    }

    /* -- Verify the signature -- */
    return_value = t_cose_crypto_verify(cose_algorithm_id,
                                        dummy_key,
                                        kid,
                                        tbs_hash,
                                        signature);
Done:
    return return_value;
}



/*
 Returns: END_OF_HEADERS if no there are no more COSE_Signatures
          CBOR decoding error
          Error decoding the COSE_Signature (but not a COSE error)
          Signature validate
          Signature didn't validate
 */
static enum t_cose_err_t
t_cose_signature_verify_short(struct t_cose_signature_verify *me_x,
                              const uint32_t                  option_flags,
                              const struct t_cose_header_location      loc,
                              const struct q_useful_buf_c       protected_body_headers,
                              const struct q_useful_buf_c       payload,
                              const struct q_useful_buf_c       aad,
                              struct t_cose_parameter_storage *params,
                              QCBORDecodeContext               *qcbor_decoder,
                              struct t_cose_parameter **decoded_parameters)
{
    QCBORError                                 qcbor_error;
    enum t_cose_err_t                          return_value;
    struct q_useful_buf_c                      protected_parameters;
    struct q_useful_buf_c                      signature;
    const struct t_cose_signature_verify_short *me;

    me = (const struct t_cose_signature_verify_short *)me_x;

    /* --- Decode the COSE_Signature ---*/
    QCBORDecode_EnterArray(qcbor_decoder, NULL);

    return_value = t_cose_headers_decode(qcbor_decoder,
                                         loc,
                                         me->reader,
                                         me->reader_ctx,
                                         params,
                                         decoded_parameters,
                                         &protected_parameters);
    if(return_value != T_COSE_SUCCESS) {
        goto Done;
    }

    /* --- The signature --- */
    QCBORDecode_GetByteString(qcbor_decoder, &signature);

    QCBORDecode_ExitArray(qcbor_decoder);
    qcbor_error = QCBORDecode_GetError(qcbor_decoder);
    if(qcbor_error != QCBOR_SUCCESS) {
        return_value = qcbor_decode_error_to_t_cose_error(qcbor_error, T_COSE_ERR_SIGN1_FORMAT);
        goto Done;
    }
    /* --- Done decoding the COSE_Signature --- */


    return_value = t_cose_signature_verify1_short(me_x,
                                                  option_flags,
                                                  protected_body_headers,
                                                  protected_parameters,
                                                  payload,
                                                  aad,
                                                *decoded_parameters,
                                                  signature);
Done:
    return return_value;
}


void
t_cose_signature_verify_short_init(struct t_cose_signature_verify_short *me)
{
    memset(me, 0, sizeof(*me));
    me->s.callback  = t_cose_signature_verify_short;
    me->s.callback1 = t_cose_signature_verify1_short;
}

#else

void t_cose_signature_verify_short_placeholder(void) {}

#endif
