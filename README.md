ipu-examples
============

IPU example codes for i.MX5 and i.MX6 families

Cross-compiling:
================

To crosscompile the ipu-examples, set the environment variables:

$ export PATH=$PATH:/opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/bin/  
$ export TOOLCHAIN=/opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/  
$ export CROSS_COMPILE=arm-none-linux-gnueabi-

Notes:  
* Usually the TOOLCHAIN is loated at /opt/freescale/usr/local/gcc-4.6.2-glibc-2.13-linaro-multilib-2011.12/fsl-linaro-toolchain/  
it may vary according to the tool you have instaled on your machine  

Now, use configure and make to build.  
  
$ ./configure --host=arm-none-linux-gnueabi --target=<target> --prefix=<prefix>  

Where:  
<target>: mx5 or mx6  
<prefix>: local where your i.MX rootfs is  

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
resizing        : Resize examples  
rotation        : Rotation examples  

ADITIONAL COMMENTS:
===================

Not all examples were implemented so far.  
If you have questions or comments, please contact me on the e-mail below.  

AUTHOR:
=======

Rogerio Pimentel  
rpimentel.silva@gmail.com  

