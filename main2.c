//Xu's schemes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pbc.h>
#include "hash.h"

pairing_t pairing;
element_t g;
element_t g1, g2, h;//G1
element_t x1, x2, f1, t;//Zr

element_t ti;//Zr

element_t Di, Ei;//G1

element_t tempG1, tempG12;
element_t tempZr, tempZr2;
element_t tempGT, tempGT2, tempGT3, tempGT4;

element_t C1, C2, C3, C4;//G1
element_t T1, T2, T3, T4;//G1

void print_hex(uint8_t* p, int len)
{
    int i=0;
    for (i=0; i<len; ++i)
    {
        printf("%0X", p[i]);
    }
    printf("\n");
}

//fx = x1 + f1x
void fx(element_t out, element_t x)
{
    element_mul(tempZr, f1, x);
    element_add(out, x1, tempZr);
}

void Setup()
{
    //参数配置
    char param[1024];
    FILE* f = fopen("a.param", "r");
    size_t count = fread(param, 1, 1024, f);
    if (!count) pbc_die("input error");
    pairing_init_set_buf(pairing, param, count);

    element_init_G1(g, pairing);
    element_init_G1(g1, pairing);
    element_init_G1(g2, pairing);
    element_init_G1(EK, pairing);
    element_init_G1(Di, pairing);
    element_init_G1(Ei, pairing);
    element_init_G1(tempG1, pairing);
    element_init_G1(tempG12, pairing);
    element_init_GT(tempGT, pairing);
    element_init_GT(tempGT2, pairing);
    element_init_GT(tempGT3, pairing);
    element_init_GT(tempGT4, pairing);

    element_init_G1(C1, pairing);
    element_init_G1(C2, pairing);
    element_init_G1(C3, pairing);
    element_init_G1(C4, pairing);

    element_init_G1(T1, pairing);
    element_init_G1(T2, pairing);
    element_init_G1(T3, pairing);
    element_init_G1(T4, pairing);

    element_init_Zr(x1, pairing);
    element_init_Zr(x2, pairing);
    element_init_Zr(f1, pairing);
    element_init_Zr(t, pairing);
    element_init_Zr(ti, pairing);
    element_init_Zr(tempZr, pairing);
    element_init_Zr(tempZr2, pairing);

    element_random(g);
    element_random(x1);
    element_random(x2);
    element_random(f1);
    element_random(t);

    element_pow_zn(g1, g, x1);
    element_pow_zn(g2, g, x2);

    //h
    fx(tempZr, t);
    element_div(tempZr, tempZr, x1);
    element_pow_zn(h, g, tempZr);
}

// 1. t/(t-ti) 2. ti/(ti-t)
void SKeyGen()
{
    fx(tempZr2, ti);

    element_sub(tempZr, t, ti);
    element_div(tempZr, t, tempZr);
    element_mul(tempZr, tempZr2, tempZr);
    element_pow_zn(Di, g2, tempZr);

    element_sub(tempZr, ti, t);
    element_div(tempZr, ti, tempZr);
    element_mul(tempZr, x1, tempZr);
    element_pow_zn(Ei, g2, tempZr);
}

void xor(uint8_t out[], uint8_t x[], uint8_t y[])
{
	int i;
	for (i=0; i<DEFAULT_BYTES; ++i)
	{
		out[i] = x[i] ^ y[i];
	}
}

//C1, C2, C3
void Enc(uint8_t m[], int len)
{
    uint8_t* buff;
    uint8_t L[DEFAULT_BYTES];
	int n;
    element_t r;

    element_init_Zr(r, pairing);
    element_random(r);
	
    element_pow_zn(tempG1, g2, r);
	element_pairing(tempG1, g1, tempG1);
	n = element_length_in_bytes(tempG1);
	buff = (uint8_t)malloc(n);
	element_to_bytes(buff, tempG1);
	H(L, buff, n); 
	xor(C1, L, m);
	
    element_pow_zn(C2, h, r);
    element_pow_zn(C3, g, r);
    //print_hex(L, sizeof(L));
}

//T1, T2, T3, T4
void Trap(char q[], int len)
{
    element_t s;

    uint8_t L[DEFAULT_BYTES];

    element_init_Zr(s, pairing);
    element_random(s);

    element_pow_zn(T1, g1, s);
    H(L, q, len);
    element_from_hash(tempG1, L, DEFAULT_BYTES);
    element_pow_zn(T2, tempG1, s);
    element_pow_zn(T3, Ei, s);
    element_pow_zn(T4, Di, s);

    //print_hex(L, sizeof(L));
}

int Match()
{
    element_pairing(tempGT, C1, T1);
    element_pairing(tempGT2, C2, T2);
    element_pairing(tempGT3, C3, T3);
    element_pairing(tempGT4, C4, T4);

    element_mul(tempGT3, tempGT3, tempGT4);
    element_mul(tempGT2, tempGT2, tempGT3);

    if (element_cmp(tempGT, tempGT2) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}

int RevCheck()
{
    element_t r;

    element_init_Zr(r, pairing);
    element_random(r);

    element_t D_i_r, E_i_r;
    element_init_G1(D_i_r, pairing);
    element_init_G1(E_i_r, pairing);

    element_pow_zn(D_i_r, Di, r);
    element_pow_zn(E_i_r, Ei, r);

    element_pairing(tempGT, T3, D_i_r);
    element_pairing(tempGT2, E_i_r, T4);

    if (element_cmp(tempGT, tempGT2) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int main()
{
    int i;
    char w[10] = "zhao";
    char* q[] = {"zhao", "zh", "zha", "zk", "kdk", "zhao"};

    int result = 2;

    Setup();
    SKeyGen();
    Enc(w, strlen(w));

    printf("index: %s\n", w);
    for (i=0; i<sizeof(q)/sizeof(char*); ++i)
    {
        Trap(q[i], strlen(q[i]));
        result = Match();

        printf("trapdoor word:%s, result: %s\n", q[i], result==1?"OK":"Fail");
    }

    printf("revoke: %s\n", result==1?"OK":"Fail");


/*
    uint8_t L[32];
    uint8_t L2[128];

    element_t test_g, test_g2;
    element_init_G1(test_g, pairing);
    element_init_G1(test_g2, pairing);

    element_from_hash(test_g, L, 32);

    H(L2, "zha", 3);
    element_from_hash(test_g2, L2, 32);

    printf("%d\n", element_cmp(test_g, test_g2));
*/
}

