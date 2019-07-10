# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.
import sys
import e2e_args
import infra.ccf
import infra.proc
import json

import logging
import time

from loguru import logger as LOG


def retire_node(primary, node_id):
    result = infra.proc.ccall(
        "./memberclient",
        "retire_node",
        "--cert=member1_cert.pem",
        "--privk=member1_privk.pem",
        "--host={}".format(primary.host),
        "--port={}".format(primary.tls_port),
        "--id={}".format(node_id),
        "--ca=networkcert.pem",
    )
    j_result = json.loads(result.stdout)
    assert not j_result["result"]["completed"]
    proposal_id = j_result["result"]["id"]

    # members vote to accept the proposal
    # result is true with just 1 vote because proposer implicit pro vote is assumed
    result = infra.proc.ccall(
        "./memberclient",
        "vote",
        "--accept",
        "--cert=member2_cert.pem",
        "--privk=member2_privk.pem",
        "--host={}".format(primary.host),
        "--port={}".format(primary.tls_port),
        "--id={}".format(proposal_id),
        "--ca=networkcert.pem",
        "--sign",
    )
    j_result = json.loads(result.stdout)
    assert j_result["result"]

    with primary.member_client() as c:
        id = c.request("read", {"table": "nodes", "key": node_id})
        assert c.response(id).result["status"].decode() == "RETIRED"


def run(args):
    hosts = ["localhost", "localhost"]

    with infra.ccf.network(
        hosts, args.build_dir, args.debug_nodes, args.perf_nodes, pdb=args.pdb
    ) as network:
        primary, others = network.start_and_join(args)

        # add a valid node
        res = network.create_and_add_node("libloggingenc", args)
        assert res[0] == True
        new_node = res[1]
        new_node_id = res[2]

        # attempt to add a node having the host and port fields
        # similar to a the ones of an existing node
        assert (
            network.add_node(new_node.remote.info()).error["code"]
            == infra.jsonrpc.ErrorCode.INVALID_PARAMS
        )

        # add an invalid node
        assert network.create_and_add_node("libluagenericenc", args, False) == (
            False,
            infra.jsonrpc.ErrorCode.CODE_ID_NOT_FOUND,
        )

        new_node.join_network()
        network.wait_for_node_commit_sync()

        # retire a node
        retire_node(primary, new_node_id)


if __name__ == "__main__":

    def add(parser):
        parser.add_argument(
            "-p",
            "--package",
            help="The enclave package to load (e.g., libsimplebank)",
            default="libloggingenc",
        )

    args = e2e_args.cli_args(add)

    if args.enclave_type != "debug":
        LOG.error("This test can only run in real enclaves, skipping")
        sys.exit(0)

    args.package = args.app_script and "libluagenericenc" or "libloggingenc"
    run(args)
