require ../../meta-raspberrypi/recipes-kernel/linux/linux-raspberrypi_5.15.bb

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

#SRC_URI += "file://defconfig"
SRC_URI += "file://bcm2709_defconfig"

KERNEL_MODULES = "1"

#KERNEL_DEVICETREE := "${@d.get('KERNEL_DEVICETREE', '').replace('broadcom/', '')}"
# The above assignment is not needed for "kirkstone" meta-raspberrypi
#SDIMG_KERNELIMAGE ?= kernel7.img
#SDIMG_KERNELIMAGE := kernel7.img
#SDIMG_KERNELIMAGE ?= "kernel7.img"
# Need to have the quotation mark on the value at the right or the assignment equation.
# But it is not good to be defined here as the decision should be left to higher level configuration
