include ../Makefile.global

DIRS = utils
OBJ = subs.o pts.o packet.o
LINKOBJ =  $(TSOBJ) $(CAPOBJ) $(OBJ)

all: $(OBJ) compile
	
clean: clean-all
	rm *.o || true
