words = dict()
with open('index_test_1', 'r') as f:
	for line in f:
		word_list = line.strip('\n').split(' ')
		for word in word_list[1:]:
			words.setdefault(word, 0)
			words[word] += 1

print(len(words))

for word in words:
	if len(word)>15:
		print(word)

with open('word_space', 'w') as f:
	f.write('\n'.join(words.keys()))