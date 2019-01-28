//Xu's schemes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pbc.h>
#include <openssl/rand.h>
#include "hash.h"

#define MAX_NODE_NUM 1<<20 
#define Ld 10 //Tree height
#define Lw 10
#define parent(x) ((x-1)/2)
#define left(x) (x*2+1)
#define right(x) (x*2+2)

//tag 4bytes; lab 4byte; w 20bytes; ran 0bytes;
pairing_t pairing;
element_t g;
element_t g1, g2, h;//G1
element_t x1, x2, f1, t;//Zr

element_t s;//used in Dec

element_t ti;//Zr
element_t di, ei, D, E;//G1

element_t tempG1, tempG12;
element_t tempZr, tempZr2; 
element_t tempGT, tempGT2, tempGT3, tempGT4;

uint8_t C1[DEFAULT_BYTES];
element_t C2, C3;//G1
element_t T1, T2, T3, T4;//G1

struct Node
{
	uint8_t C1[32];
	element_t C2;
	element_t C3;
}
Node Td[MAX_NODE_NUM], Tw[MAX_NODE_NUM];
int node_num = 1<<(L+1)-1

uint8_t invalid_data[32];
uint8_t invalid_data_Td = -1;

int LAB = 0;

void initT(Node T[])
{
	int i;
	for (i=0; i<node_num; i++)
	{
		memset(T[i].C1, 0, sizeof(T[i].C1);
		element_init_G1(T[i].C2, pairing);
		element_init_G1(T[i].C3, pairing);
	}
}

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
	
	memset(invalid_data, 0, 32);

	element_init_Zr(s, pairing);
	element_random(s);

    element_init_G1(g, pairing);
    element_init_G1(g1, pairing);
    element_init_G1(g2, pairing);
    element_init_G1(h, pairing);
    element_init_G1(di, pairing);
    element_init_G1(ei, pairing);
	element_init_G1(D, pairing);
    element_init_G1(E, pairing);
    element_init_G1(tempG1, pairing);
    element_init_G1(tempG12, pairing);
    element_init_GT(tempGT, pairing);
    element_init_GT(tempGT2, pairing);
    element_init_GT(tempGT3, pairing);
    element_init_GT(tempGT4, pairing);

    element_init_G1(C2, pairing);
    element_init_G1(C3, pairing);

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
	element_random(ti);
    fx(tempZr2, ti);

    element_sub(tempZr, t, ti);
    element_div(tempZr, t, tempZr);
    element_mul(tempZr, tempZr2, tempZr);
    element_pow_zn(di, g2, tempZr);
	element_pow_zn(D, di, s);

    element_sub(tempZr, ti, t);
    element_div(tempZr, ti, tempZr);
    element_mul(tempZr, x1, tempZr);
    element_pow_zn(ei, g2, tempZr);
	element_pow_zn(E, ei, s);
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
void Enc(Node T[], int idx, uint8_t m[])
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
	xor(T[idx].C1, L, m);
	
    element_pow_zn(T[idx].C2, h, r);
    element_pow_zn(T[idx].C3, g, r);
    //print_hex(L, sizeof(L));
}

void Dec(uint8_t m[], Node T[], int idx)
{
	element_pairing(tempGT, T[idx].C2, E);
	element_pairing(tempGT2, T[idx].C3, D);
	element_mul(tempGT, tempGT, tempGT2);
	
	element_invert(tempZr, s);
	element_pow_zn(tempGT, tempGT, tempZr);
	
	uint8_t* degist[DEFAULT_BYTES];
	int n = element_length_in_bytes(tempGT);
	uint8_t* buff = (uint8_t*)malloc(n);
	element_to_bytes(buff, tempGT);
	
	H(degist, buff, n);
	
	xor(m, degist, T[idx].C1);
}

void random_gen_L(uint8_t * L, size_t n)
{

    while(1)
    {
        int ret=RAND_status();  /*检测熵值*/
        if(ret==1)
        {
            break;
        }
        else
        {
            RAND_poll();
        }
    }

    RAND_bytes(L, n);
}

//randomly select leaf node according to w
//len: the length of w
//result: leaf node set; return value: number of result;
int H2_w_to_set(int* result, const char w[], int len)
{
	int i;
	uint8_t* buff = (uint8_t*)malloc(DEFAULT_BYTES);
	uint8_t* msg = (uint8_t*)malloc(len+sizeof(K));
	memcpy(msg, w, len);
	memcpy(msg+len, K, sizeof(K));
	
	H(buff, msg, len+sizeof(K));
	free(msg);
	
	for (i=0; i<DEFAULT_BYTES/2; ++i)
	{
		result[i] = (buff[i*2]>>6) * (1<<8) + buff[i*2+1];
	}
	return DEFAULT_BYTES/2;
}

//from root to leaf
void Path(int path[], Node T[], int L, int l)
{
	int i;
	int node_idx=l;
	
	for (i=0; i<=L; i++)
	{
		path[i] = node_idx;
		node_idx = parent(node_idx);
	}
	
	for (i=0; i<=L/2; i++)
	{
		temp = path[i];
		path[i] = path[L-i];
		path[L-i] = temp;
	}
}

