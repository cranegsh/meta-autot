# !/bin/bash
# This is to fix the kernel module build system (kmbs)

sudo cp -r kmbs-patch-6.1.77/arch-arm-include-generated/ /media/crane/root/lib/modules/6.1.77-v7/build/arch/arm/include/generated
sudo cp -r kmbs-patch-6.1.77/include-generated/ /media/crane/root/lib/modules/6.1.77-v7/build/include/generated
sudo cp kmbs-patch-6.1.77/scripts-basic-fixdep /media/crane/root/lib/modules/6.1.77-v7/build/scripts/basic/fixdep
sudo cp kmbs-patch-6.1.77/scripts-mod-modpost /media/crane/root/lib/modules/6.1.77-v7/build/scripts/mod/modpost
sudo cp kmbs-patch-6.1.77/scripts-recordmcount /media/crane/root/lib/modules/6.1.77-v7/build/scripts/recordmcount
sudo ln -s /media/crane/root/lib/modules/6.1.77-v7/build/include/asm-generic/ /media/crane/root/lib/modules/6.1.77-v7/build/include/asm
sudo ln -s /media/crane/root/lib/modules/6.1.77-v7/build/include/uapi/asm-generic/ /media/crane/root/lib/modules/6.1.77-v7/build/include/uapi/asm
sudo ln -s /media/crane/root/lib/modules/6.1.77-v7/build/arch/arm/include/uapi/asm-generic/ /media/crane/root/lib/modules/6.1.77-v7/build/arch/arm/include/uapi/asm
sudo ln -s /media/crane/root/lib/modules/6.1.77-v7/build/arch/arm/include/asm-generic/ /media/crane/root/lib/modules/6.1.77-v7/build/arch/arm/include/asm
