# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.
import infra.ccf
import infra.proc
import infra.notification
import infra.net
import infra.e2e_args
from threading import Timer
import time
import random
import suite.test_requirements as reqs
import infra.logging_app as app

from loguru import logger as LOG

# pbft will store up to 34 of each message type (pre-prepare/prepare/commit) and retransmit these messages to replicas that are behind, enabling catch up.
# If a replica is too far behind then we need to send entries from the ledger, which is one of the things we want to test here.
# By sending 18 RPC requests and a getCommit for each of them (what raft consideres as a read pbft will process as a write),
# we are sure that we will have to go via the ledger to help late joiners catch up (total 36 reqs > 34)
TOTAL_REQUESTS = 9  # x2 is 18 since LoggingTxs app sends a private and a public request for each tx index


def timeout_handler(node, suspend, election_timeout):
    if suspend:
        # We want to suspend the nodes' process so we need to initiate a new timer to wake it up eventually
        node.suspend()
        next_timeout = random.uniform(2 * election_timeout, 3 * election_timeout)
        LOG.info(f"New timer set for node {node.node_id} is {next_timeout} seconds")
        t = Timer(next_timeout, timeout_handler, args=[node, False, 0])
        t.start()
    else:
        node.resume()


def update_view_info(network, view_info):
    try:
        cur_primary, cur_view = network.find_primary()
        view_info[cur_view] = cur_primary.node_id
    except TimeoutError:
        LOG.warning("Trying to access a suspended network")


def get_node_local_commit(node):
    with node.node_client() as c:
        r = c.get("debug/getLocalCommit")
        return r.seqno, r.global_commit


def wait_for_late_joiner(old_node, late_joiner, timeout=30):
    old_node_lc, old_node_gc = get_node_local_commit(old_node)
    LOG.success(
        f"node {old_node.node_id} is at state local_commit:{old_node_lc}, global_commit:{old_node_gc}"
    )
    end = time.time() + timeout
    while time.time() <= end:
        try:
            lc, gc = get_node_local_commit(late_joiner)
            LOG.success(
                f"late joiner {late_joiner.node_id} is at state local_commit:{lc}, global_commit:{gc}"
            )
            if lc >= old_node_lc:
                return
            time.sleep(1)
        except (
            TimeoutError,
            infra.clients.CCFConnectionException,
        ):
            LOG.warning(f"late joiner {late_joiner.node_id} isn't quite ready yet")


@reqs.description("Suspend nodes")
def test_suspend_nodes(network, args, nodes_to_keep):
    cur_primary, _ = network.find_primary()

    # first timer determines after how many seconds each node will be suspended
    timeouts = []
    for i, node in enumerate(nodes_to_keep):
        # if pbft suspend half of them including the primary
        if i % 2 != 0 and args.consensus == "pbft":
            continue
        LOG.success(f"Will suspend node with id {node.node_id}")
        t = random.uniform(0, 2)
        LOG.info(f"Initial timer for node {node.node_id} is {t} seconds...")
        timeouts.append((t, node))

    for t, node in timeouts:
        suspend_time = (
            args.pbft_view_change_timeout / 1000
            if args.consensus == "pbft"
            else args.raft_election_timeout / 1000
        )
        if node.node_id == cur_primary.node_id and args.consensus == "pbft":
            # if pbft suspend the primary for more than twice the election timeout
            # in order to make sure view changes will be triggered
            suspend_time = 2.5 * suspend_time
        tm = Timer(t, timeout_handler, args=[node, True, suspend_time])
        tm.start()
    return network


def run(args):
    hosts = ["localhost", "localhost", "localhost"]

    LOG.info(f"setting seed to {args.seed}")
    random.seed(args.seed)
    txs = app.LoggingTxs()

    with infra.ccf.network(
        hosts, args.binary_dir, args.debug_nodes, args.perf_nodes, pdb=args.pdb, txs=txs
    ) as network:
        network.start_and_join(args)
        original_nodes = network.get_joined_nodes()
        view_info = {}
        update_view_info(network, view_info)

        app.test_run_txs(network=network, args=args, num_txs=TOTAL_REQUESTS)
        update_view_info(network, view_info)

        nodes_to_kill = [network.find_any_backup()]
        nodes_to_keep = [n for n in original_nodes if n not in nodes_to_kill]

        # check that a new node can catch up after all the requests
        late_joiner = network.create_and_trust_node(args.package, "localhost", args)
        nodes_to_keep.append(late_joiner)

        # some requests to be processed while the late joiner catches up
        # (no strict checking that these requests are actually being processed simultaneously with the node catchup)
        app.test_run_txs(
            network=network,
            args=args,
            num_txs=int(TOTAL_REQUESTS / 2),
            nodes=original_nodes,  # doesn't contain late joiner
            verify=False,  # will try to verify for late joiner and it might not be ready yet
        )

        wait_for_late_joiner(original_nodes[0], late_joiner)

        # kill the old node(s) and ensure we are still making progress
        for backup_to_retire in nodes_to_kill:
            LOG.success(f"Stopping node {backup_to_retire.node_id}")
            backup_to_retire.stop()

        # check nodes are ok after we killed one off
        app.test_run_txs(
            network=network,
            args=args,
            nodes=nodes_to_keep,
            num_txs=len(nodes_to_keep),
            timeout=30,
            ignore_failures=True,
            # in the event of an early view change due to the late joiner this might
            # take longer than usual to complete and we don't want the test to break here
        )

        test_suspend_nodes(network, args, nodes_to_keep)

        # run txs while nodes get suspended
        app.test_run_txs(
            network=network,
            args=args,
            num_txs=4 * TOTAL_REQUESTS,
            ignore_failures=True,
        )

        update_view_info(network, view_info)

        # check nodes have resumed normal execution before shutting down
        app.test_run_txs(network=network, args=args, num_txs=len(nodes_to_keep))

        # we have asserted that all nodes are caught up
        # assert that view changes actually did occur
        assert len(view_info) > 1

        LOG.success("----------- views and primaries recorded -----------")
        for view, primary in view_info.items():
            LOG.success(f"view {view} - primary {primary}")


if __name__ == "__main__":

    def add(parser):
        parser.add_argument(
            "--seed",
            help="seed used to randomise the node suspension timeouts",
            default=random.randint(1, 10),
        )

    args = infra.e2e_args.cli_args(add)
    if args.js_app_script:
        args.package = "libjs_generic"
    elif args.app_script:
        args.package = "liblua_generic"
    else:
        args.package = "liblogging"
    run(args)
