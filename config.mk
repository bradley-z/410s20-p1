# To build your project, edit this file--not the Makefile!

##################################################
# How to acquire updates.
##################################################
#
# afs - fastest on Andrew machines
# web - will probably work on non-Andrew machines
# offline - if machine has no network access
#
# WARNING - if you miss a staff-issued update
# because you were using "offline", you are
# responsible for the outcome.
#
##################################################
UPDATE_METHOD = afs

##################################################
# Compiler
##################################################
# Selections (see handout for details):
#
# gcc - default (what we will grade with)
# clang - Clang/LLVM
# clangalyzer - Clang/LLVM plus static analyzer
#
# "gcc" may have a better Simics debugging experience
#
# "clang" may provide helpful warnings, assembly
# code may be more readable
#
# "clangalyzer" will likely complain more than "clang"
#
# Use "make veryclean" if you adjust CC.
CC = gcc

##################################################
# Tab stops
##################################################
# If you use tabstops set to something other than
# the international standard of eight characters,
# this is your opportunity to inform our print
# scripts.
TABSTOP = 8

##################################################
# DEBUG
##################################################
# You can set CONFIG_DEBUG to "kernel" if you want
# the DEBUG flag to be #define'd for kernel code
# (all of P1 is kernel code).  This will cause
# the checks in contracts.h to be made, at the
# expense of some performance.
#
# Use "make veryclean" if you adjust CONFIG_DEBUG.
#
CONFIG_DEBUG = kernel

##################################################
# Object files from kern/ which the game AND the
# console tester should link against -- basically,
# the object files which make up your drivers.
##################################################
#
COMMON_OBJS = console.o handlers.o handlers_asm.o timer.o kb_buffer.o kb.o

##################################################
# Object files from 410kern/ for just the game
# (in other words, any game-specific helper code
# we release to you--some semesters, there isn't
# any).
##################################################
#
410_GAME_OBJS =

##################################################
# Object files from kern/ for just the game:
# this allows you to break up your game into
# multiple parts.
##################################################
#
KERN_GAME_OBJS = game.o sokoban_game.o

##################################################
# Object files from 410kern/ for just the tester
# (you should not need to change this).
##################################################
#
410TEST_OBJS = 410_test.o
