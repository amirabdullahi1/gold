import sys
import re

class concord:

        #intializes concord item
        def __init__(self, input=None, output=None):
                self.input = input
                self.output = output

                if self.input != None:
                        foutput = self.full_concordance()
                        with open(self.output, 'w') as fp:
                                [fp.write("%s\n" % line) for line in foutput]
                        fp.close()

        #establishes lines from stdin or file
        def __est(self):

                if self.input == None:
                        return sys.stdin.read().splitlines()

                finput = open(self.input, "r")
                lines = finput.read().splitlines()
                finput.close()
                return lines

        #Adds and sorts words from lines which have not been previously stored or excluded
        def __sto_idx(self, not_sto, lines):

                idx = []
                [idx.append(word) for line in lines for word in line.split() if word.lower() not in not_sto and word not in idx]
                idx.sort(key = lambda word: word.lower())

                return idx

        #strips left side of string
        def __l_strip(self, string, strip_len):

                for char in string:

                        if string.startswith(' ') and strip_len<22:
                                return ' '*(29-strip_len) + string

                        strip_len-=1
                        string = string[1:]

        #strips right side of string
        def __r_strip(self, string, strip_len):

                for char in string:

                        if string.endswith(' ') and strip_len<33:
                                return string.rstrip()

                        strip_len-=1
                        string = string[:-1]

        #configures lines with required alignment and index word capitlized
        def __config(self, key, line, res):

                loc = re.search(r"\b" + key + r"\b", line)

                if not loc:
                        return

                l_len = loc.start()
                r_len = len(line)-loc.start()

                if l_len < 21:
                        line = ' '*(29-l_len) + line
                if l_len >20:
                        line = self.__l_strip(line, l_len)
                if r_len > 31:
                        line = str(self.__r_strip(line,r_len))
                else:
                        line = line.rstrip()

                res.append(re.sub(r"\b" + key + r"\b", key.upper(), line))

                return

        #fully concords
        def full_concordance(self):

                lines = self.__est()

                if len(lines)==0 or "1" in lines[0]:
                        return []

                delim = lines.index('""""')
                excl = []
                [excl.append(lines[line].lower()) for line in range(2, delim)]
                incl = lines[delim+1:]
                idx = self.__sto_idx(excl,incl)

                output = []
                [self.__config(word, line, output) for word in idx for line in incl]

                return output
