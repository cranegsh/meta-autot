SUMMARY = "Raspberry Pi config hardware"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://0001-enable-uart0.patch"

do_install:append () {
#        echo "dtparam=uart0=on" >> ${WORKDIR}/config.txt
	touch ${WORKDIR}/test.txt
	echo "dtparam=uart0=on" >> test.txt
}

