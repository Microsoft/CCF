import array
import os
import json
from enum import Enum
import infra.ccf
import infra.proc
import infra.checker
import infra.node

from loguru import logger as LOG


class Consortium:
    def __init__(self, members):
        self.members = members
        members = [f"member{m}" for m in members]
        for m in members:
            infra.proc.ccall("./keygenerator", "--name={}".format(m)).check_returncode()
        self.status = infra.ccf.ServiceStatus.OPEN

    def get_members_certs(self):
        members_certs = [f"member{m}_cert.pem" for m in self.members]
        return members_certs

    def member_client_rpc_as_json(self, member_id, remote_node, *args):
        if remote_node is None:
            remote_node = self.find_primary()[0] # TODO: Remove this

        result = infra.proc.ccall(
            "./memberclient",
            f"--cert=member{member_id}_cert.pem",
            f"--privk=member{member_id}_privk.pem",
            f"--rpc-address={remote_node.host}:{remote_node.rpc_port}",
            "--ca=networkcert.pem",
            *args,
        )
        j_result = json.loads(result.stdout)
        return j_result

    def propose(self, member_id, remote_node, script=None, params=None, *args):
        if os.getenv("HTTP"):
            with remote_node.member_client() as mc:
                r = mc.rpc("propose", {"parameter": params, "script": {"text": script}})
                return (True, r.result)
        else:
            j_result = self.member_client_rpc_as_json(member_id, remote_node, *args)

            if j_result.get("error") is not None:
                return (False, j_result["error"])

            return (True, j_result["result"])

    def vote(
        self,
        member_id,
        remote_node,
        proposal_id,
        accept,
        force_unsigned=False,
        should_wait_for_global_commit=True,
    ):
        if os.getenv("HTTP"):
            script = """
            tables, changes = ...
            return true
            """
            with remote_node.member_client(member_id) as mc:
                r = mc.rpc(
                    "vote", {"ballot": {"text": script}, "id": proposal_id}, signed=True
                )
            return (True, r.result)
        else:
            j_result = self.member_client_rpc_as_json(
                member_id,
                remote_node,
                "vote",
                f"--proposal-id={proposal_id}",
                "--accept" if accept else "--reject",
                "--force-unsigned" if force_unsigned else "",
            )
            if j_result.get("error") is not None:
                return (False, j_result["error"])

        # If the proposal was accepted, wait for it to be globally committed
        # This is particularly useful for the open network proposal to wait
        # until the global hook on the SERVICE table is triggered
        if j_result["result"] and should_wait_for_global_commit:
            with remote_node.node_client(member_id) as mc:
                infra.checker.wait_for_global_commit(mc, j_result["commit"], j_result["term"], True)

        return (True, j_result["result"])

    def vote_using_majority(
        self, remote_node, proposal_id, should_wait_for_global_commit=True
    ):
        # There is no need to stop after n / 2 + 1 members have voted,
        # but this could prove to be useful in detecting errors
        # related to the voting mechanism
        majority_count = int(len(self.members) / 2 + 1)
        for i, member in enumerate(self.members):
            if i >= majority_count:
                break
            res = self.vote(
                member,
                remote_node,
                proposal_id,
                True,
                False,
                should_wait_for_global_commit,
            )
            assert res[0]
            if res[1]:
                break

        assert res
        return res[1]

    def propose_retire_node(self, member_id, remote_node, node_id):
        return self.propose(
            member_id, remote_node, None, None, "retire_node", f"--node-id={node_id}"
        )

    def retire_node(self, node, node_to_retire):
        member_id = 1
        result = self.propose_retire_node(member_id, node, node_to_retire.node_id)
        self.vote_using_majority(node, result[1]["id"])

        with node.member_client() as c:
            id = c.request(
                "read", {"table": "ccf.nodes", "key": node_to_retire.node_id}
            )
            assert c.response(id).result["status"].decode() == infra.node.NodeStatus.RETIRED.name

    def propose_trust_node(self, member_id, remote_node, node_id):
        return self.propose(
            member_id, remote_node, None, None, "trust_node", f"--node-id={node_id}"
        )

    def trust_node(self, remote_node, node_id):
        if not self._check_node_exists(remote_node, node_id, infra.node.NodeStatus.PENDING):
            raise ValueError(f"Node {node_id} does not exist in state PENDING")

        member_id = 1
        result = self.propose_trust_node(member_id, remote_node, node_id)
        self.vote_using_majority(remote_node, result[1]["id"])

        if not self._check_node_exists(remote_node, node_id, infra.node.NodeStatus.TRUSTED):
            raise ValueError(f"Node {node_id} does not exist in state TRUSTED")

    def propose_add_member(self, member_id, remote_node, new_member_cert):
        return self.propose(
            member_id,
            remote_node,
            None,
            None,
            "add_member",
            f"--member-cert={new_member_cert}",
        )

    def open_network(self, node):
        """
        Assuming a network in state OPENING, this functions creates a new
        proposal and make members vote to transition the network to state
        OPEN.
        """
        script = None
        if os.getenv("HTTP"):
            script = """
            tables = ...
            return Calls:call("open_network")
            """
        result = self.propose(1, node, script, None, "open_network")
        self.vote_using_majority(node, result[1]["id"])
        self.check_for_service(node, infra.ccf.ServiceStatus.OPEN)

    def add_users(self, node, users):
        if os.getenv("HTTP"):
            with node.member_client() as mc:
                for u in users:
                    user_cert = []
                    with open(f"user{u}_cert.pem") as cert:
                        user_cert = [ord(c) for c in cert.read()]
                    script = """
                    tables, user_cert = ...
                    return Calls:call("new_user", user_cert)
                    """
                    r = mc.rpc(
                        "propose", {"parameter": user_cert, "script": {"text": script}}
                    )
                    with node.member_client(2) as mc2:
                        script = """
                        tables, changes = ...
                        return true
                        """
                        r = mc2.rpc(
                            "vote",
                            {"ballot": {"text": script}, "id": r.result["id"]},
                            signed=True,
                        )
        else:
            for u in users:
                result = self.propose(
                    1, node, None, None, "add_user", f"--user-cert=user{u}_cert.pem"
                )
                self.vote_using_majority(node, result[1]["id"])

    def set_lua_app(self, node, app_script):
        result = self.propose(
            1, node, None, None, "set_lua_app", f"--lua-app-file={app_script}"
        )
        self.vote_using_majority(node, result[1]["id"])

    def accept_recovery(self, node, sealed_secrets):
        result = self.propose(
            1, node, None, None, "accept_recovery", f"--sealed-secrets={sealed_secrets}"
        )
        self.vote_using_majority(node, result[1]["id"])

    def wait_for_all_nodes_to_be_trusted(self, node, timeout=3):
        for n in self.nodes:
            self._wait_for_node_to_exist_in_store(
                node, n.node_id, infra.node.NodeStatus.TRUSTED
            )

    def check_for_service(self, node, status):
        """
        Check via the member frontend of the given node that the certificate
        associated with current CCF service signing key has been recorded in
        the KV store with the appropriate status.
        """
        with node.member_client(format="json") as c:
            rep = c.do(
                "query",
                {
                    "text": """tables = ...
                    return tables["ccf.service"]:get(0)"""
                },
            )
            current_status = rep.result["status"]
            current_cert = array.array("B", rep.result["cert"]).tobytes()

            expected_cert = open("networkcert.pem", "rb").read()
            assert (
                current_cert == expected_cert
            ), "Current service certificate did not match with networkcert.pem"
            assert (
                current_status == status.name
            ), f"Service status {current_status} (expected {status.name})"

    def _check_node_exists(self, remote_node, node_id, node_status=None):
        with remote_node.member_client() as c:
            rep = c.do("read", {"table": "ccf.nodes", "key": node_id})

            if rep.error is not None or (
                node_status and rep.result["status"].decode() != node_status.name
            ):
                return False

        return True

    def wait_for_node_to_exist_in_store(
        self, remote_node, node_id, node_status=None, timeout=3
    ):
        exists = False
        for _ in range(timeout):
            if self._check_node_exists(remote_node, node_id, node_status):
                exists = True
                break
            time.sleep(1)
        if not exists:
            raise TimeoutError(
                f"Node {node_id} has not yet been recorded in the store"
                + getattr(node_status, f" with status {node_status.name}", "")
            )
