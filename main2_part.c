#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "globals.h"

extern struct Node Tw[], Td[];
extern int node_num_Tw, node_num_Td;
extern int invalid_label_Td;
extern uint8_t invalid_data[];
extern char strLine[];
extern char word_space[5000][20];
extern uint8_t invalid_data_Td[];

void initT(struct Node T[], int level);
void Enc(struct Node T[], int idx, uint8_t m[]);
int GenLab();
int H2_w_to_set(int* result, const char w[], int len);
void Path(int path[], int L, int l);
void Tw_data_field(uint8_t data[], char w[], int* tag0, int* tag_k1, int* lab0, int* lab_k1);
void Tw_data(uint8_t data[], char w[], int tag0, int tag_k1, int lab0, int lab_k1);
void Td_data(uint8_t data[], int lab_k1, int tag_k2, int lab_k2, int idf);

void Dec_plain(uint8_t m[], struct Node T[], int idx)
{	
	memcpy(m, T[idx].C1, sizeof(T[idx].C1));
}

//C1, C2, C3
void Enc_plain(struct Node T[], int idx, uint8_t m[])
{
	memcpy(T[idx].C1, m, sizeof(T[idx].C1));
}

int AccessTw_plain(uint8_t result[], int l, uint8_t w[], uint8_t new_data[])
{
	int i;
	int path[Lw+1];
	uint8_t data[DEFAULT_BYTES];
	int result_l = -1;
	
	int match = 1;
	Path(path, Lw, l);
	
	for (i=0; i<=Lw; i++)
	{
		Dec_plain(data, Tw, path[i]);
		if (match == 1 && memcmp(w, data, 16) == 0)
		{
			Enc_plain(Tw, path[i], new_data);
			memcpy(result, data, DEFAULT_BYTES);
			result_l = i;
			match = 0;
            break;
		}
	}
	return result_l;
}


//lab||tag_k+1||lab_k+1||id||K||ran
int AccessTd_plain(uint8_t result[], int l, int lab, uint8_t new_data[])
{
	int i;
	int path[Ld+1];
	uint8_t data[DEFAULT_BYTES];
	int result_l = -1;
	
	int match = 1;
	Path(path, Ld, l);
	
	for (i=0; i<=Ld; i++)
	{
		Dec_plain(data, Td, path[i]);
		if (match == 1 && memcmp(data, (uint8_t*)&lab, 4)==0)
		{
			Enc_plain(Td, path[i], new_data);
			memcpy(result, data, DEFAULT_BYTES);
			result_l = i;
			match = 0;
            break;
		}
	}
	return result_l;
}

void Upload_plain(const char w[], int len, int id)
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
		l = AccessTw_plain(data, base+buff[i], temp, invalid_data);
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
	if (AccessTd_plain(new_data, (1<<Ld)-1+tag_k1, invalid_label_Td, new_data) == -1)
	{
		printf("no valid space for Td\n");
		exit(0);
	}
	
	uint8_t data_Tw[DEFAULT_BYTES];
	
	int new_Lw_idx = base + buff[rand()%n];
	Tw_data(data_Tw, w, tag0, tag_k2, lab0, lab_k2);
	while (AccessTw_plain(data, new_Lw_idx, invalid_data, data_Tw) == -1)//loop insert many times in case there are no invalide_data space for data_Tw
	{
		new_Lw_idx = base + buff[rand()%n];
		Tw_data(data_Tw, w, tag0, tag_k2, lab0, lab_k2);
	}
}

void Enc_All_plain()
{
    int i;
    for (i=0; i<node_num_Tw; i++)
	{
	    printf("Tw Enc %d\n", i);
		Enc(Tw, i, Tw[i].C1);
	}
    
    for (i=0; i<node_num_Td; i++)
	{
	    printf("Td Enc %d\n", i);
		Enc(Td, i, Td[i].C1);
	}
}

int read_index_plain(int n)
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
    while ((n--) && !feof(fp))
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
		
            Upload_plain(s, strlen(s), doc_id);
        }
		printf("index:%d\n", doc_id);
    }
    fclose(fp);
    
    Enc_All_plain();
    return doc_num;
}

void initDataTwTd_plain(int len)
{
	int i;
	initT(Tw, Lw);
	initT(Td, Ld);
	
	for (i=0; i<node_num_Td; i++)
	{
	    //printf("%d\n", i);
		Enc_plain(Td, i, invalid_data_Td);
	}
	
	uint8_t temp[DEFAULT_BYTES];
	int buff[DEFAULT_BYTES];
	
	for (i=0; i<len; i++)
	{
		//printf("%s\n", word_space[i]);
		int n = H2_w_to_set(buff, word_space[i], strlen(word_space[i]));
		int Lw_node_idx = (1<<Lw)-1+buff[rand()%n];
		memset(temp, 0, DEFAULT_BYTES);
		memcpy(temp, (uint8_t*)word_space[i], strlen(word_space[i]));	
		while (AccessTw_plain(temp, Lw_node_idx, invalid_data, temp)==-1)
		{
			Lw_node_idx = (1<<Lw)-1+buff[rand()%n];
			//printf("%d\n", Lw_node_idx);
			memset(temp, 0, DEFAULT_BYTES);
			memcpy(temp, (uint8_t*)word_space[i], strlen(word_space[i]));
		}
	}
}
