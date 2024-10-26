#!/usr/bin/env bash

set -e

touch swap.file
chmod 0600 swap.file

# mkswap fails when swap.file has holes.
# that means truncate would fail here.
fallocate -l 1GB swap.file
mkswap swap.file
swapon swap.file

