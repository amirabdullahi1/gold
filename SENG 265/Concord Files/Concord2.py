#!/usr/bin/python3

import sys

excl = []
idx = []

def excl_or_idx(word):
        if word.lower() in excl:
                return 1
        if word in idx:
                return 1
        return 0

def get_that_print_on(word, line):
        output = word.upper()
        for w in range(len(line)):
                if line[w]==word:
                        right_cols=0
                        for i in range(w-1, -1, -1):
                                if right_cols + 1 + len(line[i])<=20:
                                        output = line[i] + ' ' + output
                                        right_cols += 1 + len(line[i])
                                else:
                                        break
                        output = ' '*(9 + 20-right_cols) + output

                        left_cols=len(word)
                        for i in range(w+1, len(line), 1):
                                if left_cols + 1 + len(line[i])<=31:
                                        output = output + ' ' + line[i]
                                        left_cols += 1 + len(line[i])
                                else:
                                        break
        print(output)

def main():

        lines = sys.stdin.readlines()

        #double checking for correct version
        if '1' in lines[0]:
                print("Input is version 1, concord2.py expected version 2")
                return

        num_excl=0
        #stores exclusion words
        for line in range(2, len(lines), 1):
                if '"' in lines[line]:
                        break
                excl.append(lines[line].lower().rstrip())
                num_excl += 1

        #add words from each lines that arent in excl to idx
        num_idx = 0
        for line in range(3+num_excl, len(lines), 1):
                cur_line = lines[line].rstrip().split()
                for word in cur_line:
                        if excl_or_idx(word) == 0:
                                idx.append(word)
                                num_idx += 1

        #case insensitive alphabeticall sorting of index words
        idx.sort(key = lambda x: x.lower())

        #loops through index words looping through each index line for that word
        for word in range(len(idx)):
                for line in range(3+num_excl, len(lines), 1):
                        cur_line = lines[line].rstrip().split()
                        if idx[word] in cur_line:
                                get_that_print_on(idx[word], cur_line)

if __name__ == "__main__":
        main()
