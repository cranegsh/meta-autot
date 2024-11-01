SUMMARY = "Automation Test"
LICENSE = "CLOSED"

inherit core-image
require recipes-core/images/core-image-base.bb

# add users with password
include files/autot-users.inc

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"
SRC_URI:append = " \
    file://autot-banner.txt \
"

# add files to rootfs or modify files in rootfs
do_update_rootfs() {
#    install -m 0644 ${THISDIR}/files/10-wired-static.network ${IMAGE_ROOTFS}/etc/systemd/network/
#
# add paths for user crane in /etc/profile
    sed '/^PATH=/a [ "$HOME" != "/home/crane" ] || PATH=$PATH:/usr/local/sbin:/usr/sbin:/sbin' -i ${IMAGE_ROOTFS}/etc/profile
    sed '/^PATH=/a #add /sbin to $PATH for the user crane' -i ${IMAGE_ROOTFS}/etc/profile
#
# add user crane and autot to /etc/sudoers
    echo "crane ALL=(ALL:ALL) ALL" >> ${IMAGE_ROOTFS}/etc/sudoers
    echo "autot ALL=(ALL:ALL) ALL" >> ${IMAGE_ROOTFS}/etc/sudoers
#
# copy banner text to /etc/issue
    cp -f ${THISDIR}/files/autot-banner.txt ${WORKDIR}/
    cat ${WORKDIR}/autot-banner.txt > ${IMAGE_ROOTFS}/etc/issue
#
# specify shell prompt format
    echo 'export PS1="\u@\h:\w\$ "' >> ${IMAGE_ROOTFS}/home/crane/.bashrc
    echo 'export PS1="\[\e[32m\]\u@\h\[\e[m\]:\[\e[34m\]\w\[\e[m\]\$ "' >> ${IMAGE_ROOTFS}/etc/profile
    install -d ${IMAGE_ROOTFS}/etc/skel
    echo 'export PS1="\[\e[32m\]\u@\h\[\e[m\]:\[\e[34m\]\w\[\e[m\]\$ "' >> ${IMAGE_ROOTFS}/etc/skel/.bashrc
    install -m 644 ${IMAGE_ROOTFS}/etc/skel/.bashrc ${IMAGE_ROOTFS}/home/autot/.bashrc
}
ROOTFS_POSTPROCESS_COMMAND:append = " do_update_rootfs"
# Below is to allow running "bitbake autot-image-base -c do_update_rootfs" without error
addtask do_update_rootfs after do_rootfs before do_image

# add custom recipes
IMAGE_INSTALL:append = " systemd-conf autot-service"

# add services
IMAGE_INSTALL:append = " openssh"

# add commands
IMAGE_INSTALL:append = " sudo"
