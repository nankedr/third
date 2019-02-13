//Xu's schemes;;;;Tag:the node in last level(index from the first node in last level); 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pbc.h>
#include <openssl/rand.h>
#include "hash.h"

#define MAX_CHAR 200
#define MAX_NODE_NUM 1<<20 
#define Ld 15 //Tree height
#define Lw 13
#define parent(x) ((x-1)/2)
#define left(x) (x*2+1)
#define right(x) (x*2+2)

float time_use=0;
struct timeval start, end;

char strLine[MAX_CHAR];
char word_space[5000][20];

//tag 4bytes; lab 4byte; w 20bytes; ran 0bytes;
pairing_t pairing;
element_t g;
element_t g1, g2, h;//G1
element_t x1, x2, f1, t;//Zr

element_t s;//used in Dec

element_t ti;//Zr
element_t di, ei, D, E;//G1

element_t tempG1;
element_t tempZr, tempZr2; 
element_t tempGT, tempGT2;

uint8_t K[32];

struct Node
{
	uint8_t C1[32];
	element_t C2;
	element_t C3;
};
struct Node Td[MAX_NODE_NUM], Tw[MAX_NODE_NUM];
int node_num_Tw = (1<<(Lw+1))-1;
int node_num_Td = (1<<(Ld+1))-1;

uint8_t invalid_data[32];
int invalid_label_Td = -1;

uint8_t invalid_data_Td[DEFAULT_BYTES];

int LAB = 0;
int file_id[1<<20];

void Geninvalid_data_Td()
{
	memset(invalid_data_Td, 0, 32);
	memcpy(invalid_data_Td, (uint8_t*)&invalid_label_Td, 4);
}
void initT(struct Node T[], int level)
{
	int i;
	for (i=0; i<(1<<(level+1))-1; i++)
	{
		memset(T[i].C1, 0, sizeof(T[i].C1));
		element_init_G1(T[i].C2, pairing);
		element_init_G1(T[i].C3, pairing);
	}
}
void initDataTwTd(int len)
{
	int i;
	initT(Tw, Lw);
	initT(Td, Ld);
	
	for (i=0; i<node_num_Tw; i++)
	{
		Enc(Tw, i, invalid_data);
	}
	for (i=0; i<node_num_Td; i++)
	{
		Enc(Td, i, invalid_data_Td);
	}
	
	uint8_t temp[DEFAULT_BYTES];
	int buff[DEFAULT_BYTES];
	
	for (i=0; i<len; i++)
	{
		printf("%s\n", w[i]);
		int n = H2_w_to_set(buff, word_space[i], strlen(word_space[i]));
		int Lw_node_idx = (1<<Lw)-1+buff[rand()%n];
		memset(temp, 0, DEFAULT_BYTES);
		memcpy(temp, (uint8_t*)word_space[i], strlen(word_space[i]));	
		while (AccessTw(temp, Lw_node_idx, invalid_data, temp)==-1)
		{
			Lw_node_idx = (1<<Lw)-1+buff[rand()%n];
			memset(temp, 0, DEFAULT_BYTES);
			memcpy(temp, (uint8_t*)word_space[i], strlen(word_space[i]));
		}
	}
}

void print_hex(uint8_t* p, int len)
{
    int i=0;
    for (i=0; i<len; ++i)
    {
        printf("%02X", p[i]);
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
	Geninvalid_data_Td();
	
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
    element_init_GT(tempGT, pairing);
    element_init_GT(tempGT2, pairing);

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
void Enc(struct Node T[], int idx, uint8_t m[])
{
    uint8_t* buff;
    uint8_t L[DEFAULT_BYTES];
	int n;
    element_t r;
    element_init_Zr(r, pairing);
    element_random(r);
	
    element_pow_zn(tempG1, g2, r);
	element_pairing(tempGT, g1, tempG1);
	n = element_length_in_bytes(tempGT);
	buff = (uint8_t*)malloc(n);
    if (buff == NULL)
        return;
	element_to_bytes(buff, tempGT);
	H(L, buff, n); 
	xor(T[idx].C1, L, m);
	
    element_pow_zn(T[idx].C2, h, r);
    element_pow_zn(T[idx].C3, g, r);
    //print_hex(L, sizeof(L));
    free(buff);
    element_clear(r);
}

void Dec(uint8_t m[], struct Node T[], int idx)
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


//randomly select leaf node according to w
//len: the length of w
//result: leaf node set; return value: number of result;
//调节移动位数
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
		//result[i] = buff[i]>>(8-Lw);
	}
	return DEFAULT_BYTES/2;
}

