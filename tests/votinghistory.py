# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.
import sys
import os
import io

import e2e_args
import getpass
import time
import logging
import multiprocessing
from random import seed
import infra.ccf
import infra.proc
import infra.jsonrpc
import infra.remote
import json
import ledger
import msgpack

import cryptography.x509
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.backends import default_backend


def verify_sig(raw_cert, sig, req):
    cert = cryptography.x509.load_der_x509_certificate(
        raw_cert, backend=default_backend()
    )
    pub_key = cert.public_key()
    hash_alg = ec.ECDSA(cert.signature_hash_algorithm)
    pub_key.verify(sig, req, hash_alg)


def run(args):
    hosts = ["localhost", "localhost"]

    ledger_filename = None

    with infra.ccf.network(
        hosts, args.build_dir, args.debug_nodes, args.perf_nodes, pdb=args.pdb
    ) as network:
        primary, others = network.start_and_join(args)

        # create a lua query file to change a member state to accepted
        with open("query.lua", "w") as qfile:
            qfile.write(
                """local tables, param = ...
               local member_id = param
               local STATE_ACCEPTED = 0
               local member_info = {status = STATE_ACCEPTED}
               local p = Puts:new()
               p:put("members", member_id, member_info)
               return Calls:call("raw_puts", p)"""
            )

        # create json file to be passed as the argument to the query.lua file
        # it is passing a member id
        with open("param.json", "w") as pfile:
            pfile.write("""{"p": 0}""")

        # propose to add a new member
        # proposal number 0
        infra.proc.ccall("./genesisgenerator", "cert", "--name=member4")
        result = infra.proc.ccall(
            "./memberclient",
            "add_member",
            "--cert=member1_cert.pem",
            "--privk=member1_privk.pem",
            "--host={}".format(primary.host),
            "--port={}".format(primary.tls_port),
            "--member_cert=member4_cert.pem",
            "--ca=networkcert.pem",
        )

        # when proposal is added the proposal id and the result of running complete proposal are returned
        j_result = json.loads(result.stdout)
        assert not j_result["result"]["completed"]
        assert j_result["result"]["id"] == 0

        # 2 out of 3 members vote to accept the new member so that that member can send its own proposals
        result = infra.proc.ccall(
            "./memberclient",
            "vote",
            "--accept",
            "--cert=member1_cert.pem",
            "--privk=member1_privk.pem",
            "--host={}".format(primary.host),
            "--port={}".format(primary.tls_port),
            "--id=0",
            "--ca=networkcert.pem",
            "--sign",
        )
        j_result = json.loads(result.stdout)
        assert not j_result["result"]

        result = infra.proc.ccall(
            "./memberclient",
            "vote",
            "--accept",
            "--cert=member2_cert.pem",
            "--privk=member2_privk.pem",
            "--host={}".format(primary.host),
            "--port={}".format(primary.tls_port),
            "--id=0",
            "--ca=networkcert.pem",
            "--sign",
        )
        j_result = json.loads(result.stdout)
        assert j_result["result"]

        # member 4 ack
        result = infra.proc.ccall(
            "./memberclient",
            "ack",
            "--cert=member4_cert.pem",
            "--privk=member4_privk.pem",
            "--host={}".format(primary.host),
            "--port={}".format(primary.tls_port),
            "--ca=networkcert.pem",
        )
        j_result = json.loads(result.stdout)
        assert j_result["result"]

        ledger_filename = network.find_leader()[0].remote.get_ledger_full_path()

    l = ledger.Ledger(ledger_filename)

    # this maps a member_id to a cert object, and is updated when we iterate the transactions,
    # so that we always have the correct cert for a member on a given transaction
    members = {}
    verified_votes = 0
    for tr in l:
        tables = tr.get_public_domain().get_tables()
        members_table = tables["membercerts"]
        for cert, member_id in members_table.items():
            members[member_id] = cert

        if "votinghistory" in tables:
            votinghistory_table = tables["votinghistory"]
            for member_id, signed_request in votinghistory_table.items():
                # if the signed vote is stored - there has to be a member at this point
                assert member_id in members
                cert = members[member_id]
                sig = signed_request[0][0]
                req = signed_request[0][1]
                verify_sig(cert, sig, req)
                ++verified_votes

    assert verified_votes == 0


if __name__ == "__main__":

    def add(parser):
        parser.add_argument(
            "-p",
            "--package",
            help="The enclave package to load (e.g., libsimplebank)",
            required=True,
        )

    args = e2e_args.cli_args(add)
    run(args)
