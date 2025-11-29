/*
 * libcontain -- Companion helper library of contain, a simple container engine
 *
 * v0 "base": base empty version.
 *
 * See "libcontain.h".
 */

#define _GNU_SOURCE

#include "libcontain.h"

#include <netlink/netlink.h>
#include <netlink/route/link/veth.h>
#include <netlink/route/link/bridge.h>
#include <netlink/route/addr.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/mount.h>
#include <sys/stat.h>

void contfs_set(struct container_fs *fs, const char* hostname,
        const char* image) {
    // Build the name of the container's filesystem directory from its hostname
    // and its image's name.
    // Use basename of provided image to make it possible to pass a path to the
    // image directory, for convenience.
    snprintf(fs->base, PATH_MAX, "%s-%s", basename(image), hostname);

    strcpy(fs->root, fs->base);
    strcat(fs->root, "/run");
    strcat(strcpy(fs->diff, fs->base), "/.diff");
    strcat(strcpy(fs->workdir, fs->base), "/.workdir");
}


/* Make the filesystem for the container.
 *
 * 1. create the necessary directories for the mountpoint
 * 2. mount the filesystem
 *
 * Parameters:
 * * cont_fs: the representation of the filesystem of the container
 * * image: the path to the image of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int contfs_make(const struct container_fs *cont_fs, const char* image) {
    /*** STUDENT CODE BELOW (q10) ***/

    // Create the directories used to mount the container FS.
    // libcontain: contfs_mkdirs
    return 0;

    /*** STUDENT CODE ABOVE (q10) ***/

    if (contfs_mount(cont_fs, image) < 0) {
        if (contfs_rmdirs(cont_fs) < 0)
            warn("failed removing container filesystem directories");
        raise_msg("failed mounting container filesystem");
    }

    return 0;
}

int contfs_demake(const struct container_fs *cont_fs) {
    struct stat statbuf;
    // To avoid errors because the student has not yet written the code to mount
    // the container filesystem (as an overlay FS or at all), check that the
    // directory exist.
    if (stat(cont_fs->root, &statbuf) < 0) {
        return 0;
    } else {
        // MNT_DETACH to lazily unmount: for timing reasons, the container's
        // filesystem may still be used by the terminating container process.
        if (umount2(cont_fs->root, MNT_DETACH) < 0)
            raise_err("failed unmounting container filesystem at \"%s\"",
                    cont_fs->root);
    }

    return 0;
}

/* Prepare the options to mount the filesystem of the container as an overlay.
 *
 * Parameters:
 * * mount_options: text buffer where to write the options
 * * lowerdir_path: path to the lower directory of the overlay filesystem
 * * upperdir_path: path to the upper directory of the overlay filesystem
 * * workdir_path: path to the work directory of the overlay filesystem
 *
 * At most CONTFS_MOUNT_OPTIONS_SZ (including terminating null byte) are written
 * to mount_options.
 *
 * Return: number of characters (excluding null byte) written to mount_options.
 * It means that a return value of CONTFS_MOUNT_OPTIONS_SZ or more indicated a
 * truncated output (this is an error).
 */
static inline int contfs_make_mount_options(char* mount_options,
        const char* lowerdir_path, const char* upperdir_path,
        const char* workdir_path) {
    return snprintf(mount_options, CONTFS_MOUNT_OPTIONS_SZ,
            "lowerdir=%s,upperdir=%s,workdir=%s",
            lowerdir_path, upperdir_path, workdir_path);
}