//生成节点label
int GenLab()
{
	return LAB++;
}

//访问Tw，找到对应关键词/无效数据，并且将其位置重置数据
//返回路径的序号,即层数
int AccessTw(uint8_t result[], int l, uint8_t w[], int len, uint8_t new_data[])
{
	int i;
	int path[Lw+1];
	uint8_t data[DEFAULT_BYTES];
	int result_l = -1;
	
	int match = 1;
	Path(path, Tw, Lw, l);
	
	for (i=0; i<=Lw; i++)
	{
		Dec(data, Tw, path[i])
		if (match == 1 && memcmp(w, data, len) == 0)
		{
			Enc(Tw, path[i], new_data);
			memcpy(result, data, DEFAULT_BYTES);
			result_l = i;
			match = 0;
		}
		else
		{
			Enc(Tw, path[i], data);
		}
	}
	return result_l;
}

//访问Td，找到对应关键词/无效数据，并且将其位置重置数据
//返回路径的序号,即层数
//lab||tag_k+1||lab_k+1||id||K||ran
int AccessTd(uint8_t result[], int l, int lab, uint8_t new_data[])
{
	int i;
	int path[Ld+1];
	uint8_t data[DEFAULT_BYTES];
	int result_l = -1;
	
	int match = 1;
	Path(path, Td, Ld, l);
	
	for (i=0; i<=Ld; i++)
	{
		Dec(data, Td, path[i])
		if (match == 1 && memcmp(data, (uint8_t*)&lab, 4)==0)
		{
			Enc(Td, path[i], new_data);
			memcpy(result, data, DEFAULT_BYTES);
			result_l = i;
			match = 0;
		}
		else
		{
			Enc(Td, path[i], data);
		}
	}
	return result_l;
}


//拼接Td节点中数据
void Td_data(uint8_t data[], int lab_k1, int tag_k2, int lab_k2, int idf)
{
	memcpy(data, (uint8_t*)&lab_k1, 4);
	memcpy(data+4, (uint8_t*)&tag_k2, 4);
	memcpy(data+8, (uint8_t*)&lab_k2, 4);
	memcpy(data+12, (uint8_t*)&idf, 4);
	memset(data+16, 0, 16);
}

void Tw_data_field(uint8_t data[], char w[], int* tag0, int* tag_k1, int* lab0, int* lab_k1)
{
	memcpy(w, (char*)data, 16);
	*tag0 = *((int*)(data+16));
	*tag_k1 = *((int*)(data+20));
	*lab0 = *((int*)(data+24));
	*lab_k1 = *((int*)(data+28));
}

void Tw_data(uint8_t data[], char w[], int tag0, int tag_k1, int lab0, int lab_k1)
{
	uint8_t buff[16];
	memset(buff, 0, 16);
	memcpy(buff, (uint8_t*)w, strlen(w));
	memcpy(data, buff, 16);
	memcpy(data+16, (uint8_t*)&tag0, 4);
	memcpy(data+20, (uint8_t*)&tag_k1, 4);
	memcpy(data+24, (uint8_t*)&lab0, 4);
	memcpy(data+28, (uint8_t*)&lab_k1, 4);
}

void upload(const char w[], int len, int id)
{
	int i;
	int buff[DEFAULT_BYTES];
	uint8_t new_data[DEFAULT_BYTES];
	int n = H2_w_to_set(buff, w, len);
	//int Lw_node_idx = 1<<Lw-1+buff[rand()%n];
	int base = 1<<Lw-1;
	
	uint8_t data[DEFAULT_BYTES];
	//找到对应关键词节点
	for (i=0; i<n; i++)
	{
		int l = AccessTw(data, base+buff[i], (uint8_t*)w, strlen(w), invalid_data);
		if (l==-1)
			continue
	}
    uint8_t result[DEFAULT_BYTES];
	//找到节点后，有两种情况，1）该关键词还没有对应文件id；2）追加文件id
	//1）没有对应文件id
    //w||tag0||tag_k+1||lab0||lab_k+1||ran---->16||4||4||4||4||0
	int tag0, tag_k1, lab0, lab_k1;
	Tw_data_field(data, &tag0, &tag_k1, &lab0, &lab_k1);
	if (lab0 == lab_k1)
	{
		tag_k1 = rand()%(1<<Ld);
		tag0= tag_k1;
		lab_k1 = GenLab();
		lab_0 = lab_k1;
	}
	int tag_k2 = rand()%(1<<Ld);
	int lab_k2 = GenLab();
	
	Td_data(new_data, lab_k1, tag_k2, lab_k2, id);
	AccessTd(new_data, 1<<Ld-1+tag_k1, invalid_data_Td, new_data);
	
	uint8_t data_Tw[DEFAULT_BYTES];
	
	int new_Lw_idx = base + buff[rand()%n];
	Tw_data(data_Tw, w, tag0, tag_k2, lab0, lab_k2);
	AccessTw(data, new_Lw_idx, invalid_data, data_Tw);
	
	int rannum = 3;
	for (i=0; i<rannum; i++)
	{
		AccessTw(data, base+rand()%(1<<Lw), invalid_data, invalid_data);
	}
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