//from root to leaf
void Path(int path[], int L, int l)
{
	int i;
	int node_idx=l;
	int temp; 
	
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
int AccessTw(uint8_t result[], int l, uint8_t w[], uint8_t new_data[])
{
	int i;
	int path[Lw+1];
	uint8_t data[DEFAULT_BYTES];
	int result_l = -1;
	
	int match = 1;
	Path(path, Lw, l);
	
	for (i=0; i<=Lw; i++)
	{
		Dec(data, Tw, path[i]);
		if (match == 1 && memcmp(w, data, 16) == 0)
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
	Path(path, Ld, l);
	
	for (i=0; i<=Ld; i++)
	{
		Dec(data, Td, path[i]);
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

//提取Td节点中数据
void Td_data_field(uint8_t data[], int* lab_k1, int* tag_k2, int* lab_k2, int* idf)
{
	*lab_k1 = *((int*)data);
	*tag_k2 = *((int*)(data+4));
	*lab_k2 = *((int*)(data+8)); 
	*idf = *((int*)(data+12));
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

void Upload(const char w[], int len, int id)
{
	int i,l;
	uint8_t temp[16];
	int buff[DEFAULT_BYTES];
	uint8_t new_data[DEFAULT_BYTES];
	int n = H2_w_to_set(buff, w, len);
	//int Lw_node_idx = 1<<Lw-1+buff[rand()%n];
	int base = (1<<Lw)-1;
	
	uint8_t data[DEFAULT_BYTES];
	memset(temp, 0, 16);
	memcpy(temp, (uint8_t*)w, strlen(w));
	//找到对应关键词节点
	for (i=0; i<n; i++)
	{
		l = AccessTw(data, base+buff[i], temp, invalid_data);
		if (l != -1)
			break;
	}
	if (l == -1)
	{
		printf("no index word..upload\n");
		return;
	}
    //w||tag0||tag_k+1||lab0||lab_k+1||ran---->16||4||4||4||4||0
	int tag0, tag_k1, lab0, lab_k1;
	Tw_data_field(data, temp, &tag0, &tag_k1, &lab0, &lab_k1);
	if (lab0 == lab_k1)
	{
		tag_k1 = rand()%(1<<Ld);
		tag0= tag_k1;
		lab_k1 = GenLab();
		lab0 = lab_k1;
	}
	int tag_k2 = rand()%(1<<Ld);
	int lab_k2 = GenLab();
	
	Td_data(new_data, lab_k1, tag_k2, lab_k2, id);
	if (AccessTd(new_data, (1<<Ld)-1+tag_k1, invalid_label_Td, new_data) == -1)
	{
		printf("no valid space for Td\n");
		exit(0);
	}
	
	uint8_t data_Tw[DEFAULT_BYTES];
	
	int new_Lw_idx = base + buff[rand()%n];
	Tw_data(data_Tw, w, tag0, tag_k2, lab0, lab_k2);
	while (AccessTw(data, new_Lw_idx, invalid_data, data_Tw) == -1)//loop insert many times in case there are no invalide_data space for data_Tw
	{
		new_Lw_idx = base + buff[rand()%n];
		Tw_data(data_Tw, w, tag0, tag_k2, lab0, lab_k2);
	}
	
	int rannum = 3;
	for (i=0; i<rannum; i++)
	{
		AccessTw(data, base+rand()%(1<<Lw), invalid_data, invalid_data);
	}
}


int sub_leaf(int tag, int level)
{
	int path[Ld+1];
	Path(path, Ld, (1<<Ld)-1+tag);
	int left_node = path[level];
	int right_node = left_node;
	
	while (1)
	{
		if (left(left_node) >= (1<<Ld)-1)
			break;
		left_node = left(left_node);
		right_node = right(right_node);
	}
	int first_node = (1<<Ld)-1;
	int diff = right_node-left_node;
	if (diff == 0)
	{
		return left_node-first_node;
	}
	int ran = rand()%diff;
	return ran + left_node - first_node;
}

int Search(const char w[], int len)
{
	int i;
	uint8_t w_buff[16];
	int l;
	int buff[DEFAULT_BYTES];
	int n = H2_w_to_set(buff, w, len);
	int base = (1<<Lw)-1;
	
	uint8_t data[DEFAULT_BYTES];
	memset(w_buff, 0, 16);
	memcpy(w_buff, (uint8_t*)w, strlen(w)); 
	
	//找到对应关键词节点
	for (i=0; i<n; i++)
	{
		l = AccessTw(data, base+buff[i], w_buff, invalid_data);
		if (l!=-1)
			break;
	}
	if (l==-1)
	{
		printf("%s\n", "no this index word");
		return 0;
	}
	int j = 0;
	
    uint8_t data_Td[DEFAULT_BYTES];
    //w||tag0||tag_k+1||lab0||lab_k+1||ran---->16||4||4||4||4||0
	int tag0, tag_k1, lab0, lab_k1;
	Tw_data_field(data, w_buff, &tag0, &tag_k1, &lab0, &lab_k1);
	int lab_cur=lab0, tag_cur=tag0;
	int lab_next, tag_next;
	if (lab0 == lab_k1)
	{
		tag_next = tag0;
		lab_next = lab0;
	}
	else
	{
		while (lab_cur != lab_k1)
		{
			int idf;
			if (AccessTd(data_Td, (1<<Ld)-1+tag_cur, lab_cur, invalid_data_Td)==-1)
			{
				printf("no valid space for Td\n");
				exit(0);
			}
			Td_data_field(data_Td, &lab_cur, &tag_next, &lab_next, &idf); 
			printf("%d\n", idf); 
			file_id[j++] = idf;
			lab_cur = lab_next;
			tag_cur = tag_next;
		}
	}
	
	if (j != 0 )
	{
		tag_next = tag_k1;
		lab_next = lab_k1;
		
		for (int p=0; p<j; p++)
		{
			tag_cur = rand()%(1<<Ld);
			lab_cur = GenLab();
			Td_data(data_Td, lab_cur, tag_next, lab_next, file_id[p]);
			int temp_i = AccessTd(data, (1<<Ld)-1+tag_cur, invalid_label_Td, data_Td);
			if (temp_i == -1)
			{
				printf("no valid space for Td\n");
				exit(0);
			}
			
			lab_next = lab_cur;
			tag_next = sub_leaf(tag_cur, temp_i);
		}
	}
	
	uint8_t data_Tw[DEFAULT_BYTES];
	
	int new_Lw_idx = base + buff[rand()%n];
	Tw_data(data_Tw, w, tag_next, tag_k1, lab_next, lab_k1);

	while (AccessTw(data, new_Lw_idx, invalid_data, data_Tw)==-1)
	{
		new_Lw_idx = base + buff[rand()%n];
		Tw_data(data_Tw, w, tag_next, tag_k1, lab_next, lab_k1);
	}
	
	int rannum = 3;
	for (i=0; i<rannum; i++)
	{
		AccessTw(data, base+rand()%(1<<Lw), invalid_data, invalid_data);
	}
	
	return j;
}

int read_index()
{
    FILE* fp;
    char* s;
    uint32_t doc_id;
    int doc_num = 0;

    if ((fp=fopen("index_test", "r")) == NULL)
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
		
            Upload(s, strlen(s), doc_id);
        }
		printf("index:%d\n", doc_id);
    }
    fclose(fp);
    return doc_num;
}

int initword_space()
{
	FILE* fp;
    int i = 0;

    if ((fp=fopen("word_space_test", "r")) == NULL)
    {
        printf("no word space file!\n");
        return -1;
    }
    while (!feof(fp))
    {
        fgets(strLine, MAX_CHAR, fp);
        if (feof(fp))
            break;
        strLine[strlen(strLine)-1] = '\0';
        strncpy(word_space[i++], strLine, strlen(strLine));
	}
	return i;
}

void testEncDec()
{
	uint8_t m[DEFAULT_BYTES];
	Geninvalid_data_Td();
	Enc(Tw, 0, invalid_data_Td);
	Dec(m, Tw, 0);
	
	print_hex(m, DEFAULT_BYTES);
}

void printT(struct Node T[], int level)
{
    uint8_t buff[32];
	for (int i=0; i<(1<<(level+1))-1; i++)
	{
        Dec(buff, T, i);
        print_hex(buff, 32);
	}
}

int main()
{
	int words_num, doc_num;
	int index_mem_use = 0;
	int n;
	char input[50];
	Setup();
     printf("Setup finish\n");              
	SKeyGen();
    printf("SKeyGen finish\n");
	
	//initT(Tw);
	//testEncDec();
	
	gettimeofday(&start, NULL);
	words_num = initword_space();
    printf("initword_space finish\n");

    initDataTwTd(words_num);
    printf("initDataTwTd finish\n");
    
    doc_num = read_index();
	printf("dddddd\n");
	
	gettimeofday(&end, NULL);
	time_use = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec);//微秒
	printf("document:%d\n", doc_num);
	printf("words:%d\n", words_num);
	printf("time: %f\n", time_use/1e6);
	
	index_mem_use = (sizeof(struct Node)+32
					+element_length_in_bytes(Tw[0].C2)+element_length_in_bytes(Tw[0].C3))*node_num_Tw
					+(sizeof(struct Node)+32
					+element_length_in_bytes(Td[0].C2)+element_length_in_bytes(Td[0].C3))*node_num_Td;
					
	printf("memory: %fM\n", ((double)index_mem_use)/1024/1024);
    printf("index construction complete...\n");
    
    //printT(Tw, Lw);
    //printf("Td: \n");
    //printT(Td, Ld);
	//Dec(buff, Tw, 0);
	//print_hex(buff, 32);
/*
	
	Dec(buff, Tw, 0);
    print_hex(buff, 32);
	*/
	while(1)
	{	
		fgets(input, 50, stdin);
        input[strlen(input)-1] = '\0';
		gettimeofday(&start, NULL);
		
		n = Search(input, strlen(input));
		
		gettimeofday(&end, NULL);
		time_use = (end.tv_sec-start.tv_sec)*1000000 + (end.tv_usec-start.tv_usec);//微秒
		printf("time_used:%f, doc_num:%d\n", time_use/1e6, n);
		
	}
}

