include ../../Makefile.global

APPNAME = echohexaschar
OBJ = main.o

all: clean $(OBJ)
	$(CC) $(CFLAGS) -o $(APPNAME) $(OBJ) 

clean:
	rm *.o $(APPNAME) || true