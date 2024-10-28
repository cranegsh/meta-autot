SUMMARY = "Automation Test"
LICENSE = "CLOSED"

inherit core-image
require recipes-core/images/core-image-base.bb

# add users with password
include files/autot-users.inc

# add configuraton files to rootfs or modify configuration files in rootfs
update_rootfs() {
#    install -m 0644 ${THISDIR}/files/10-wired-static.network ${IMAGE_ROOTFS}/etc/systemd/network/
    sed '/^PATH=/a [ "$HOME" != "/home/crane" ] || PATH=$PATH:/usr/local/sbin:/usr/sbin:/sbin' -i ${IMAGE_ROOTFS}/etc/profile
    sed '/^PATH=/a #add /sbin to $PATH for the user crane' -i ${IMAGE_ROOTFS}/etc/profile
}
ROOTFS_POSTPROCESS_COMMAND:append = " update_rootfs"

# add a custom recipe
IMAGE_INSTALL:append = " systemd-conf"

# add a service
IMAGE_INSTALL:append = " openssh"

# add a command
IMAGE_INSTALL:append = " sudo"
