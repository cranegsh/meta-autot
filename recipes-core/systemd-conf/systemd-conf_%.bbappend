FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += " \
    file://10-wired-static.network \
"

FILES:${PN} += " \
    ${sysconfdir}/systemd/network/10-wired-static.network \
"

# add configuraton files to rootfs
do_install:append() {
    install -d ${D}${sysconfdir}/systemd/network
    install -m 0644 ${WORKDIR}/10-wired-static.network ${D}${sysconfdir}/systemd/network/
}
