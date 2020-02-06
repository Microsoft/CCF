# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.
import os
import getpass
import json
import time
import logging
import multiprocessing
import shutil
import random
import infra.network
import infra.proc
import infra.jsonrpc
import infra.e2e_args

from loguru import logger as LOG


def run(args):
    # SNIPPET_START: parsing
    with open(args.scenario) as f:
        scenario = json.load(f)

    hosts = scenario.get("hosts", ["localhost", "localhost"])
    args.package = scenario["package"]
    # SNIPPET_END: parsing

    scenario_dir = os.path.dirname(args.scenario)

    # SNIPPET_START: create_network
    with infra.network.network(
        hosts, args.binary_dir, args.debug_nodes, args.perf_nodes
    ) as network:
        network.start_and_join(args)
        # SNIPPET_END: create_network

        primary, backups = network.find_nodes()

        with primary.node_client() as mc:

            check = infra.checker.Checker()
            check_commit = infra.checker.Checker(mc)
            with primary.user_client(format="json") as uc:
                check_commit(uc.do("mkSign", params={}), result=True)

            for connection in scenario["connections"]:
                with (
                    primary.user_client(format="json")
                    if not connection.get("on_backup")
                    else random.choice(backups).user_client(format="json")
                ) as client:
                    txs = connection.get("transactions", [])

                    for include_file in connection.get("include", []):
                        with open(os.path.join(scenario_dir, include_file)) as f:
                            txs += json.load(f)

                    for tx in txs:
                        r = client.rpc(tx["method"], tx["params"])

                        if tx.get("expected_error") is not None:
                            check(
                                r,
                                error=lambda e: e is not None
                                and e["code"]
                                == infra.jsonrpc.ErrorCode(tx.get("expected_error")),
                            )

                        elif tx.get("expected_result") is not None:
                            check_commit(r, result=tx.get("expected_result"))

                        else:
                            check_commit(r, result=lambda res: res is not None)

                network.wait_for_node_commit_sync()

    if args.network_only:
        LOG.info("Keeping network alive with the following nodes:")
        LOG.info("  Primary = {}:{}".format(primary.pubhost, primary.rpc_port))
        for i, f in enumerate(backups):
            LOG.info("  Backup[{}] = {}:{}".format(i, f.pubhost, f.rpc_port))

        input("Press Enter to shutdown...")


if __name__ == "__main__":

    def add(parser):
        parser.add_argument(
            "--scenario",
            help="Path to JSON file listing transactions to execute",
            type=str,
            required=True,
        )

    args = infra.e2e_args.cli_args(add=add)
    run(args)
