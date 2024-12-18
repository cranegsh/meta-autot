# !/bin/bash
# fix the kernel module build system on kernel v5.15 running on Raspberry pi 2B

sudo cp -r kmbs-patch_5.15/arch-arm-include-generated/asm/* /media/crane/root/lib/modules/5.15.34-v7/build/arch/arm/include/asm
sudo cp -r kmbs-patch_5.15/arch-arm-include-generated/uapi/asm/* /media/crane/root/lib/modules/5.15.34-v7/build/arch/arm/include/uapi/asm
sudo cp -r kmbs-patch_5.15/include-generated/ /media/crane/root/lib/modules/5.15.34-v7/build/include/generated
sudo cp kmbs-patch_5.15/scripts-basic-fixdep /media/crane/root/lib/modules/5.15.34-v7/build/scripts/basic/fixdep
sudo cp kmbs-patch_5.15/scripts-mod-modpost /media/crane/root/lib/modules/5.15.34-v7/build/scripts/mod/modpost
sudo cp kmbs-patch_5.15/scripts-recordmcount /media/crane/root/lib/modules/5.15.34-v7/build/scripts/recordmcount
sudo ln -s /media/crane/root/lib/modules/5.15.34-v7/build/include/asm-generic/ /media/crane/root/lib/modules/5.15.34-v7/build/include/asm
sudo ln -s /media/crane/root/lib/modules/5.15.34-v7/build/include/uapi/asm-generic/ /media/crane/root/lib/modules/5.15.34-v7/build/include/uapi/asm
