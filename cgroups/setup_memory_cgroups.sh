#!/usr/bin/env bash

set -e

# make cgroups.
# assume cgroupv2 mountpoint is at /sys/fs/cgroup
mkdir -p /sys/fs/cgroup/grp1
mkdir -p /sys/fs/cgroup/grp2
mkdir -p /sys/fs/cgroup/grp1/leaf1
mkdir -p /sys/fs/cgroup/grp1/leaf2

echo "+memory" > /sys/fs/cgroup/cgroup.subtree_control
echo "+memory" > /sys/fs/cgroup/grp1/cgroup.subtree_control
echo "+memory" > /sys/fs/cgroup/grp2/cgroup.subtree_control
