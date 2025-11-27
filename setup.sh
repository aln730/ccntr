#! /usr/bin/bash

# Setup the environment to work on the Simple Container Engine.
#
# Uses sudo to run commands with privileges.
# 
# Setup steps:
# 1. extract the container image
# 2. prepare the cgroup root of container cgroups
# 3. prepare the host bridge for container networking

# Name of the image (its directory) to create the container from.
IMAGE="debian-stable+python+iproute2"

# Mount point of the cgroup FS.
CGROUP_FS="./cgroup"
# Root cgroup of the containers' cgroups.
ROOT_CGROUP="containers"
# Cgroup controllers that must be enabled.
CGROUP_CTRLRS="+cpu +memory"

# Host bridge for container networking.
HOST_BRIDGE="contbr0"
# Host bridge's IP address.
HOST_BRIDGE_IP="192.168.42.1/24"

# DISPLAY ROUTINES {{{
if tput colors >/dev/null 2>&1; then
    reset="$(tput sgr0)"

    bold="$(tput bold)"
    # tput does not have a default capability to turn bold off, but most often
    # it is 21.
    normal="\e[21m"
    underline="$(tput smul)"
    nounderline="$(tput rmul)"

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

[ -d "$IMAGE" ] && die "image directory $bold$underline$IMAGE$normal$nounderline already exists, run ${underline}make undosetup$nounderline before running $underline$0$nounderline again"
[ "$(stat --file-system /sys/fs/cgroup --format=%T)" != cgroup2fs ] && die "cgroup setup mode is not unified (a.k.a. cgroup v2): see https://rootlesscontaine.rs/getting-started/common/cgroup2/"
[ -d "$CGROUP_FS" ] && die "mount point of cgroup FS $bold$underline$CGROUP_FS$nounderline$normal already exists, run ${underline}make undosetup$nounderline before running $underline$0$nounderline again"
ip link show "$HOST_BRIDGE" > /dev/null 2>&1 && die "host bridge $bold$HOST_BRIDGE$normal already exists, run ${underline}make undosetup$nounderline before running $underline$0$nounderline again"

echo_info "extracting the container image $bold$underline$IMAGE.tar.xz$nounderline$normal"

mkdir "$IMAGE" ||
    die "failed creating image directory $bold$underline$IMAGE$nounderline$normal"
tar -xf "$IMAGE".tar.xz -C"$IMAGE" ||
    die "failed extracting image archive $bold$underline$IMAGE.tar.xz$nounderline$normal to $bold$underline$IMAGE$nounderline$normal"

echo_info "container image: $bold$underline$IMAGE$nounderline$normal"

echo_info "preparing the cgroup filesystem"

mkdir "$CGROUP_FS" ||
    die "failed creating directory to mount the cgroup FS at $bold$underline$CGROUP_FS$nounderline$normal"
sudo mount -t cgroup2 cgroup2 "$CGROUP_FS" ||
    die "failed mounting cgroup2 FS at $bold$underline$CGROUP_FS$nounderline$normal"
echo "$CGROUP_CTRLRS" | sudo tee "$CGROUP_FS"/cgroup.subtree_control > /dev/null ||
    die "failed enabling controllers $bold$underline$CGROUP_CTRLRS$nounderline$normal in cgroups"

echo_info "cgroup filesystem: $bold$underline$CGROUP_FS$nounderline$normal"

echo_info "preparing the root cgroup"

sudo mkdir -p "$CGROUP_FS/$ROOT_CGROUP" ||
    die "failed creating root cgroup $bold$underline$ROOT_CGROUP$nounderline$normal for containers"
echo "$CGROUP_CTRLRS" | sudo tee "$CGROUP_FS/$ROOT_CGROUP"/cgroup.subtree_control > /dev/null ||
    die "failed enabling controllers $bold$underline$CGROUP_CTRLRS$nounderline$normal in root container cgroups $bold$underline$ROOT_CGROUP$nounderline$normal"

echo_info "root cgroup: $bold$ROOT_CGROUP$normal"

echo_info "preparing the host bridge for container networking"

sudo ip link add name "$HOST_BRIDGE" type bridge ||
    die "failed creating host bridge $bold$HOST_BRIDGE$normal for containers"
sudo ip addr add "$HOST_BRIDGE_IP" dev "$HOST_BRIDGE" ||
    die "failed assigning IP address $bold$HOST_BRIDGE_IP$normal to host bridge $bold$HOST_BRIDGE$normal for containers"
sudo ip link set dev "$HOST_BRIDGE" up

echo_info "host bridge: $bold$HOST_BRIDGE$normal ($bold$HOST_BRIDGE_IP$normal)"

echo_info "done"
