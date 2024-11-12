SUMMARY = "Autot image kernel module"
DESCRIPTION = "Kernel module template"
#LICENSE = "GPLv2"
LICENSE = "MIT"

inherit module

# FILESEXTRAPATHS specifies the search path(s) in meta layer
FILESEXTRAPATHS:prepend := "${THISDIR}/../../additions/prjsrc/:"

# SRC_URI specifies the final names of the file or directory to be fetched from all search paths.
# Here, "file://" is to tell Bitbake that the file or directory is local file/path.
# Bitbake will cope this file or folder to ${WORKDIR}
SRC_URI = "file://mod-hello"

S = "${WORKDIR}/mod-hello"
# path to get license file, "file://" is specified by variable "S" (${S}) 
#LIC_FILES_CHKSUM = "file://../mod-hello/LICENSE.gpl;md5=75859989545e37968a99b631ef42722e"
#LIC_FILES_CHKSUM = "file://LICENSE.gpl;md5=75859989545e37968a99b631ef42722e"
LIC_FILES_CHKSUM = "file://COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

# specify module name "mod-hello" and telll bitbake to build kernel module (with "inherit module" above)
RPROVIDES:${PN} += "kernel-module-mod-hello"

# specify macros when compiling
EXTRA_OEMAKE = "DEBUG=NO"					
# specify kernel headers path
EXTRA_OEMAKE += "KERNEL_LOCATION=${STAGING_KERNEL_DIR}"
