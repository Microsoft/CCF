#!/bin/bash
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the Apache 2.0 License.

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# Shell scripts
find . -type f -regex ".*\.sh$" | grep -E -v "^./3rdparty/" | xargs shellcheck -s bash -e SC2044,SC2002,SC1091

# TODOs
"$SCRIPT_DIR"/check-todo.sh src

# C/C++ Format
"$SCRIPT_DIR"/check-format.sh src samples

# Copyright notice headers
python3.7 "$SCRIPT_DIR"/notice-check.py

# CMake format
"$SCRIPT_DIR"/check-cmake-format.sh cmake samples src tests CMakeLists.txt

# Virtual Environment w/ dependencies for Python steps
if [ ! -f "scripts/env/bin/activate" ]
    then
        python3.7 -m venv scripts/env
fi

source scripts/env/bin/activate
pip --disable-pip-version-check install black pylint 1>/dev/null

# Python code format
black --check tests/ scripts/*.py

# Install test dependencies before linting
pip --disable-pip-version-check install -U -r tests/requirements.txt 1>/dev/null

# Python lint
find tests/ -type f -name "*.py" -exec python -m pylint {} +