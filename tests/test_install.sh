#!/bin/bash
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.
set -e

# Setup env
INSTALL_PREFIX=$(cat /tmp/install_prefix)
mkdir -p nested/run
cd nested/run

python3.7 -m venv env
source env/bin/activate
python -m pip install -U -r "$INSTALL_PREFIX"/bin/requirements.txt
pip freeze > "$INSTALL_PREFIX"/bin/requirements.txt

# Start ephemeral network in the background
timeout --signal=SIGINT --kill-after=30s --preserve-status 30s \
python "$INSTALL_PREFIX"/bin/start_network.py \
    -p ../../build/liblogging \
    -b "$INSTALL_PREFIX"/bin \
    -g "$(pwd)"/../../src/runtime_config/gov.lua \
    -v &

# Issue tutorial transactions to ephemeral network
sleep 20
python ../../python/tutorial.py client_info.txt
sleep 15

# Recover network
cp -r ./workspace/start_network_0/0.ledger .
cp ./workspace/start_network_0/network_enc_pubk.pem .

timeout --signal=SIGINT --kill-after=30s --preserve-status 30s \
python "$INSTALL_PREFIX"/bin/start_network.py \
    -p ../../build/liblogging \
    -b "$INSTALL_PREFIX"/bin \
    -v \
    --recover \
    --ledger-dir 0.ledger \
    --network-enc-pubk network_enc_pubk.pem \
    --common-dir ./workspace/start_network_common/