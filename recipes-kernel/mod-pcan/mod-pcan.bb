SUMMARY = "Autot image kernel module"
DESCRIPTION = "Driver module for PCAN-USB Pro"
LICENSE = "GPLv2"
# Here, "file://" is to tell Bitbake that the path is a local path and it refers to ${S}!
LIC_FILES_CHKSUM = "file://../pcan-8.18.0-driver/LICENSE.gpl;md5=75859989545e37968a99b631ef42722e"

inherit module

#DEPENDS = "virtual/kernel"

# FILESEXTRAPATHS specifies the search path(s)
FILESEXTRAPATHS:prepend := "${THISDIR}/../../additions/prjsrc:"

# SRC_URI specifies the final names of the file or directory to be fetched from all search paths.
# Here, "file://" is to tell Bitbake that the file or directory is local file/path.
SRC_URI = "file://pcan-8.18.0-driver"

S = "${WORKDIR}/pcan-8.18.0-driver"

# This is to specify where to find Modules.symvers in ${B}
MODULES_MODULE_SYMVERS_LOCATION = "/driver"

RPROVIDES:${PN} += "kernel-module-pcan"

EXTRA_OEMAKE = "PCC=NO PCI=NO ISA=NO DNG=NO"
#EXTRA_OEMAKE += "SRC_DIR=${WORKDIR} DEST_DIR=${D}"
EXTRA_OEMAKE += "KERNEL_LOCATION=${STAGING_KERNEL_DIR}"

do_install:append() {
    install -d ${D}${sysconfdir}/udev/rules.d
    install -m 0644 ${S}/driver/udev/pcanusb.rules ${D}${sysconfdir}/udev/rules.d/
}
FILES:${PN} = "${sysconfdir}/udev/rules.d/pcanusb.rules"
