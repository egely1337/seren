<div align="center">

# Seren Operating System

</div>

Welcome to the Seren project. This is basically my playground for building a hobby operating system from scratch.

## What's This All About?

Seren is all about learning how operating systems actually work. The Nucleus Kernel is my attempt at a microkernel.

### Where We're At?

- It boots! (Thanks, Limine!)
- Got the basic higher-half kernel memory layout sorted.
- The build system is doing its thing.

## Building Seren

### Prerequisites

1. **A Cross-Compiler Toolchain (sounds scarier than it is):**

   - `x86_64-elf-gcc`
   - `nasm`

2. **QEMU:**

   - `qemu-system-x86_64` lets us run the OS in a safe little sandbox.

3. **Limine Bootloader Files:**

   - Good news! The Makefile has a script (`tools/fetch_limine.sh`) that'll try to download these for you.

4. **The Usual Suspects:**
   - `make`
   - `xorriso`
   - `bash`

### Building Seren

```bash
git clone https://github.com/ardytstrn/seren.git
cd seren
make iso
```

This command will:

- Compile the kernel source code.
- Link it all up into `build/nucleus.elf`.
- And finally, create a bootable `build/dist/seren.iso` image. Voila!

### Running Seren

```bash
make run
```

This should boot up your `seren.iso` in QEMU.

### Debugging Seren

1.  Start QEMU in "wait for GDB" mode:

```bash
make qemu_debug
```

2.  In a new terminal window, get GDB going:

```bash
(gdb) file build/nucleus.elf
Reading symbols from build/nucleus.elf...
(gdb) target remote localhost:1234
Remote debugging using localhost:1234
0x000000000000fff0 in ?? ()
(gdb) b kmain
Breakpoint 1 at 0xffffffff80001020: file nucleus/main.c, line 11.
(gdb) c
Continuing.

Breakpoint 1, kmain () at nucleus/main.c:11
11 if (framebuffer_request.response == NULL ||
```

## Wanna Help?

Right now, this is mostly a "learn-as-I-go" project. But hey, feel free to look through the code! If you spot a bug or have a cool idea, don't be shy - open an issue or shoot a pull request. Contributions are always welcome!

## Legal Stuff (The License)

This project is licensed under MIT License - check out [LICENSE](LICENSE) file for the boring details.
