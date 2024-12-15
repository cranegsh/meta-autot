SUMMARY = "Raspberry Pi config hardware"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

#SRC_URI += "file://0001-enable-uart-console-at-pin8-10.patch"
#SRC_URI += "file://0001-enable-uart-console-at-pin26-27.patch"

do_install:append () {
         echo "enable_uart=1" >> ${WORKDIR}/git/config.txt
         echo "dtoverlay=uart0" >> ${WORKDIR}/git/config.txt
}

do_package:append () {
#        echo "dtparam=uart0=on" >> ${WORKDIR}/config.txt
#        echo "enable_uart=1" >> ${WORKDIR}/git/config.txt
#        echo "dtoverlay=uart3" >> ${WORKDIR}/git/config.txt
}

