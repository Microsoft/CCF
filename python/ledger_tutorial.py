# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.

import ccf.ledger
import sys
from loguru import logger as LOG

# Note: It is safer to run the ledger tutorial when the service has stopped
# as all ledger files will have been written to.

# Change default log format
LOG.remove()
LOG.add(
    sys.stdout,
    format="<green>[{time:HH:mm:ss.SSS}]</green> {message}",
)

if len(sys.argv) < 2:
    print("Error: Ledger directory should be specified as first argument")
    sys.exit(1)

ledger_dir = sys.argv[1]

# SNIPPET: import_ledger

# SNIPPET: create_ledger
ledger = ccf.ledger.Ledger(ledger_dir)

# SNIPPET: target_table
target_table = "public:ccf.gov.nodes.info"

# SNIPPET_START: iterate_over_ledger
target_table_changes = 0  # Simple counter

for chunk in ledger:
    for transaction in chunk:
        # Retrieve all public tables changed in transaction
        public_tables = transaction.get_public_domain().get_tables()

        # If target_table was changed, count the number of keys changed
        if target_table in public_tables:
            # Ledger verification is happening implicitly in ccf.ledger.Ledger()
            for key, value in public_tables[target_table].items():
                target_table_changes += 1  # A key was changed
                # Log the key and value for the transaction on the target table
                # The target_table: 'public:ccf.gov.nodes.info' has already been decoded in ledger.py
                # For other tables knowledge of serialization scheme used is important.
                # If the table was using msgpack, use ccf.ledger.extract_msgpacked_data(data)
                LOG.info(f"{key} : {value}")
# SNIPPET_END: iterate_over_ledger