/*
 * Mount the filesystem of the container.
 *
 * Parameters:
 * * cont_fs: the representation of the filesystem of the container
 * * image: the path to the image of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int contfs_mount(const struct container_fs *cont_fs, const char* image) {
    // Also used to tell whether the filesystem is mounted as a bind mount or as
    // an overlay filesystem (it contains some string in the latter case).
    char mount_options[CONTFS_MOUNT_OPTIONS_SZ];

    /*** STUDENT CODE BELOW (q16) ***/

    // Prepare the mount options in mount_options for the overlay FS.
    // libcontain: contfs_make_mount_options
    mount_options[0] = 0;

    /*** STUDENT CODE ABOVE (q16) ***/

    if (mount_options[0] == 0) {        // Bind mount
        /*** STUDENT CODE BELOW (q10) ***/

        // Mount the container FS as a bind mount.
        // mount(2), MS_BIND
        return 0;

        /*** STUDENT CODE ABOVE (q10) ***/
    } else {                            // Overlay filesystem
        /*** STUDENT CODE BELOW (q16) ***/

        // Mount the container FS as an overlay filesystem.
        // mount(2)
        return 0;

        /*** STUDENT CODE ABOVE (q16) ***/
    }

    return 0;
}

int contfs_mkdirs(const struct container_fs *cont_fs) {
    int the_errno;

    if (mkdir(cont_fs->base, S_IRWXU | S_IRGRP | S_IXGRP) < 0) {
        if (errno != EEXIST)
            raise_err("failed creating base directory \"%s\"", cont_fs->base);
    }
    if (mkdir(cont_fs->root, S_IRWXU | S_IRGRP | S_IXGRP) < 0) {
        if (errno != EEXIST) {
            the_errno = errno;
            if (rmdir(cont_fs->base) < 0)
                warn("failed removing base directory \"%s\"", cont_fs->base);
            errno = the_errno;
            raise_err("failed creating root directory");
        }
    }
    if (mkdir(cont_fs->workdir, S_IRWXU | S_IRGRP | S_IXGRP) < 0) {
        if (errno != EEXIST) {
            the_errno = errno;
            if (rmdir(cont_fs->root) < 0)
                warn("failed removing root directory \"%s\"", cont_fs->root);
            if (rmdir(cont_fs->base) < 0)
                warn("failed removing base directory \"%s\"", cont_fs->base);
            errno = the_errno;
            raise_err("failed creating workdir directory");
        }
    }
    if (mkdir(cont_fs->diff, S_IRWXU | S_IRGRP | S_IXGRP) < 0) {
        if (errno != EEXIST) {
            the_errno = errno;
            if (rmdir(cont_fs->workdir) < 0)
                warn("failed removing workdir directory \"%s\"",
                        cont_fs->workdir);
            if (rmdir(cont_fs->root) < 0)
                warn("failed removing root directory \"%s\"", cont_fs->root);
            if (rmdir(cont_fs->base) < 0)
                warn("failed removing base directory \"%s\"", cont_fs->base);
            errno = the_errno;
            raise_err("failed creating diff directory");
        }
    }

    return 0;
}

int contfs_rmdirs(const struct container_fs *cont_fs) {
    if (rmdir(cont_fs->root) < 0)
        raise_err("failed removing root directory \"%s\"", cont_fs->root);
    if (rmdir(cont_fs->workdir) < 0)
        raise_err("failed removing workdir directory \"%s\"", cont_fs->workdir);
    if (rmdir(cont_fs->diff) < 0)
        raise_err("failed removing diff directory \"%s\"", cont_fs->diff);
    if (rmdir(cont_fs->base) < 0)
        raise_err("failed removing base directory \"%s\"", cont_fs->base);

    return 0;
}

int contfs_mount_pseudo_fs(const struct container_fs *cont_fs) {
    // This procedure should run "inside the container", i.e., after mountpoints
    // are unshared.

    char pathbuf[PATH_MAX], *pathbuf_end;
    // Save some place to copy last component (longest: "/proc").
    strncpy(pathbuf, cont_fs->root, PATH_MAX-5);
    pathbuf_end = pathbuf + strlen(pathbuf);

    // proc, sys, and tmp mountpoints are supposed to already exist in the
    // container image.
    memcpy(pathbuf_end, "/proc\0", 6);
    if (mount("proc", pathbuf, "proc", 0, NULL) < 0)
        raise_err("failed mounting proc pseudo-filesystem at \"%s\"", pathbuf);
    memcpy(pathbuf_end, "/sys\0", 5);
    if (mount(NULL, pathbuf, "sysfs", 0, NULL) < 0)
        raise_err("failed mounting sysfs pseudo-filesystem at \"%s\"", pathbuf);
    memcpy(pathbuf_end, "/tmp\0", 5);
    if (mount(NULL, pathbuf, "tmpfs", 0, NULL) < 0)
        raise_err("failed mounting tmpfs pseudo-filesystem at \"%s\"", pathbuf);

    return 0;
}

