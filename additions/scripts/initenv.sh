#!/bin/bash

echo "export TEMPLATECONF=./sources/meta-autot/conf/templates/conf"
export TEMPLATECONF=./sources/meta-autot/conf/templates/conf
if [ $? -ne 0 ]; then
    echo "Failed to set TEMPLATECONF"
    exit 1
fi

echo "source ./sources/poky/oe-init-build-env"
source ./sources/poky/oe-init-build-env
if [ $? -ne 0 ]; then
    echo "Failed to source environment script"
    exit 1
fi
