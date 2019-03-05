all:test_main2 test_mainTDSC
LIBS = -lssl -lcrypto -lpbc -lgmp
OBJS0 = main2.o
OBJS1 = mainTDSC.o
OBJS2 = main2_part.o
OBJS3 = hash.o
INCLUDES = /usr/local/include/pbc

$(OBJS0):main2.c hash.h globals.h
	gcc -g -W -Wall -I $(INCLUDES) $(LIBS) -c  $< -o $@
	
$(OBJS1):mainTDSC.c hash.h
	gcc -g -W -Wall -I $(INCLUDES) $(LIBS) -c  $< -o $@

$(OBJS2):main2_part.c hash.h globals.h
	gcc -g -W -Wall -I $(INCLUDES) $(LIBS) -c  $< -o $@
$(OBJS3):hash.c
	gcc -g -W -Wall -I $(INCLUDES) $(LIBS) -c  $< -o $@
	
test_main2:$(OBJS0) $(OBJS2) $(OBJS3)
	g++ -o $@ $^ $(LIBS)
	
test_mainTDSC:$(OBJS1) $(OBJS3)
	g++ -o $@ $^ $(LIBS)
clean:
	rm -f *.o test_main2 test_mainTDSC

