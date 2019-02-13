import math

def max_words_num_per_line():
    max_num = 0
    i = 0
    with open('index_1w', 'r') as f:
        for line in f:
            i += 1
            cur_num = len(line.strip('\n').split())-1
            if cur_num > max_num:
                max_num = cur_num
    print(max_num, i)

def forward_to_inverted():
    inverted_index = dict()
    with open('index_test', 'r') as f:
        for line in f:
            templist = line.strip('\n').split()
            doc_id = templist[0]
            for word in templist[1:]:
                inverted_index.setdefault(word, 0)
                inverted_index[word] += 1
    num_index = sum(inverted_index.values())
    num_words = len(inverted_index.keys())
    print('number of index: {}, log_2_:{}'.format(num_index, math.log(3*num_index, 2)))
    print('number of words: {}, log_2_:{}'.format(num_words, math.log(3*num_words, 2)))
    
if __name__ == '__main__':
    forward_to_inverted()
            