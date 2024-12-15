SUMMARY = "Autot image libraries"
DESCRIPTION = "Libraries for software package for device PCAN-USB Pro in autotester"
LICENSE = "GPLv2"
# Here, "file://" is to tell Bitbake that the path is a local path and it refers to ${S}!
LIC_FILES_CHKSUM = "file://../pcan-8.18.0-lib/LICENSE.gpl;md5=75859989545e37968a99b631ef42722e"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*  pcanlib created by Peak System Gmbh        *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

addtask display_banner before do_build

# FILESEXTRAPATHS specifies the search path(s)
FILESEXTRAPATHS:prepend := "${THISDIR}/../../additions/prjsrc:"

# SRC_URI specifies the final names of the file or directory to be fetched from all search paths.
# Here, "file://" is to tell Bitbake that the file or directory is local file/path.
SRC_URI = "file://pcan-8.18.0-lib"

S = "${WORKDIR}/pcan-8.18.0-lib"

# copy the headers needed to sysroot before do_compile
#do_configure() {
#    install -m 0644 ${S}/driver/pcan.h ${STAGING_INCDIR}/
#    install -m 0644 ${S}/driver/pcanfd.h ${STAGING_INCDIR}/
#    install -m 0644 ${S}/lib/libpcan.h ${STAGING_INCDIR}/
#    install -m 0644 ${S}/lib/libpcanfd.h ${STAGING_INCDIR}/
#    install -m 0644 ${S}/libpcanbasic/pcanbasic/include/PCANBasic.h ${STAGING_INCDIR}/
#}
# not needed anymore when adding the directories to these header files with "-I" option to CFLAGS in assiging EXTRA_OEMAKE below

#EXTRA_OEMAKE = "CFLAGS=-I${STAGING_DIR_TARGET}/usr/include"
#EXTRA_OEMAKE += "CFLAGS= -I${S}/lib"
#EXTRA_OEMAKE = "CFLAGS='-I${STAGING_DIR_TARGET}/usr/include -I${S}/lib -I${S}/driver -DNO_RT'"
#EXTRA_OEMAKE = "--sysroot=${STAGING_DIR_TARGET} CFLAGS='-I${STAGING_DIR_TARGET}/usr/include -I${S}/lib -I${S}/driver -DNO_RT'"
#EXTRA_OEMAKE += "LDFLAGS='--sysroot=${STAGING_DIR_TARGET}/usr/lib'"
#EXTRA_OEMAKE = "LDFLAGS='--sysroot=${STAGING_DIR_TARGET}' CFLAGS='-I${STAGING_DIR_TARGET}/usr/include -I${S}/lib -I${S}/driver -I${S}/libpcanbasic/pcanbasic/include -DNO_RT'"
#EXTRA_OEMAKE += "--sysroot=${STAGING_DIR_TARGET} CFLAGS='-DNO_RT'"
#EXTRA_OEMAKE += "CFLAGS='-DNO_RT'"
#EXTRA_OEMAKE += "CFLAGS='-DNO_RT -I${S}/driver -I${S}/lib -I${S}/libpcanbasic/pcanbasic/include -I${S}/libpcanbasic/pcanbasic/src' LDFLAGS='-lm, --hash-style=gnu'"
EXTRA_OEMAKE += "CFLAGS='-DNO_RT -I${S}/driver -I${S}/lib -I${S}/libpcanbasic/pcanbasic/include -I${S}/libpcanbasic/pcanbasic/src' LDFLAGS='-lm ${LDFLAGS}'"

do_compile() {
	oe_runmake ARCH=${TARGET_ARCH} CROSS_COMPILE=${TARGET_PREFIX}
}

# do_install is to add these files to recipe's destination folder ${D}
do_install() {
    install -d ${D}${libdir}
    install -m 0644 ${S}/lib/lib/libpcan.so.6 ${D}${libdir}/
    install -m 0644 ${S}/lib/lib/libpcanfd.so.8 ${D}${libdir}/
    install -m 0644 ${S}/lib/lib/libpcanfd.a ${D}${libdir}/
    install -m 0644 ${S}/libpcanbasic/pcanbasic/lib/libpcanbasic.so.4.8.0 ${D}${libdir}/
    ln -sf libpcan.so.6 ${D}${libdir}/libpcan.so
    ln -sf libpcanfd.so.8 ${D}${libdir}/libpcanfd.so
    ln -sf libpcanbasic.so.4.8.0 ${D}${libdir}/libpcanbasic.so
    ln -sf libpcanbasic.so.4.8.0 ${D}${libdir}/libpcanbasic.so.4

    install -d ${D}${includedir}
    install -m 0644 ${S}/driver/* ${D}${includedir}/
    install -m 0644 ${S}/lib/*.h ${D}${includedir}/
    install -m 0644 ${S}/libpcanbasic/pcanbasic/include/PCANBasic.h ${D}${includedir}/

    install -d ${D}${bindir}
    install -m 0755 ${S}/libpcanbasic/pcaninfo/pcaninfo.1.3.2 ${D}${bindir}/
    ln -sf pcaninfo.1.3.2 ${D}${bindir}/pcaninfo
}

# Define the order of packages explicitly
#PACKAGES = "${PN} ${PN}-dev ${PN}-staticdev ${PN}-dbg ${PN}-doc ${PN}-locale ${PN}-src"

# add file to FILES so that they are shipped to final image rootfs
# files that are shipped to the main(runtime) package: runtime libraries
FILES:${PN} += " \
    ${libdir}/libpcan.so.6 \
    ${libdir}/libpcanfd.so.8 \
    ${libdir}/libpcanbasic.so.4.8.0 \
    ${bindir}/* \
"
# files that are shipped to the development package: development libraries, headers, symlinks
FILES:${PN}-dev += " \
    ${includedir}/*.h \
    ${libdir}/libpcan.so \
    ${libdir}/libpcanfd.so \
    ${libdir}/libpcanbasic.so \
    ${libdir}/libpcanbasic.so.4 \
"
# files that are shipped to the static development package: static development libraries
FILES:${PN}-staticdev += " \
    ${libdir}/libpcanfd.a \
"
# add this in image recipe: IMAGE_INSTALL += " pcanlib pcanlib-dev"
# Static libraries (staticdev package, here is pcanlib-staticdev) are not added to IMAGE_INSTALL and not encouraged to be installed to rootfs. Yocto automatically don't package them. But they will be copied to a recipe's recipe-sysroot if that recipe depends on pcanlib (DEPENDS).

# Ensure `-dev` depends on the runtime package
RDEPENDS:${PN}-dev += "${PN}"
