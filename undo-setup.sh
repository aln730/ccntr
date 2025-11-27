#! /usr/bin/bash

# Try to clean the results of the steps done by setup.
# Does not error out if a step cannot be cleaned because there is just no result
# to clean (i.e., the step did not work).

# Host bridge for container networking.
HOST_BRIDGE="contbr0"
# Mount point of the cgroup FS.
CGROUP_FS="./cgroup"
# Name of the image (its directory) to create the container from.
IMAGE="debian-stable+python+iproute2"

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

if ip link show dev "$HOST_BRIDGE" > /dev/null 2>&1; then
    echo_info "deleting host bridge $bold$HOST_BRIDGE$reset"

    sudo ip link del dev "$HOST_BRIDGE"
fi

if mountpoint --quiet "$CGROUP_FS"; then
    echo_info "removing local cgroup filesystem at $bold$underline$CGROUP_FS$reset"

    sudo umount "$CGROUP_FS"
fi

if [ -d "$CGROUP_FS" ]; then
    echo_info "removing mountpoint of local cgroup filesystem at $bold$underline$CGROUP_FS$reset"

    rm -fd "$CGROUP_FS"
fi

if [ -d "$IMAGE" ]; then
    echo_info "removing container image at $bold$underline$IMAGE$reset"

    rm -rf "$IMAGE"
fi

