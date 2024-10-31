FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://10-wired-static.network \
    file://20-wireless-dynamic.network \
    file://wpa_supplicant_wifi.conf \
"

FILES:${PN} += " \
    ${sysconfdir}/systemd/network/10-wired-static.network \
    ${sysconfdir}/systemd/network/20-wireless-dynamic.network \
    ${sysconfdir}/wpa_supplicant/wpa_supplicant_wifi.conf \
"

# add configuraton files to rootfs
do_install:append() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/10-wired-static.network ${D}${sysconfdir}/systemd/network/
    install -m 0644 ${WORKDIR}/20-wireless-dynamic.network ${D}${sysconfdir}/systemd/network/
    install -d ${D}${sysconfdir}/wpa_supplicant
    install -m 0644 ${WORKDIR}/wpa_supplicant_wifi.conf ${D}${sysconfdir}/wpa_supplicant/
}
