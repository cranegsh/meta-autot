SUMMARY = "Autot development tools package"
DESCRIPTION = "A package containing essential development tools including kernel headers, gcc, etc."
LICENSE = "CLOSED"

# Without below line, bitbake "Unable to fine a match: autot-devtools" when building the image
# while it works fine to build autot-devtools separately
inherit packagegroup
# Below line is to override PACKAGE_ARCH whose value is overridden by "inherit packagegroup" to "all"
# But it doesn't work to change it here. Effective putting in conf/local.conf, but can't!
PACKAGE_ARCH = "${MACHINE_ARCH}"

# for kernel moduel development
RDEPENDS:${PN} = " \
    kernel-dev \
    kernel-devsrc \
    kernel-modules \
"

# User app development tools
RDEPENDS:${PN} += " \
    gdb \
"
# When using tools-sdk feature, all below and some other tools are included
#RDEPENDS:${PN} += " \
#    gcc \
#    g++ \
#    make \
#"
# It looks that it doesn't work to add the image feature here
#EXTRA_IMAGE_FEATURES += " tools-sdk"

# Add specific library for development
# If using libpopt-dev, seeing error: Nothing RPROVIDES 'libpopt-dev'
# If using popt-dev, seeing error: do_package_write_ipk: An allarch packageroup shouldn't depend on packages which are dynamically renamed (popt-dev to libpopt-dev)
# Adding PACKAGE_ARCH assignment in the recipe, it doesn't work to override its value
#RDEPENDS:${PN} += " \
#    libpopt-dev \
#"
# If adding dev-pkgs to IMAGE_FEATURES, all libraries for development will be installed
#EXTRA_IMAGE_FEATURES += " dev-pkgs"
