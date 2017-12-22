# msrtools
Two code set for reading performance counters

The first code was written based on the msr tools and I have only modified the rdmsr.c code to get the frequency and DRAM, and CPU energy consumptions. This driver works perfectly on all Intel processors. However, if you need to read the processor performance counters then it should be changed and wrmsr be used too. (it is not a big deal, but put it simple I didn't need it)

The second code set was adopted from http://www.mindfruit.co.uk/ (Michael! He is a nice guy, I talked to him on google hangouts and he was very open to discuss and explain things). This one was written as a driver. First it makes an ioctl interface with OS kernel to be able to use rdmsr and wrmsr priviledged instructions. Then, a user-space simple code does the rest of the trick. It is well-written. However, the last time I tried to use it on our new Intel processors it did not work out. It should be easy to resolve but I didn't spend time on it (maybe it is working now?)

If you use these tools, please cite my paper "VR-scale: runtime dynamic phase scaling of processor voltage regulators for improving power efficiency"


