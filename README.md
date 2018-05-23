# XC8-CDThread

This is a cooperative scheduler with messages and events capability that is compiled with Microchip XC8 C compiler (C for PIC microcontroller). I had created it to solve a simple problem, it has a system that allowed me to manage code as is available a kernel but in a system with poor resource as a PIC16/PIC18 microcontrollers families with only 16KB rom and 256 bytes. 
I also have created an empowered 32bit version of this scheduler for Analog devices BLACKFIN DSP that I use normally inside a single VDK microkernel thread to manage with semplicity complex works avoiding to make VDK heavy and slow in responce.

One of the key of this scheduler is that is statically configurable (no dynamic memory allocation) in the number of CDthreads CDEvents and CDmessages, so directly at compile time is possible evaluate the memory footprint. Also the scheduler is very simple and could be recoded to solve different application constrain.

At today I have used it in many projects but there is a some problems with documentation that is not updated and present in a separated document. In the fact the only documentation is in the initial part of the header, and some source, file. Please se the cdthread.h to understand how the code work

...good work
