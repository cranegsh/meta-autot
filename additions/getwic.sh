#!/bin/bash

cd ~/work/yocto/build/tmp/deploy/images/raspberrypi5/

SYMLINK_NAME="autot-image-base-raspberrypi5.rootfs.wic.bz2"
TARGET_FILE=$(readlink "$SYMLINK_NAME")

echo "target file: $SYMLINK_NAME"
echo "Resolved target file: $TARGET_FILE"

if [ -f "$TARGET_FILE" ]; then
    echo "Unzipping file: $TARGET_FILE"

    case "$TARGET_FILE" in
        *.zip)
            unzip "$TARGET_FILE"
            ;;
        *.tar.gz)
            tar -xzf "$TARGET_FILE"
            ;;
        *.tar.bz2)
            tar -xjf "$TARGET_FILE"
            ;;
	*.wic.bz2)
#	    bzip2 -dk "$TARGET_FILE"
	    cp "$TARGET_FILE" autot-image-base.wic.bz2
	    bzip2 -d autot-image-base.wic.bz2
	    ;;
        *)
            echo "Unsupported file type: $TARGET_FILE"
            ;;
    esac
else
    echo "Target file does not exist or symbolic link is broken."
    exit 1
fi

cd ~/work/yocto/build
