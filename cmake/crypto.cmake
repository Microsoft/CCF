# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.

set(CCFCRYPTO_SRC
    ${CCF_DIR}/src/crypto/hash.cpp
    ${CCF_DIR}/src/crypto/symmetric_key.cpp
    ${CCF_DIR}/src/crypto/key_pair.cpp
    ${CCF_DIR}/src/crypto/rsa_key_pair.cpp
    ${CCF_DIR}/src/crypto/verifier.cpp
    ${CCF_DIR}/src/crypto/key_pair_mbedtls.cpp
    ${CCF_DIR}/src/crypto/key_pair_openssl.cpp
)

if("sgx" IN_LIST COMPILE_TARGETS)
  add_library(ccfcrypto.enclave STATIC ${CCFCRYPTO_SRC})
  target_compile_definitions(
    ccfcrypto.enclave PRIVATE INSIDE_ENCLAVE _LIBCPP_HAS_THREAD_API_PTHREAD
  )
  target_compile_options(ccfcrypto.enclave PRIVATE -nostdinc++)
  target_link_libraries(
    ccfcrypto.enclave
    PRIVATE -nostdlib -nodefaultlibs -nostartfiles -Wl,--no-undefined
            -Wl,-Bstatic,-Bsymbolic,--export-dynamic,-pie
  )
  use_oe_mbedtls(ccfcrypto.enclave)
  set_property(TARGET ccfcrypto.enclave PROPERTY POSITION_INDEPENDENT_CODE ON)

  install(
    TARGETS ccfcrypto.enclave
    EXPORT ccf
    DESTINATION lib
  )
endif()

add_library(ccfcrypto.host STATIC ${CCFCRYPTO_SRC})
add_san(ccfcrypto.host)
target_compile_options(ccfcrypto.host PUBLIC -stdlib=libc++)
target_link_options(ccfcrypto.host PUBLIC -stdlib=libc++)
target_link_libraries(ccfcrypto.host PUBLIC crypto)
use_client_mbedtls(ccfcrypto.host)
set_property(TARGET ccfcrypto.host PROPERTY POSITION_INDEPENDENT_CODE ON)

install(
  TARGETS ccfcrypto.host
  EXPORT ccf
  DESTINATION lib
)
