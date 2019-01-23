all:test
LIBS = -lssl -lcrypto -lpbc -lgmp
OBJS = test.o
INCLUDES = /usr/local/include/pbc
test:main.c hash.h
	gcc -g -W -Wall -I $(INCLUDES) $(LIBS) -c  $< -o $(OBJS)
	g++ -o $@ $(OBJS) $(LIBS)
clean:
	rm test

