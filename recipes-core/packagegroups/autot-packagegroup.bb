DESCRIPTION = "Custom package group for autot image"
SUMMARY = "additional packagegroups"
LICENSE = "CLOSED"

inherit packagegroup

RDEPENDS:${PN} += " \
    systemd-boot \
    systemd-analyze \
"
