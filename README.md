# Analysis and Optimization of Dynamic Voltage and Frequency Scaling for AVX Workloads Using a Software-Based Reimplementation
This repository contains the LaTeX sources of my bachelor's thesis (conducted at KIT between 05/03/2019 and 09/02/2019), 
all source code written for the research done in the thesis, and CSV sheets with the raw results.

## Abstract
While using the Advanced Vector Extensions (AVX) on current Intel x86 
processors allows for great performance improvements in programs that can be
parallelized by using vectorization, many heterogeneous workloads that use
both vector and scalar instructions expose degraded throughput when making
use of AVX2 or AVX-512. This effect is caused by processor frequency
reductions that are required to maintain system stability while executing AVX
code. Due to the delays incurred by frequency switches, reduced clock speeds
are retained for some additional time after the last demanding instruction has
retired, causing code in scalar phases directly following AVX phases to be
executed at a slower rate than theoretically possible.

We present an analysis of the precise frequency switching behavior of an
Intel Syklake (Server) CPU when AVX instructions are used. Based on the
obtained results, we propose AVXFreq, a software reimplementation of the AVX
frequency selection mechanism. AVXFreq is designed to enable us to devise and
evaluate alternative algorithms that govern the processor’s frequency smarter
with regard to workloads that mix both AVX and scalar instructions. Using
an oracle mechanism built atop AVXFreq, we show that the performance of
scalar phases in heterogeneous workloads can theoretically be improved by up
to 15 % with more intelligent reclocking mechanisms.

## Build & Run
**Note**: All code was specifically written to run on an Intel Core i9-7940X. Both AVXFreq and the analysis framework are 
theoretically capable of running on other *Skylake (Server)* processors as well, however, minor modifications will be 
necessary.

### AVXFreq
* Grab a fresh copy of the Linux 5.1.0 source code
* Apply `avxfreq.diff`
* `make && sudo make modules_install install`
* Ensure the following settings are configured in your motherboard's UEFI:
  * SpeedShift (HWP/CPPC): Off
  * SpeedStep (EIST): On
  * Turbo Boost: On
  * Turbo License Level Offsets 1 and 2 both set to 1
* Boot your kernel with `intel_pstate=avxfreq`

### Reclocking Analysis Tool
* Build and run a kernel with AVXFreq and `reclocking-analysis/entry.diff` applied
* Build the kernel module:
  * `cd reclocking-analysis/kernel`
  * `make -C <kernel source path> M=$(pwd) && sudo make -C <kernel source path> M=$(pwd) modules_install`
* Build the user-space component:
  * `cd reclocking-analysis/user`
  * `make`
* `sudo php reclocking-analysis/user/run-all.php` to run all tests (this requires PHP 7.2)

The analysis framework will automatically detect whether the system is running with HWP or AVXFreq and adapt accordingly.

## License
You may redistribute, present, build, and run everything in this repository, as long as you keep/show a clear notice of authorship.
All modifications and other uses require explicit written permission by me.

Execute this software at your own risk. I am not responsible for any damage done to your or anyone's computer.
