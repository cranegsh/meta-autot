SUMMARY = "Raspberry Pi arguments to Linux kernel"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

do_install:append () {
	sed -i '$ s/$/ console=serial0,115200/' ${WORKDIR}/cmdline.txt
}
