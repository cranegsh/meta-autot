SUMMARY = "Hello worls application"
DESCRIPTION = "Simplest application as a template"
LICENSE = "MIT"
# path to get license file, "file://" is specified by variable "S" (${S}) 
LIC_FILES_CHKSUM = "file://COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

# FILESEXTRAPATHS specifies the search path(s) in meta layer
FILESEXTRAPATHS:prepend := "${THISDIR}/../../additions/prjsrc/:"

# SRC_URI specifies the final names of the file or directory to be fetched from all search paths.
# Here, "file://" is to tell Bitbake that the file or directory is local file/path.
# Bitbake will cope this file or folder to ${WORKDIR}
SRC_URI = "file://hello"

S = "${WORKDIR}/hello"

# Build source file directly
do_compile() {
    ${CC} hello.c ${LDFLAGS} -o hello
}
# run Makefile in source files
# but later in do_install cause QA error "doesn't have GNU_HASH"
#do_compile() {
#    oe_runmake
#}

do_install() {
    install -d ${D}${bindir}
    install -m 0755 hello ${D}${bindir}/
}
