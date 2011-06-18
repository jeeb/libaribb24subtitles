include Makefile.global

LIBNAME = libaribb24captions
OBJ = src/utils/string-utils.o src/subs.o src/pts.o
DIRS = src

all: clean compile
	ar crs $(LIBNAME).a src/*.o src/utils/*.o

clean: clean-all
	rm *.o $(LIBNAME).a || true
