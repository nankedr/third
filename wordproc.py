line_num = 0
words = dict()
with open('index_1w', 'r') as f:
	for line in f:
		line_num += 1
		word_list = line.strip('\n').split(' ')
		for word in word_list[1:]:
			words.setdefault(word, 0)
			words[word] += 1

print('number of index: {}\nnumber of words: {}'.format(line_num, len(words)))

# for word in words:
	# if len(word)>15:
		# print(word)

# with open('word_space_1w', 'w') as f:
	# f.write('\n'.join(words.keys())+'\n')

kvlist = sorted(words.items(), key=lambda d:d[1])
for kv in kvlist:
	print(kv)