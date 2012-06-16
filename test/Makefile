# use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
#    Nachos assumes that the location of the program startup routine (the
# 	location the kernel jumps to when the program initially starts up)
#       is at location 0.  This means: start.o must be the first .o passed 
# 	to ld, in order for the routine "Start" to be loaded at location 0
#

# If you are running on SunOS 5.x machines, uncomment the next line
GCCDIR = /auto/home-scf-22/csci402/testgnu/decstation-ultrix/bin/
LDFLAGS = -T newscript -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

# if you aren't cross-compiling:
# GCCDIR =
# LDFLAGS = -N -T 0
# ASFLAGS =
# CPPFLAGS = -P $(INCDIR)


CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld
CPP = $(GCCDIR)cpp

INCDIR =-I../userprog -I../threads
CFLAGS = -G 0 -c $(INCDIR)

all: halt matmult sort testfiles locktest cvtest testcode exec testp1p2

start.o: start.s ../userprog/syscall.h
	$(CPP) $(CPPFLAGS) start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

halt.o: halt.c
	$(CC) $(CFLAGS) -c halt.c
halt: halt.o start.o
	$(LD) $(LDFLAGS) start.o halt.o -o halt.coff
	../bin/coff2noff halt.coff halt

sort.o: sort.c
	$(CC) $(CFLAGS) -c sort.c
sort: sort.o start.o
	$(LD) $(LDFLAGS) start.o sort.o -o sort.coff
	../bin/coff2noff sort.coff sort

matmult.o: matmult.c
	$(CC) $(CFLAGS) -c matmult.c
matmult: matmult.o start.o
	$(LD) $(LDFLAGS) start.o matmult.o -o matmult.coff
	../bin/coff2noff matmult.coff matmult

testfiles.o: testfiles.c
	$(CC) $(CFLAGS) -c testfiles.c
testfiles: testfiles.o start.o
	$(LD) $(LDFLAGS) start.o testfiles.o -o testfiles.coff
	../bin/coff2noff testfiles.coff testfiles

locktest.o: locktest.c
	$(CC) $(CFLAGS) -c locktest.c
locktest: locktest.o start.o
	$(LD) $(LDFLAGS) start.o locktest.o -o locktest.coff
	../bin/coff2noff locktest.coff locktest

cvtest.o: cvtest.c
	$(CC) $(CFLAGS) -c cvtest.c
cvtest: cvtest.o start.o
	$(LD) $(LDFLAGS) start.o cvtest.o -o cvtest.coff
	../bin/coff2noff cvtest.coff cvtest

testcode.o: testcode.c
	$(CC) $(CFLAGS) -c testcode.c
testcode: testcode.o start.o
	$(LD) $(LDFLAGS) start.o testcode.o -o testcode.coff
	../bin/coff2noff testcode.coff testcode

exec.o: exec.c
	$(CC) $(CFLAGS) -c exec.c
exec: exec.o start.o
	$(LD) $(LDFLAGS) start.o exec.o -o exec.coff
	../bin/coff2noff exec.coff exec

testp1p2.o: testp1p2.c
	$(CC) $(CFLAGS) -c testp1p2.c
testp1p2: testp1p2.o start.o
	$(LD) $(LDFLAGS) start.o testp1p2.o -o testp1p2.coff
	../bin/coff2noff testp1p2.coff testp1p2

clean:
	rm -f *.o *.coff
