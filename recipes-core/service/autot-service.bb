DESCRIPTION = "Autot custom services"
#LICENSE = "CLOSED"
#LIC_FILES_CHKSUM = "file://${COREBASE}/../meta-autot/license/CLOSED;md5=<checksum>"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${WORKDIR}/COPYING.MIT;md5=3da9cfbcb788c80a0384361b4de20420"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

inherit systemd
PACKAGE_ARCH = "${MACHINE_ARCH}"

SRC_URI = " \
    file://COPYING.MIT \
    file://autot-script.sh \
"

FILES:${PN} += " \
    /usr/bin/autot-script.sh \
"
#    /usr/lib/systemd/system/disable-wifi-powersave.service
#    /usr/lib/systemd/system/wifi-autoconnect.service

do_install() {
#    install -d ${D}${systemd_system_unitdir}
#    install -m 0644 ${WORKDIR}/disable-wifi-powersave.service ${D}${systemd_system_unitdir}
#    install -m 0644 ${WORKDIR}/wifi-autoconnect.service ${D}${systemd_system_unitdir}

#    install -d ${D}${sysconfdir}/systemd/system/
#    install -m 0644 ${WORKDIR}/disable-wifi-powersave.service ${D}${sysconfdir}/systemd/system
#    install -m 0644 ${WORKDIR}/wifi-autoconnect.service ${D}${sysconfdir}/systemd/system

    install -d ${D}/usr/bin
    install -m 0755 ${WORKDIR}/autot-script.sh ${D}/usr/bin/
}

# Enable the service to start on boot
#SYSTEMD_SERVICE:{PN} = "disable-wifi-powersave.service wifi-autoconnect.service"
#SYSTEMD_AUTO_ENABLE:{PN} = "enable" 
#SYSTEMD_SERVICE:${PN} = "disable-wifi-powersave.service wifi-autoconnect.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable" 
