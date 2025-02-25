v0.2.2
======
* Added a hacked version for overridden malloc/free functions, enabling libcef (and derivative) to work.
    => Steam Large mode and new BigPicture are working
    => Heroic Launcher is working
* Added rcfile handling: box64 now read `/etc/box64.box64rc` and `~/.box64rc` and apply per process parameters
    => Every program can be finetuned, for speed or just to make them work
* Added BOX64_DYNAREC_CALLRET to optimized CALL/RET opcode using semi-direct native CALL/RET (get more than 10% speedup)
* Added BOX64_DYNAREC_FORWARD to allow bigger Dynarec block to be build (can get more then 30% of Speedup!)
* Improved memory protection tracking and signal handling, again
* Box64 now return a custom name for the CPU instal of P4, build with Box64 and the cpu name it's running on
* Simplified mutex handling (faster, cleaner)
* Reduce memory used by the memory tracking (memory almost divide by 4 is some critical use case)
* Added d3dadapter9 wrapping
    => gallium nine is now supported on box64 too
* More wrapped libs (especially on the gtk familly)
* Improved/fixed some Syscall handling
* Refactored Strong Memory Model emulation (faster and more accurate)
* Added some more opcode to ARM64 Dynarec, and fixed some other
* Some fixes to flags handling in the Dynarec on some special cases
* Refactor library loading/unloading, with refcount.
* Some special cases fixes for dlopen (fixes deadcells, maybe others too)
* Improved mmap mapping, espcially with MAP_32BITS flag
* More wrapped functions and syscalls
* Lots of improvment in the Rounding of float & double (Dynarec and Interpreter), and proper handling of denormals when converting to/from 80 bits long double
* Added specific suppport for RK3588 board (needed some hack in signal handling, probably due to the use of a non mainlined kernel)
* More support on the RV64 port
* More support to build under musl environment

v0.2.0
======
* Wrapped more vulkan function, dxvk 2.0 now works
* Added support for overridden malloc/free functions
* Refactor Dynarec memory management, reducing memory footprint (from 20% to 40% of Dynarec size)
* Improved elf loader
* Improved dlsym handling with RTLD_NEXT
* Added BOX64_DYNAREC_SAFEFLAGS to allow finetuning Dynarec generated code handling of flags accross function calls and returns.
* Added BOX64_BASH to setup x86_64 bash binary, so shell script can be run in x86_64 world 
* Added BOX64_ROLLING_LOG to have details log only on crash
* Work on alignment for some of pthread structures
* More wrapped libraries in the gtk family
* Some fixes on the Dynarec x87 code (fixing a bunch of games like ut2004 or Piczle Puzzle)
* Improved TLS Size handling (Unity3D games now works)
* More PageSize Compile options
* Improved `execv` family of function to have better box64 integration
* Added support for wrapped libfuse so AppImage works
* Adjusted the binfmt integration so AppImage are automaticaly picked
* Improved FAudio, SDL2, GL (and more) wrapping
* More wrapped libs, like libEGL.so, to helps emulated Qt4/5 libs to load
* Improved Signal Handling and memory protection traking
* Some signature fixes to some wrapped functions (in libc)
* Added more syscalls
* Added a lot of new opcodes to the ARM64 Dynarec
=> Lot's of new games stat working, to name a few: Surviving Mars, Piczle Cross Adventure (using Proton experimental), UT2004, Obduction, Machinarium, Silicon Zero, Hacknet, Absolute Drift, Art of Rally, Dota2
=> Some apps also runs now, like TeamSpeak 3 Client and Server or Discord
=> The bash integration is usefull for device with no binfmt integration or for script that check the machine it's running on

v0.1.8
======
* Added an option to get generated -NAN on SSE2 operation
* Many small fixes and improvement to get SteamPlay (i.e. proton) working (along with box86)
* Added some workaround for program calling uname to mimic x86_64 system
* Added some granularity to mmap, and generic memory protection tracking improvment, to mimic x86 system (for wine stagging and proton)
* Better clone syscall and libc implementation
* Improved wrapping of GTK libs
* More wrapped libs (more gtk libs, like gstreamer-1.0 among other non-gtk libs too)
* More wrapped functions(with some tricky one, like dladdr1)
* Some fixes and improvment to some wrapped functions
* Refactor Dynarec:
    * better handling of SSE/x87 registers
    * optimisation to limit float/double conversion
    * better handling of "internal" jump inside a dynablock
    * make adding new architecture easier, by sharing as much infrastructure as possible
