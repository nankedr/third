import math
import sys

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

def forward_to_inverted(input_lines_num = 5000):
    count = input_lines_num
    inverted_index = dict()
    with open('index_test', 'r') as f:
        for line in f:
            if count == 0:
                break
            count -= 1
            
            templist = line.strip('\n').split()
            doc_id = templist[0]
            for word in templist[1:]:
                inverted_index.setdefault(word, 0)
                inverted_index[word] += 1
                
    kvlist = sorted(inverted_index.items(), key=lambda d:d[1])
    for kv in kvlist:
        print(kv)      
        
    num_index = sum(inverted_index.values())
    num_words = len(inverted_index.keys())
    print('number of file: {}'.format(input_lines_num))
    print('number of index: {}, log_2_:{}'.format(num_index, math.log(3*num_index, 2)))
    print('number of words: {}, log_2_:{}'.format(num_words, math.log(3*num_words, 2)))
    
if __name__ == '__main__':
    file_num = 50
    if len(sys.argv) != 1:
        file_num = int(sys.argv[1])
    forward_to_inverted(file_num)
            