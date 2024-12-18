# !/bin/bash
# This is to fix the kernel module build system (kmbs)

sudo cp -r kmbs-patch-6.1.77-rpi5/arch-arm64-include-generated/ /media/crane/root/lib/modules/6.1.77-v8-16k/build/arch/arm64/include/generated
sudo cp -r kmbs-patch-6.1.77-rpi5/include-generated/ /media/crane/root/lib/modules/6.1.77-v8-16k/build/include/generated
sudo cp -r kmbs-patch-6.1.77-rpi5/include-uapi-asm_generic-bitsperlong.h /media/crane/root/lib/modules/6.1.77-v8-16k/build/include/uapi/asm-generic
sudo cp kmbs-patch-6.1.77-rpi5/scripts-basic-fixdep /media/crane/root/lib/modules/6.1.77-v8-16k/build/scripts/basic/fixdep
sudo cp kmbs-patch-6.1.77-rpi5/scripts-mod-modpost /media/crane/root/lib/modules/6.1.77-v8-16k/build/scripts/mod/modpost
sudo cp kmbs-patch-6.1.77-rpi5/scripts-recordmcount /media/crane/root/lib/modules/6.1.77-v8-16k/build/scripts/recordmcount
sudo ln -s /media/crane/root/lib/modules/6.1.77-v7/build/include/asm-generic/ /media/crane/root/lib/modules/6.1.77-v8-16k/build/include/asm
sudo ln -s /media/crane/root/lib/modules/6.1.77-v7/build/include/uapi/asm-generic/ /media/crane/root/lib/modules/6.1.77-v8-16k/build/include/uapi/asm
