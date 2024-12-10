SUMMARY = "Autot image main application"
DESCRIPTION = "Main application for Autotester"
LICENSE = "CLOSED"

python do_display_banner() {
    bb.plain("***********************************************");
    bb.plain("*                                             *");
    bb.plain("*  Autotester main application                *");
    bb.plain("*                                             *");
    bb.plain("***********************************************");
}

addtask display_banner before do_build

# FILESEXTRAPATHS specifies the search path(s) in meta layer
FILESEXTRAPATHS:prepend := "${THISDIR}/../../additions/prjsrc/:"
# SRC_URI specifies the final names of the file or directory to be fetched from all search paths.
# Here, "file://" is to tell Bitbake that the file or directory is local file/path.
# Bitbake will cope this file or folder to ${WORKDIR}
SRC_URI = "file://autotester-0.1y"
S = "${WORKDIR}/autotester-0.1y"

# Specify the remote repository and branch/tag
#SRC_URI = "git@github.com:cranegsh/autotester.git;branch=develop-yocto"
#SRCREV = "3e7b329983bf94a2060a2b0aa5e8140ef79d4530"
# Define the name of the source directory after fetching
#S = "${WORKDIR}/git"

# Inherit standard classes
#inherit cmake # or autotools or meson, depending on your build system

# Specify any dependencies required to build the project
#DEPENDS = "libtool gcc g++"
DEPENDS = "farviewpcan"
# The libraries needed are from recipe farviewpcan.bb

# Optional: Custom configuration for the build system (if using cmake)
#EXTRA_OECMAKE = "-DCMAKE_BUILD_TYPE=Release"
EXTRA_OEMAKE += "LDFLAGS='-lpcanbasic  -Wl,--hash-style=gnu' CFLAGS='-I${S}/include -D__ARM_PCS_VFP'"

# Optional: Override tasks for custom build steps
#do_configure[depends] += "mydependency-native:do_populate_sysroot"
# copy the library needed to sysroot before do_compile
#do_configure() {
#    install -m 0644 ${S}/lib/libpcanbasic.so.4.8.0 ${STAGING_LIBDIR}/
#}

do_compile() {
#    oe_runmake CC="${CC}" CFLAGS="${CFLAGS}" LDFLAGS="${LDFLAGS}"
    oe_runmake
}

# Define installation steps
do_install() {
    # Example: Installing binaries and other files
    install -d ${D}${bindir}
    install -m 0755 farview ${D}${bindir}/farview
    install -m 0755 test.sh ${D}${bindir}/test.sh
}

# Specify files to include in the package
FILES:${PN} = "${bindir}/*"
