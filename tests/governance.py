# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.
import os
import sys
import subprocess
import infra.network
import infra.path
import infra.proc
import infra.notification
import infra.net
import infra.e2e_args
import suite.test_requirements as reqs
import infra.logging_app as app

from loguru import logger as LOG


@reqs.description("Test quotes")
@reqs.supports_methods("quote", "quotes")
def test_quote(network, args, notifications_queue=None, verify=True):
    primary, _ = network.find_nodes()
    with primary.client() as c:
        oed = subprocess.run(
            [
                args.oesign,
                "dump",
                "-e",
                infra.path.build_lib_path(args.package, args.enclave_type),
            ],
            capture_output=True,
            check=True,
        )
        lines = [
            line
            for line in oed.stdout.decode().split(os.linesep)
            if line.startswith("mrenclave=")
        ]
        expected_mrenclave = lines[0].strip().split("=")[1]

        r = c.get("/node/quote")
        quotes = r.result["quotes"]
        assert len(quotes) == 1
        primary_quote = quotes[0]
        assert primary_quote["node_id"] == 0
        primary_mrenclave = primary_quote["mrenclave"]
        assert primary_mrenclave == expected_mrenclave, (
            primary_mrenclave,
            expected_mrenclave,
        )

        r = c.get("/node/quotes")
        quotes = r.result["quotes"]
        assert len(quotes) == len(network.find_nodes())
        for quote in quotes:
            mrenclave = quote["mrenclave"]
            assert mrenclave == expected_mrenclave, (mrenclave, expected_mrenclave)

    return network


@reqs.description("Add user, remove user, add user back")
@reqs.supports_methods("log/private")
def test_user(network, args, notifications_queue=None, verify=True):
    primary, _ = network.find_nodes()
    new_user_id = 3
    network.create_users([new_user_id], args.participants_curve)
    network.consortium.add_user(primary, new_user_id)
    txs = app.LoggingTxs(notifications_queue=notifications_queue, user_id=3)
    txs.issue(
        network=network, number_txs=1, consensus=args.consensus,
    )
    if verify:
        txs.verify(network)
    network.consortium.remove_user(primary, new_user_id)
    with primary.client(f"user{new_user_id}") as c:
        r = c.get("/app/log/private")
        assert r.status == 403
    return network


def run(args):
    hosts = ["localhost"] * (3 if args.consensus == "pbft" else 2)

    with infra.notification.notification_server(args.notify_server) as notifications:
        # Lua apps do not support notifications
        # https://github.com/microsoft/CCF/issues/415
        notifications_queue = (
            notifications.get_queue()
            if (args.package == "liblogging" and args.consensus == "raft")
            else None
        )

        with infra.network.network(
            hosts, args.binary_dir, args.debug_nodes, args.perf_nodes, pdb=args.pdb
        ) as network:
            network.start_and_join(args)
            network = test_quote(network, args, notifications_queue)
            network = test_user(network, args, notifications_queue)


if __name__ == "__main__":

    def add(parser):
        parser.add_argument(
            "--oesign", help="Path oesign binary", type=str, required=True
        )

    args = infra.e2e_args.cli_args(add=add)

    if args.enclave_type == "virtual":
        LOG.warning("This test can only run in real enclaves, skipping")
        sys.exit(0)

    notify_server_host = "localhost"
    args.notify_server = (
        notify_server_host
        + ":"
        + str(infra.net.probably_free_local_port(notify_server_host))
    )

    args.package = "liblogging"
    run(args)
