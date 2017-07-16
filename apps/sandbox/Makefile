CC      = gcc
LD	= ld
RANLIB	= ranlib
CFLAGS  = -Wall -O0 -g -MD -I ../../
LDFLAGS = -O0 -g -L. -L ../../libdune -lsandbox -ldune -lpthread -T sandbox.ld
OBJ     = main.o umm.o trap.o exec.o

all: libsandbox.a sandbox

libsandbox.a: $(OBJ)
	$(AR) cru $(@) $(OBJ)
	$(RANLIB) $(@)

sandbox: nullsandbox.o libsandbox.a ../../libdune/libdune.a
	$(CC) -o $(@) $(<) $(LDFLAGS)

clean:
	rm -f *.o test *.d sandbox libsandbox.a

-include *.d