/*
 * Map the root user in the container, to the user running the program.
 *
 * Parameter:
 * * cont_pid: the PID of the process of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int cgroup_map_root_user(pid_t cont_pid) {

    char uidmap_path[PATH_MAX];
    char gidmap_path[PATH_MAX];
    char setgroups_path[PATH_MAX];

    snprintf(setgroups_path, PATH_MAX, "/proc/%d/setgroups", cont_pid);
    FILE *setgroups = fopen(setgroups_path, "w");
    if (!setgroups) {
        perror("fopen setgroups");
        return -1;
    }
    if (fprintf(setgroups, "deny") < 0) {
        perror("write setgroups");
        fclose(setgroups);
        return -1;
    }
    fclose(setgroups);

    snprintf(uidmap_path, PATH_MAX, "/proc/%d/uid_map", cont_pid);
    FILE *uidmap = fopen(uidmap_path, "w");
    if (!uidmap) {
        perror("fopen uid_map");
        return -1;
    }
    if (fprintf(uidmap, "0 %d 1\n", getuid()) < 0) {
        perror("write uid_map");
        fclose(uidmap);
        return -1;
    }
    fclose(uidmap);

    snprintf(gidmap_path, PATH_MAX, "/proc/%d/gid_map", cont_pid);
    FILE *gidmap = fopen(gidmap_path, "w");
    if (!gidmap) {
        perror("fopen gid_map");
        return -1;
    }
    if (fprintf(gidmap, "0 %d 1\n", getgid()) < 0) {
        perror("write gid_map");
        fclose(gidmap);
        return -1;
    }
    fclose(gidmap);

    return 0;
}

void cgroup_set(struct container_cgroup *cgroup, const char* hostname) {
    snprintf(cgroup->path, PATH_MAX, CGROUP_ROOT "/%s", hostname);
}


/*
 * Set up the cgroup of the container.
 *
 * Create the container's cgroup, and set its memory and CPU limits.
 *
 * Parameters:
 * * cgroup: the representation of the cgroup of the container
 * * memory_MB: the limit of memory usage of the container (in MB)
 * * cpu_perc: the limit of CPU usage of the container (as a percentage of CPU)
 *
 * Return:
 * * a file descriptor of the cgroup (i.e., its directory in the cgroup FS)
 * * -1 on error (and print an error message)
 */
int cgroup_make(struct container_cgroup *cgroup, unsigned int memory_MB,
        double cpu_perc) {
    /*** STUDENT CODE BELOW (q12) ***/

    // Create the cgroup by making its directory at the cgroup path.
    // mkdir(2), inode(7): file modes
    return 0;

    // Open (take a FD) to the cgroup's directory in cgroup->fd
    // open(2)
    return 0;

    /*** STUDENT CODE ABOVE (q12) ***/

    if (cgroup_limit_memory(cgroup, memory_MB) < 0)
        raise_msg("failed setting memory limit");

    if (cgroup_limit_cpu(cgroup, cpu_perc) < 0)
        raise_msg("failed setting CPU limit");


    return cgroup->fd;
}

int cgroup_demake(struct container_cgroup *cgroup) {
    if (close(cgroup->fd) < 0)
        raise_err("failed closing cgroup FD");
    cgroup->fd = -1;

    if (rmdir(cgroup->path) < 0)
        raise_err("failed removing cgroup");

    return 0;
}

