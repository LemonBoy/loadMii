# Copyright 2008-2009  Segher Boessenkool  <segher@kernel.crashing.org>
# This code is licensed to you under the terms of the GNU GPL, version 2;
# See file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

	.text
	.section .init

	.globl _start
	.type _start, @function
_start:
	b init
	.size _start, .-_start

	.globl __stub_magic
	.type __stub_magic, @object
__stub_magic:
	# Required by libogc 1.8.0
	.byte 'S','T','U','B','H','A','X','X'
	.size __stub_magic, .-__stub_magic

	.section .text
	.type init, @function
init:
	# Disable interrupts, enable FP.
	mfmsr 3 ; rlwinm 3,3,0,17,15 ; ori 3,3,0x2000 ; mtmsr 3 ; isync

	# Setup stack.
	lis 1,__stack_top@ha ; addi 1,1,__stack_top@l ; li 0,0 ; stwu 0,-64(1)

	# Initialize data.
	lis 3,__data_start@ha ; addi 3,3,__data_start@l
	lis 4,__data_init@ha ; addi 4,4,__data_init@l
	lis 5,__data_end@ha ; addi 5,5,__data_end@l ; sub 5,5,3
	bl memcpy

	# Clear BSS.
	lis 3,__bss_start@ha ; addi 3,3,__bss_start@l
	li 4,0
	lis 5,__bss_end@ha ; addi 5,5,__bss_end@l ; sub 5,5,3
	bl memset

	# Go!
	bl main

	# If it returns, hang. Shouldn't happen.
	b .
	.size init, .-init

	.globl __eabi
	.type __eabi, @function
__eabi:
	# main() calls __eabi() even when compiled with -mno-eabi
	# Just put a stub function here for that.
	blr
	.size __eabi, .-__eabi

