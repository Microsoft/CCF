# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.
import e2e_args
import infra.ccf

import logging
from time import gmtime, strftime
import csv
import random
from enum import IntEnum

from iso3166 import countries

from loguru import logger as LOG


class TransactionType(IntEnum):
    PAYMENT = 1
    TRANSFER = 2
    CASH_OUT = 3
    DEBIT = 4
    CASH_IN = 5


KNOWN_COUNTRIES = ["us", "gbr", "fr", "grc"]


def run(args):
    hosts = ["localhost"]

    with infra.ccf.network(
        hosts, args.build_dir, args.debug_nodes, args.perf_nodes, pdb=args.pdb
    ) as network:
        primary, others = network.start_and_join(args)

        script = "if amt == 99 then return true else return false end"
        if args.lua_script is not None:
            data = []
            with open(args.lua_script, "r") as f:
                data = f.readlines()
            script = "".join(data)

        # TODO: Use check_commit for Write RPCs
        regulator = (0, "gbr", script)
        banks = [(1, "us", 99), (1, "gbr", 29), (2, "grc", 99), (2, "fr", 29)]

        with primary.management_client() as mc:
            with primary.user_client(format="msgpack", user_id=regulator[0] + 1) as c:
                check_commit = infra.ccf.Checker(mc)
                check = infra.ccf.Checker()

                check(
                    c.rpc(
                        "REG_register",
                        {
                            "country": countries.get(regulator[1]).numeric,
                            "script": regulator[2],
                        },
                    ),
                    result=regulator[0],
                )
                check(
                    c.rpc("REG_get", {"id": regulator[0]}),
                    result=[
                        countries.get(regulator[1]).numeric.encode(),
                        regulator[2].encode(),
                    ],
                )

            LOG.debug(f"User {regulator[0]} successfully registered as regulator")

        for bank in banks:
            with primary.user_client(format="msgpack", user_id=bank[0] + 1) as c:
                check_commit = infra.ccf.Checker(mc)
                check = infra.ccf.Checker()

                check(
                    c.rpc("BK_register", {"country": countries.get(bank[1]).numeric}),
                    result=bank[0],
                )
                check(
                    c.rpc("BK_get", {"id": bank[0]}),
                    result=countries.get(bank[1]).numeric.encode(),
                )
            LOG.debug(f"User {bank[0]} successfully registered as bank")

        LOG.success(f"{1} regulator and {len(banks)} bank(s) successfully setup")

        tx_id = 0  # Tracks how many transactions have been issued
        bank_id = banks[0][0] + 1
        LOG.info(f"Loading scenario file as bank {bank_id}")

        with primary.user_client(format="msgpack", user_id=regulator[0] + 1) as reg_c:

            with primary.user_client(format="msgpack", user_id=bank_id) as c:
                with open(args.datafile, newline="") as f:
                    datafile = csv.DictReader(f)
                    for row in datafile:
                        json_tx = {
                            "src": row["nameOrig"],
                            "dst": row["nameDest"],
                            "amt": row["amount"],
                            "type": TransactionType[row["type"]].value,
                            "timestamp": strftime(
                                "%a, %d %b %Y %H:%M:%S +0000", gmtime()
                            ),
                            "src_country": countries.get(
                                random.choice(KNOWN_COUNTRIES)
                            ).numeric,
                            "dst_country": countries.get(
                                random.choice(KNOWN_COUNTRIES)
                            ).numeric,
                        }

                        check(c.rpc("TX_record", json_tx), result=tx_id)
                        tx_id += 1

                        # TODO separate script
                        reg_c.rpc("REG_poll_flagged", {})
                LOG.success("Scenario file successfully loaded")


if __name__ == "__main__":

    def add(parser):
        parser.add_argument(
            "--datafile", help="Load an existing scenario file (csv)", type=str
        )
        parser.add_argument(
            "--lua-script", help="Regulator checker loaded as lua script file", type=str
        )

    args = e2e_args.cli_args(add)
    args.package = args.app_script and "libluagenericenc" or "libloggingenc"
    run(args)
