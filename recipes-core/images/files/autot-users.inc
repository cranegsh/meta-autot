# add extra user crane and autot

EXTRA_IMAGE_FEATURES:remove = "debug-tweaks"

inherit extrausers

EXTRA_USERS_PARAMS += " \
    usermod -p '$(openssl passwd -6 root)' root; \
    useradd -p '$(openssl passwd -6 2024)' crane; \
    useradd -p '$(openssl passwd -6 2024)' autot; \
"
