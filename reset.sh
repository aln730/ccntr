#! /usr/bin/bash

# Uses sudo to run commands with privileges.

IMAGE="debian-stable+python+iproute2"
CGROUP_FS="./cgroup"
ROOT_CGROUP="containers"
VETH_CONT_IFNAME="veth-h"

# DISPLAY ROUTINES {{{
if tput colors >/dev/null 2>&1; then
    reset="$(tput sgr0)"

    bold="$(tput bold)"
    underline="$(tput smul)"

    red="$(tput setaf 1)"
    yellow="$(tput setaf 3)"
    blue="$(tput setaf 4)"
fi

echo_info() {
    echo -e "$blue$*$reset"
}

echo_warn() {
    echo -e "$yellow$*$reset"
}

echo_err() {
    echo -e "$red$*$reset" 1>&2
}

die() {
    echo_err "$*"
    exit 1
}
# DISPLAY ROUTINES }}}

if [ $# -lt 1 ]; then
    echo -e "Usage: $bold$red$0$reset CONT_NAME"
    echo
    echo -e "\t$bold${red}CONT_NAME$reset: name of the previously created container"
    exit 2
fi >&2

cont_name="$1"

if [ -d "$CGROUP_FS/$ROOT_CGROUP/$cont_name" ]; then
    echo_info "killing all processes in cgroups under $bold$underline$CGROUP_FS/$ROOT_CGROUP$reset"

    echo 1 | sudo tee "$CGROUP_FS/$ROOT_CGROUP"/cgroup.kill > /dev/null
fi

if mount | grep -q "$IMAGE"; then
    echo_info "unmounting container image at $bold$underline$IMAGE-$cont_name/run$reset"

    sudo umount "$IMAGE-$cont_name"/run
fi

if [ -d "$IMAGE-$cont_name" ]; then
    echo_info "removing mountpoint of container image at $bold$underline$IMAGE-$cont_name$reset"

    sudo rm -fr "$IMAGE-$cont_name"
fi

if ip link show dev "$VETH_CONT_IFNAME" > /dev/null 2>&1; then
    echo_info "deleting veth pair $bold$VETH_CONT_IFNAME$reset"

    sudo ip link del dev "$VETH_CONT_IFNAME"
fi

