# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.

import infra.checker
import time
import http
import ccf.clients
from collections import defaultdict

from loguru import logger as LOG


class LoggingTxs:
    def __init__(self, user_id=0):
        self.pub = defaultdict(list)
        self.priv = defaultdict(list)
        self.idx = 0
        self.user = f"user{user_id}"

    def issue(
        self,
        network,
        number_txs=1,
        on_backup=False,
        repeat=False,
    ):
        remote_node, _ = network.find_primary()
        if on_backup:
            remote_node = network.find_any_backup()

        LOG.info(f"Applying {number_txs} logging txs to node {remote_node.node_id}")

        with remote_node.client(self.user) as uc:
            check_commit = infra.checker.Checker(uc)

            if repeat:
                idx = self.idx
            else:
                self.idx += 1
                idx = self.idx

            for _ in range(number_txs):
                priv_msg = f"Private message at idx {idx} [{len(self.priv[idx])}]"
                rep_priv = uc.post(
                    "/app/log/private",
                    {
                        "id": self.idx,
                        "msg": priv_msg,
                    },
                )
                check_commit(rep_priv, result=True)
                self.priv[idx].append({"msg": priv_msg, "seqno": rep_priv.seqno, "view": rep_priv.view})

                # Public records do not handle historical queries
                if not repeat:
                    pub_msg = f"Public message at idx {idx} [{len(self.pub[idx])}]"
                    rep_pub = uc.post(
                        "/app/log/public",
                        {
                            "id": self.idx,
                            "msg": pub_msg,
                        },
                    )
                    check_commit(rep_pub, result=True)
                    self.pub[idx].append({"msg": pub_msg, "seqno": rep_priv.seqno, "view": rep_priv.view})

        network.wait_for_node_commit_sync()

    def verify(self, network, timeout=3):
        LOG.info("Verifying all logging txs")
        LOG.error(self.pub)
        LOG.error(self.priv)

        n = network.get_joined_nodes()[0]
        for pub_idx, pub_value in self.pub.items():
            assert (
                len(pub_value) == 1
            ), "Public records do not handle historical queries"

            entry = pub_value[0]
            self._verify_tx(n, pub_idx, entry["msg"],
              entry["seqno"],
              entry["view"],
              priv=False, timeout=timeout)

        for priv_idx, priv_value in self.priv.items():
            for v in priv_value:
                self._verify_tx(
                    n,
                    priv_idx,
                    v["msg"],
                    v["seqno"],
                    v["view"],
                    priv=True,
                    historical=(v != priv_value[-1]),
                    timeout=timeout,
                )

    def _verify_tx(self, node, idx, msg, seqno, view, priv=True, historical=False, timeout=3):
        if historical and not priv:
            raise ValueError(
                "Historical queries are only implemented with private records"
            )

        cmd = "/app/log/private" if priv else "/app/log/public"

        headers = {}
        if historical:
            LOG.error("Historical")
            cmd = "/app/log/private/historical"
            timeout += 15
            headers = {
                ccf.clients.CCF_TX_VIEW_HEADER: str(view),
                ccf.clients.CCF_TX_SEQNO_HEADER: str(seqno),
            }


        found = False
        end_time = time.time() + timeout
        while time.time() < end_time:
            with node.client(self.user) as uc:
                rep = uc.get(f"{cmd}?id={idx}", headers=headers)
                if rep.status_code == http.HTTPStatus.OK:
                        infra.checker.Checker(uc)(
                            rep,
                            result={"msg": msg},
                        )
                        found = True
                        break
                elif rep.status_code == http.HTTPStatus.NOT_FOUND:
                        LOG.warning("User frontend is not yet opened")
                        time.sleep(0.1)
                        break

                if historical:
                    if rep.status_code == http.HTTPStatus.ACCEPTED:
                        retry_after = rep.headers.get("retry-after")
                        if retry_after is None:
                            raise ValueError(
                                f"Response with status {rep.status_code} is missing 'retry-after' header"
                            )
                        retry_after = int(retry_after)
                        LOG.info(f"Sleeping for {retry_after}s waiting for historical query processing...")
                        time.sleep(retry_after)
                    elif rep.status_code == http.HTTPStatus.NO_CONTENT:
                        raise ValueError(
                            f"Historical query response claims there was no write to {idx} at {view}.{seqno}"
                        )
                    else:
                        raise ValueError(
                            f"Unexpected response status code {rep.status_code}: {rep.body}"
                        )

        if not found:
            raise TimeoutError(f"Unable to retrieve entry at {idx} after {timeout}s")
