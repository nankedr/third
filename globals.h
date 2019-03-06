#ifndef GLOBALS_H_INCLUDED
#define GLOBALS_H_INCLUDED

#include <pbc.h>
struct Node
{
	uint8_t C1[32];
	element_t C2;
	element_t C3;
};

#define Ld 15 //Tree height
#define Lw 12
#define MAX_CHAR 200
#define MAX_NODE_NUM 1<<20 

#endif
