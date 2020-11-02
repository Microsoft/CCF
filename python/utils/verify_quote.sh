#!/bin/bash
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.

set -e

quote_file_name="quote.bin"
open_enclave_bin_path="/opt/openenclave/bin"
quote_format="LEGACY_REPORT_REMOTE"

function usage()
{
    echo "Usage:"""
    echo "  $0 https://<node-address> [--mrenclave <mrenclave_hex>] [CURL_OPTIONS]"
    echo "Verify target node's remote attestation quote."
    echo "For the verification to be successful, the public key of the node certificate should also match the SGX report data and the corresponding mrenclave should be trusted."
    echo "If no trusted mrenclave is specified (--mrenclave), the quote measurement should match one of the service's currently accepted code versions."
}

if [[ "$1" =~ ^(-h|-\?|--help)$ ]]; then
    usage
    exit 0
fi

if [ -z "$1" ]; then
    echo "Error: First argument should be CCF node address, e.g.: https://127.0.0.1:8000"
    exit 1
fi

node_address=$1
shift

while [ "$1" != "" ]; do
    case $1 in
        -h|-\?|--help)
            usage
            exit 0
            ;;
        --mrenclave)
            trusted_mrenclaves=("$2")
            ;;
        *)
            break
    esac
    shift
    shift
done

if [ -z "${trusted_mrenclaves}" ]; then
    for code_id in $(curl -sS --fail -X GET "${node_address}"/node/code "${@}" | jq .versions | jq -c ".[]"); do
        code_status=$(echo "${code_id}" | jq -r .status)
        if [ ${code_status} = "ACCEPTED" ]; then
            trusted_mrenclaves+=($(echo "${code_id}" | jq -r .digest))
        fi
    done
    echo "Retrieved ${#trusted_mrenclaves[@]} accepted code versions from service."
fi

# Temporary directory for storing retrieved quote
tmp_dir=$(mktemp -d)
function cleanup() {
  rm -rf "${tmp_dir}"
}
trap cleanup EXIT

curl -sS --fail -X GET "${node_address}"/node/quote "${@}" | jq .raw | xxd -r -p > "${tmp_dir}/${quote_file_name}"

if [ ! -s "${tmp_dir}/${quote_file_name}" ]; then
    echo "Error: Node quote is empty. Virtual mode does not support SGX quotes."
    exit 1
fi

echo "Node quote successfully retrieved. Verifying quote..."

oeverify_output=$(${open_enclave_bin_path}/oeverify -r ${tmp_dir}/${quote_file_name} -f ${quote_format})

# Extract SGX report data
oeverify_report_data=$(echo "${oeverify_output}" | grep "sgx_report_data" | cut -d ":" -f 2)
# Extract hex sha-256 (64 char) from report data (128 char)
extracted_report_data=$(echo ${oeverify_report_data#*0x} | head -c 64)

# Remove protocol and compute hash of target node's public key
stripped_node_address=${node_address#*//}
node_pubk_hash=$(echo | openssl s_client -showcerts -connect ${stripped_node_address} 2>/dev/null | openssl x509 -pubkey -noout | sha256sum | awk '{ print $1 }')

# Extract mrenclave
is_mrenclave_valid=false
oeverify_mrenclave=$(echo "${oeverify_output}" | grep "unique_id" | cut -d ":" -f 2)
extracted_mrenclave=$(echo ${oeverify_mrenclave#*0x})
for mrenclave in "${trusted_mrenclaves[@]}"; do
    if [ ${mrenclave} == ${extracted_mrenclave} ]; then
        is_mrenclave_valid=true
    fi
done

if [ ${extracted_report_data} != ${node_pubk_hash} ]; then
    echo "Error: quote verification failed."
    echo "Reported quote data does not match node certificate public key:"
    echo "${extracted_report_data} != ${node_pubk_hash}"
    exit 1
elif [ ${is_mrenclave_valid} != true ]; then
    echo "Error: quote verification failed."
    echo "Reported mrenclave ${extracted_mrenclave} is not trusted. List of trusted mrenclave:"
    echo "[${trusted_mrenclaves}]"
    exit 1
else
    echo "Quote verification successful."
    exit 0
fi