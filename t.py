max_num = 0
i = 0
with open('index_1w', 'r') as f:
	for line in f:
		i += 1
		cur_num = len(line.strip('\n').split())-1
		if cur_num > max_num:
			max_num = cur_num
print(max_num, i)
		