/*
 * Set the memory limit of the cgroup of the container.
 *
 * Parameters:
 * * cgroup: the representation of the cgroup of the container
 * * memory_MB: the limit of memory usage of the container (in MB)
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int cgroup_limit_memory(const struct container_cgroup *cgroup,
        unsigned int memory_MB) {
    // FD to a control file.
    int ctrl_fd;
    // Stream to a control file.
    FILE *ctrl_file;

    memory_MB = memory_MB * 1024 * 1024;

    /*** STUDENT CODE BELOW (q14) ***/

    // Set the container's cgroup memory limit.
    // 1. take a FD to the control file for the high memory watermark;
    // 2. make a stream (FILE*) from the FD;
    // 3. write the memory limit of the container to the control file;
    // 4. close the stream (it will also close the FD).
    // openat(2), fdopen(3), fprintf(3), fclose(3)
    // CGROUP_CTRL_MEMORY_HIGH
    // cgroup doc, memory controller:
    // https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#memory
    return 0;

    /*** STUDENT CODE ABOVE (q14) ***/

    return 0;
}

/*
 * Set the CPU limit of the cgroup of the container.
 *
 * Parameters:
 * * cgroup: the representation of the cgroup of the container
 * * cpu_perc: the limit of CPU usage of the container (as a percentage of CPU)
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int cgroup_limit_cpu(const struct container_cgroup *cgroup, double cpu_perc) {
    // FD to a control file.
    int ctrl_fd;
    // Stream to a control file.
    FILE *ctrl_file;
    // Period of CPU scheduling.
    unsigned int cpu_period;
    // Calculated CPU limit from the CPU percentage and the CPU period.
    unsigned int cpu_limit;

    /*** STUDENT CODE BELOW (q15) ***/

    // Set the container's cgroup CPU limit.
    // 1. take a FD to the control file for the max CPU bandwidth;
    // 2. make a stream (FILE*) from the FD;
    // 3. read the scheduler period from the control file;
    // 4. write the CPU limit (computed from the CPU percentage limit and the
    //    scheduler period) and the scheduler period of the container to the
    //    control file;
    // 5. close the stream (it will also close the FD).
    // openat(2), fdopen(3), fscanf(3), fprintf(3), fclose(3)
    // CGROUP_CTRL_CPU_MAX
    // cgroup doc, cpu controller:
    // https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#cpu
    return 0;

    /*** STUDENT CODE ABOVE (q15) ***/

    return 0;
}

/*
 * Make the network of the container on the host side.
 *
 * 1. create a veth pair
 * 2. put one end of the pair in the container (in its network namespace)
 * 3. set up the network on the host side by plugging the host end of the veth
 *    pair in the host bridge
 *
 * Parameters:
 * * cont_pid: the PID of the process of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int contnet_make_host(pid_t cont_pid) {
    int err = 0;
    errno = 0;

    // Netlink socket.
    struct nl_sock *nlsk = nl_socket_alloc();
    if (nlsk == NULL)
        raise_err("failed opening Netlink socket");
    if ((err = nl_connect(nlsk, NETLINK_ROUTE)) < 0)
        raise_err("failed connecting socket: %s", nl_geterror(err));

    // veth endpoints: veth_host on the host, veth_cont in the container.
    struct rtnl_link *veth_cont, *veth_host = rtnl_link_veth_alloc();
    if (veth_host == NULL)
        raise_err("failed allocating veth link");
    veth_cont = rtnl_link_veth_get_peer(veth_host);

    /*** STUDENT CODE BELOW (q18) ***/

    // Set the names of the veth endpoints.
    // libnl: rtnl_link_set_name
    // HOST_VETH_NAME, CONT_VETH_NAME
    return 0;

    // Move veth_cont to the network namespace of the container.
    // libnl: rtnl_link_set_ns_pid
    return 0;

    /*** STUDENT CODE ABOVE (q18) ***/

    if (contnet_connect_host_bridge(nlsk, veth_host) < 0)
        raise_err("failed connecting host end to host bridge");

    /*** STUDENT CODE BELOW (q18) ***/

    // Create veth_host (its peer veth_cont is automatically created).
    // libnl: rtnl_link_add, flags: NLM_F_CREATE
    return 0;

    /*** STUDENT CODE ABOVE (q18) ***/

    rtnl_link_veth_release(veth_host);

    nl_socket_free(nlsk);

    return 0;
}

