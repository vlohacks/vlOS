# vlOS

## What
A kernel writing adventure for fun and ~~profit~~ education

## Why
- why not?
- learning very low level coding

## How to get started

### Binaries
- download and unpack latest ISO image from [releases](https://github.com/vlohacks/vlOS/releases/)
- set up your favorite virtual machine (for example QEMU or VirtualBox) to i386 / 8M RAM... or be verwegen and burn a CD to boot it on real hardware ;-)
- run your vm / turn on your computer... the button turns it on :-) 

### Build from source code
- clone this repo
- set up a baremetal gcc/binutils toolchain (details coming soon)
- set up TOOLCHAIN variable in kernel/Makefile and testtask/Makefile to point to toolchain
- run `make iso` to build iso
- run `make run` to instantly run the iso in QEMU

