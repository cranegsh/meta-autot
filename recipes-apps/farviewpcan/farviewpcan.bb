SUMMARY = "Autot image main application"
DESCRIPTION = "Main application for Autotester with 64-bit libraries built with cross SDK separately"
LICENSE = "CLOSED"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*  farviewpcan application with libraries     *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

addtask display_banner before do_build

FILESEXTRAPATHS:prepend := "${THISDIR}/../../additions/prjdeploy/:"

SRC_URI = "file://farviewpcan"

S = "${WORKDIR}/farviewpcan"

# do_install is to add these files to recipe's destination folder ${D}
do_install() {
    install -d ${D}${libdir}
    install -m 0644 ${S}/lib/* ${D}${libdir}/
    ln -sf libpcan.so.6 ${D}${libdir}/libpcan.so
    ln -sf libpcanfd.so.8 ${D}${libdir}/libpcanfd.so
    ln -sf libpcanbasic.so.4.8.0 ${D}${libdir}/libpcanbasic.so
    ln -sf libpcanbasic.so.4.8.0 ${D}${libdir}/libpcanbasic.so.4

    install -d ${D}${includedir}
    install -m 0644 ${S}/include/* ${D}${includedir}/

    install -d ${D}${bindir}
    install -m 0755 ${S}/tools/pcaninfo.1.3.2 ${D}${bindir}/
    ln -sf pcaninfo.1.3.2 ${D}${bindir}/pcaninfo
#    install -m 0755 ${S}/farviewpcan ${D}${bindir}/
#    install -m 0755 ${S}/pcantest.sh ${D}${bindir}/
}

# Define the order of packages explicitly
PACKAGES = "${PN} ${PN}-dev ${PN}-staticdev ${PN}-dbg ${PN}-doc ${PN}-locale ${PN}-src"

# add file to FILES so that they are shipped to final image rootfs
# files that are shipped to the main(runtime) package: runtime libraries
FILES:${PN} += " \
    ${libdir}/libpcan.so.6 \
    ${libdir}/libpcan.test \
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
# add this in image recipe: IMAGE_INSTALL += " farviewpcan farviewpcan-dev farviewpcan-staticdev"

# Ensure `-dev` depends on the runtime package
RDEPENDS:${PN}-dev += "${PN}"

# Exclude static library from the `-dev` package
#EXCLUDE_FROM_DEV += "${libdir}/libpcanfd.a"

#INSANE_SKIP:${PN} += "installed-vs-shipped"		# skip QA for debug purpose

do_install:append() {
    # prevent stripping the libraries during installation	# not working
    do_strip() { :; }
}