int contnet_connect_host_bridge(struct nl_sock *nlsk,
        struct rtnl_link *veth_host) {
    int err = 0;

    // Bridge master of veth_host.
    struct rtnl_link *host_br;
    if ((err = rtnl_link_get_kernel_by_name(nlsk, HOST_BRIDGE, &host_br)) < 0)
        raise_err("failed looking up host bridge: %s", nl_geterror(err));

    /*** STUDENT CODE BELOW (q19) ***/

    // Set veth_host up.
    // libnl: rtnl_link_set_flags
    // man: netdevice(7)
    return 0;

    // Put veth_host in its bridge master.
    // libnl: rtnl_link_set_master, rtnl_link_get_ifindex
    return 0;

    /*** STUDENT CODE ABOVE (q19) ***/

    rtnl_link_put(host_br);

    return 0;
}


/*
 * Make the network of the container on the container side.
 *
 * 1. set the IP address of the container end of the veth pair
 * 2. set up the container end of the veth pair
 *
 * Parameters:
 * * ip_addr: the IP address of the container
 *
 * Return:
 * * 0 on success
 * * -1 on error (and print an error message)
 */
int contnet_make_cont(const char* ip_addr) {
    int err = 0;
    errno = 0;

    // Netlink socket.
    struct nl_sock *nlsk = nl_socket_alloc();
    if (nlsk == NULL)
        raise_msg("failed opening Netlink socket");
    if ((err = nl_connect(nlsk, NETLINK_ROUTE)) < 0)
        raise_msg("failed connecting socket: %s", nl_geterror(err));

    // veth endpoint in the container.
    // veth_cont is the one created by the container manager, queried from the
    // kernel; veth_cont_up is modified with new flags to apply.
    struct rtnl_link *veth_cont, *veth_cont_up = rtnl_link_veth_alloc();
    if ((err = rtnl_link_get_kernel_by_name(nlsk, CONT_VETH_NAME, &veth_cont))
            < 0)
        raise_msg("failed getting container veth interface %s",
                nl_geterror(err));

    // IP address of the veth endpoint in the container (parsed from ip_addr).
    struct nl_addr *nladdr_cont;
    nl_addr_parse(ip_addr, AF_INET, &nladdr_cont);
    // IP address of the veth endpoint in the container (for the link).
    struct rtnl_addr *rtnladdr_cont = rtnl_addr_alloc();
    if (rtnladdr_cont == NULL)
        raise_msg("failed allocating veth address");

    /*** STUDENT CODE BELOW (q18) ***/

    // Set the address of rtnladdr_cont to be nladdr_cont.
    // libnl: rtnl_addr_add
    return 0;

    // Set the interface index of rtnladdr_cont to be the index of veth_cont.
    // libnl: rtnl_addr_set_ifindex, rtnl_link_get_ifindex
    return 0;

    // Add the link address rtnladdr_cont
    // libnl: rtnl_addr_add, no flag
    return 0;

    // Set veth_cont_up up.
    // libnl: rtnl_link_set_flags
    // man: netdevice(7)
    return 0;

    // Apply changes made in veth_cont_up to veth_cont
    // libnl: rtnl_link_change, no flag
    return 0;

    /*** STUDENT CODE ABOVE (q18) ***/

    nl_addr_put(nladdr_cont);
    rtnl_addr_put(rtnladdr_cont);

    rtnl_link_veth_release(veth_cont);

    nl_socket_free(nlsk);

    return 0;
}
