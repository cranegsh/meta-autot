require ../../meta-raspberrypi/recipes-kernel/linux/linux-raspberrypi_6.1.bb

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

#SRC_URI += "file://defconfig"
SRC_URI += "file://bcm2709_defconfig"

KERNEL_MODULES = "1"

KERNEL_DEVICETREE := "${@d.get('KERNEL_DEVICETREE', '').replace('broadcom/', '')}"
