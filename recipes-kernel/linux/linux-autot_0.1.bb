require ../../meta-raspberrypi/recipes-kernel/linux/linux-raspberrypi_6.1.bb

FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

#SRC_URI += "file://defconfig"
SRC_URI += "file://bcm2709_defconfig"

#SRC_URI += "file://autot-defconfig"
#KERNEL_DEFCONFIG = "autot-defconfig"
# adding autot-defconfig is an example to change the original bcm2709_defconfig a little
# verified by checking .config in ${B} folder after running "bitbake -c menuconfig linux-autot"

KERNEL_MODULES = "1"

KERNEL_DEVICETREE := "${@d.get('KERNEL_DEVICETREE', '').replace('broadcom/', '')}"
