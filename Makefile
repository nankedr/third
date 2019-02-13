all:test_main2 test_mainTDSC
LIBS = -lssl -lcrypto -lpbc -lgmp
OBJS0 = main2.o
OBJS1 = mainTDSC.o
INCLUDES = /usr/local/include/pbc
test_main2:main2.c hash.h
	gcc -g -W -Wall -I $(INCLUDES) $(LIBS) -c  $< -o $(OBJS0)
	g++ -o $@ $(OBJS0) $(LIBS)
test_mainTDSC:mainTDSC.c hash.h
	gcc -g -W -Wall -I $(INCLUDES) $(LIBS) -c  $< -o $(OBJS1)
	g++ -o $@ $(OBJS1) $(LIBS)
clean:
	rm -f *.o test_main2 test_mainTDSC

