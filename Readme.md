# Tiger in xv6

## How to run it:

First of all, make sure you have the RISC-V compiler toolchain and Qemu installed. Here is an instruction from
[MIT](https://pdos.csail.mit.edu/6.828/2024/tools.html).

Then, clone the repo:
```shell
git clone https://github.com/bajzc/xv6-riscv --recursive
cd xv6-riscv
```

Put your tiger program under the top dir ``test.tig``, and then run:
```shell
make qemu
```

This will do several things for you:
1. Compile the xv6 system
2. Compile the tiger compiler (host: your system target: riscv-xv6)
3. Compile the ``test.tig`` using the compiler generated in step 2 to assembly
4. Link the ``test.tig.s`` with the xv6 user library
5. Compile another tiger compiler (host: riscv-xv6 target: riscv-xv6)
6. Write the base system, ``test.tig`` and output from step 4 and 5 to ``fs.img``
7. Boot xv6 from ``fs.img``

Once you are in xv6, you could try to run the program compiled from step 4, which is named ``runtime`` because it is
actually linked with the ``runtime.c`` to provide full functionality. Or you could try compiling again using the compiler
inside xv6 (from step 5) and comparing the result with ``test.tig.s`` (from step 3).

```shell
# in xv6
$ tigerc test.tig
# it might takes a while...
$ cat test.tig.s
# check out the output
# press Ctrl-a-x to quit qemu
```