* Improved elfloader
* Improved some tests
* Improved a bit the "Hotpage" handling and messages
* Added more dynarec opcodes
* A few fixes in some Dynarec opcodes
* A bit more optimisation in some Dynarec opcodes
* Don't quit on unsupported syscall, return -1 / ENOSYS error
* Added some more Syscall
* Wrapped Vulkan library. Vulkan and dxvk now supported!
* Support building on PPC64LE (interpreter only)
* Added support for Risc-V (interpreter only)

v0.1.6
======
* Introduce "HotPage", to temporarily disable Dynarec on a page were writing is also occuring (can help speed up C# code)
* Some work on Dynarec to limit the number of mutex use, and also allow smaller block to be built (for JIT'd programs)
* Introduce BOX64_DYNAREC_STRONGMEM for the (Arm) dynarec to emulate StrngMemory Model (needed by some games, like RimWorld)
* More functions wrapped
* Improve speed of the custom allocator used by dynarec and Hash (speedup loading)
* Added a workaround for streamwebhelper to not load it
* More opcodes added
* More wrapped libraries, including gtk2
* Added a fix for linking box64 on system using glibc 2.34+
* Some residual fixes on thread attribute handling
* Better handling of app that want 32bits jumps, like unity3d/mono (thanks mogery)
* More ARM hardware option in CMake (Phytium, NX...)
* Added loongarch64 support (interpreter only)
* Some fixes to elfloader
* Added a mecanism to cancel a Dynarec block construction if it triggers a segfault (instead of a crash)

v0.1.4
======
* Reworked README a bit
* Added some more wrapped libs
* Added some SSE4x opcodes
* Preliminary support for VSyscall (still missing VDSO)
* Added support for AES-NI cpu extension, also in Dynarec (and using ARM extension if present)
* Added some syscall
* Added some more wrapped functions
* Improvment to the handling of longjmp inside signal handlers
* Improved wrapper helpers (from rajdakin)
* Some fixes to opcodes (especially C2 flags for some x87 opcodes)
* Handling of some specific opcode as privileged instruction (instead of NOP)
* Fixed ELF signature detection
* Some fixes to a few env. var. handling

v0.1.2
======
* OpenSource!
* A few more opcode added to the Interpreter and the Dynarec
* Improved Native Function calling
* Added some function to limit allocated memory to 47bits space (for Wine)
* Improved and Fixed the functions to limit allocated memory to 32bits space
* Wine is starting now, but not everything and every version works

v0.1.0
======
* Improvement to ElfLoader
* More opcode added to the Dynarec
* More lib wrapping
* Some fixes to a few function signature
* Improvement to Signal handling, and internal mutex handling
* A few fixes to some dynarec opcodes
* A few fixes to some interpreter opcodes

v0.0.8
======
* Improvement to SSE/x87 handling
* More opcode added to the Dynarec
* More lib wrapping

v0.0.6
======
* Added Dynarec for ARM64
* Many general CPU opcode added to the Dynarec
* Many SSEx opcodes added to the Dynarec
* Added a few more symbols in libc
* A few changes to the "thread once" handling
* New Logo and Icon from @grayduck

v0.0.4
======
* Added a lot of opcodes
* Added SDL1 and SDL2 wrapping
* OpenGL/GLU wrapping done
* libasound / libpulse wrapping done
* libopenal / libalure / libalut wrapping done
* Many X11 libs wrapped
* Added "install" with binfmt integration
* A few games are actualy starting now (old WorldOfGoo, Into The Breach, VVVVVV and Thimbeweed Park)

v0.0.2
======
* Using box86 as a base, implemented most planned box64 part except dynarec
* Wrapped a few libs
* ELF loader a bit crude, but seems working
* Signal handling implemented
* MMX and x87 are sharing their registers
