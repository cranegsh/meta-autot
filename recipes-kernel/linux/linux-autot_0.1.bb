require ../../meta-raspberrypi/recipes-kernel/linux/linux-raspberrypi_6.1.bb

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://defconfig"
