ipu-examples
============

IPU example codes for i.MX5 and i.MX6 families

Cross-compiling:
================

To crosscompile the ipu-examples, set the environment variables:

$ export TOOLCHAIN=/opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi  
$ export ROOTFS=your_rootfs_folder  

Notes:  
* your_rootfs_folder is the locations where your i.MX root file system is  
* Usually the TOOLCHAIN is loated at /opt/freescale/usr/local/gcc-4.4.4-glibc-2.11.1-multilib-1.0/arm-fsl-linux-gnueabi  
it may vary according to the tool you have instaled on your machine  

Now, just use make to build.

$ make

Available examples:
===================

The project has the following folders:

bin             : Generated binaries  
images          : Pictures used by examples  
mx5             : i.MX5 family examples  
mx6             : i.MX6 family examples  

Inside mx5 and mx6 folders, there are the following folders:

alphablending   : Alpha blending usage examples  
basic           : Basic examples like turn on/off cursor, fill framebuffer and etc  
combining       : Frame combining examples  
cropping        : Crop image examples  
csc             : Color Space Conversion examples  
deinterlacing   : Video de-interlace examples  
doc             : Documentation. Details about IPU and code implementation  
others          : Mix of various common use cases  
panning         : Panning images examples  
resizing        : Resize examples  
rotation        : Rotation examples  

ADITIONAL COMMENTS:
===================

Not all examples were implemented so far.  
If you have questions or comments, please contact me on the e-mail below.  

AUTHOR:
=======

Rogerio Pimentel  
rogerio.pimentel@freescale.com  

