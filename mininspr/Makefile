CFLAGS=-D _PR_PTHREADS

all: libthread.a ../dist

libthread.a: ptsynch.o pratom.o prthread.o prtpd.o
	ar rc libthread.a ptsynch.o pratom.o prthread.o prtpd.o

../dist:
	mkdir ../dist
	mkdir ../dist/Linux_All_OPT.OBJ
	mkdir ../dist/Linux_All_OPT.OBJ/include
	mkdir ../dist/Linux_All_OPT.OBJ/lib
	cp libthread.a ../dist/Linux_All_OPT.OBJ/lib/libnspr4.a
	cp *.h ../dist/Linux_All_OPT.OBJ/include

clean:
	rm -rf *.o libthread.a *~ ../dist
