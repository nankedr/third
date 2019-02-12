//2018-TDSC-Proxy-Free Privacy-Preserving Task Matching with Efficient Revocation in Crowdsourcing
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pbc.h>
#include "hash.h"
//每行最多10个词
#define MAX_FILE_NUM 15000
#define MAX_WORDS_NUM 15
#define MAX_CHAR 500
char strLine[MAX_CHAR];

pairing_t pairing;
element_t g;
element_t g1, g2, EK;//G1
element_t x1, x2, f1, t;//Zr

element_t ti;//Zr

element_t Di, Ei;//G1

element_t tempG1, tempG12;
element_t tempZr, tempZr2;
element_t tempGT, tempGT2, tempGT3, tempGT4;

element_t C1, C2, C3, C4;//G1
element_t T1, T2, T3, T4;//G1

struct Cipher
{
	element_t C1;
	element_t C2;
	element_t C3;
	element_t C4;
};
struct Cipher f_index[MAX_FILE_NUM][MAX_WORDS_NUM];//forward index
int file_num;
int words_num[MAX_FILE_NUM];

void construct_index(char* w, int len, int doc_id)
{
	int i = words_num[doc_id];
	element_init_G1(f_index[doc_id][i].C1, pairing);
	element_init_G1(f_index[doc_id][i].C2, pairing);
	element_init_G1(f_index[doc_id][i].C3, pairing);
	element_init_G1(f_index[doc_id][i].C4, pairing);
	
	Enc(w, len, doc_id, i);
	words_num[doc_id] += 1;
}

int read_index()
{
    FILE* fp;
    char* s;
    uint32_t doc_id;
    int doc_num = 0;

    if ((fp=fopen("index_test_1", "r")) == NULL)
    {
        printf("no index file!\n");
        return -1;
    }
    while (!feof(fp))
    {
        fgets(strLine, MAX_CHAR, fp);
        if (feof(fp))
            break;
        strLine[strlen(strLine)-1] = '\0';
        s = strtok(strLine, " ");
        if (!s)
            break ;
        doc_id = atoi(s);
        ++doc_num;
        //printf("%d\n", atoi(s));

        while (1)
        {
            s = strtok(NULL, " ");
            if (!s)
                break;
            printf("%s\n", s);
		
            construct_index(s, strlen(s), doc_id);
        }
		printf("index:%d\n", doc_id);
    }
    fclose(fp);
    return doc_num;
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

    //EK
    fx(tempZr, t);
    element_div(tempZr, tempZr, x1);
    element_pow_zn(EK, g, tempZr);
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

//C1, C2, C3, C4
void Enc(char w[], int len, int doc_id, int i)
{
    uint8_t* buff;

    uint8_t L[DEFAULT_BYTES];

    element_t r1, r2;

    element_init_Zr(r1, pairing);
    element_init_Zr(r2, pairing);

    element_random(r1);
    element_random(r2);

    H(L, w, len);
    element_from_hash(tempG1, L, DEFAULT_BYTES);
    element_pow_zn(tempG1, tempG1, r1);
    element_pow_zn(tempG12, g2, r2);
    element_mul(f_index[doc_id][i].C1, tempG12, tempG1);
    element_pow_zn(f_index[doc_id][i].C2, g1, r1);
    element_pow_zn(f_index[doc_id][i].C3, EK, r2);
    element_pow_zn(f_index[doc_id][i].C4, g, r2);
	
	element_clear(r1);
	element_clear(r2);

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
	
	element_clear(s);

    //print_hex(L, sizeof(L));
}

int Match(int doc_id, int i)
{
    element_pairing(tempGT, f_index[doc_id][i].C1, T1);
    element_pairing(tempGT2, f_index[doc_id][i].C2, T2);
    element_pairing(tempGT3, f_index[doc_id][i].C3, T3);
    element_pairing(tempGT4, f_index[doc_id][i].C4, T4);

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
	
	element_clear(r);

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
    // int i;
    // char w[10] = "zhao";
    // char* q[] = {"zhao", "zh", "zha", "zk", "kdk", "zhao"};

    // int result = 2;

    // Setup();
    // SKeyGen();
    // Enc(w, strlen(w));

    // printf("index: %s\n", w);
    // for (i=0; i<sizeof(q)/sizeof(char*); ++i)
    // {
        // Trap(q[i], strlen(q[i]));
        // result = Match();

        // printf("trapdoor word:%s, result: %s\n", q[i], result==1?"OK":"Fail");
    // }

    // printf("revoke: %s\n", result==1?"OK":"Fail");
	float time_use=0;
	struct timeval start, end;
	int doc_num;
	int index_mem_use = 0;
	int n = 0;
	char input[50];
	
	Setup();
    printf("Setup finish\n");              
	SKeyGen();
    printf("SKeyGen finish\n");
	
	gettimeofday(&start, NULL);
    
    doc_num = read_index();
	
	gettimeofday(&end, NULL);
	time_use = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec);//微秒
	printf("document:%d\n", doc_num);
	printf("time: %f\n", time_use/1e6);
	
	index_mem_use = (sizeof(struct Cipher) + element_length_in_bytes(f_index[0][0].C1)
					+ element_length_in_bytes(f_index[0][0].C2)+ element_length_in_bytes(f_index[0][0].C3)
					+ element_length_in_bytes(f_index[0][0].C4))*doc_num
					+ 4* doc_num;
					
	printf("memory: %fM\n", ((double)index_mem_use)/1024/1024);
    printf("index construction complete...\n");
    
	while(1)
	{	
		
		fgets(input, 50, stdin);
        input[strlen(input)-1] = '\0';
		gettimeofday(&start, NULL);
		
		Trap(input, strlen(input));
		n = 0;
		for (int i=0; i< doc_num; i++)
		{
			for (int j=0; j<words_num[i]; j++)
			{
				if (Match(i, j)==1){
					n++;
					printf("%d\n", i);
					break;
				}
			}
		}
		gettimeofday(&end, NULL);
		time_use = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec);//微秒
		printf("time_used:%f, doc_num:%d\n", time_use/1e6, n);
	}

}

