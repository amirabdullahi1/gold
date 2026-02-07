# CSC421 ASSIGNMENT 2 - QUESTION 1

## ----------------------------------------------------------- ##
def evaluate(s: str):
    operator = s[0]
    operands = s[1:]

    if operator == '&':
        if 'Z' in operands:
            return 'Z'
        if 'U' in operands:
            return 'U'
        return 'O'

    if operator == '|':
        if 'O' in operands:
            return 'O'
        if 'U' in operands:
            return 'U'
        return 'Z'
## ----------------------------------------------------------- ##
    

# examples
e1_1 = "& Z O"
e1_2 = "| O O"
e1_3 = "| Z Z"
e1_4 = "& U U"
e1_5 = "& U Z"

res_e1_1 = evaluate(e1_1)
res_e1_2 = evaluate(e1_2)
res_e1_3 = evaluate(e1_3)
res_e1_4 = evaluate(e1_4)
res_e1_5 = evaluate(e1_5)


print(f'{e1_1} = {res_e1_1}')
print(f'{e1_2} = {res_e1_2}')
print(f'{e1_3} = {res_e1_3}')
print(f'{e1_4} = {res_e1_4}')
print(f'{e1_5} = {res_e1_5}')


# CSC421 ASSIGNMENT 2 - QUESTION 2

d = {'foo': "Z", 'b': "O"}
print(d)
e2_1 = '& Z O'
e2_2 = '& foo O'
e2_3 = '& foo b'

## ----------------------------------------------------------- ##
def evaluate_with_bindings(s: str, d: dict):
    for k, v in d.items():
        s = s.replace(k, v)

    operator = s[0]
    operands = s[1:]

    if operator == '&':
        if 'Z' in operands:
            return 'Z'
        if 'U' in operands:
            return 'U'
        return 'O'

    if operator == '|':
        if 'O' in operands:
            return 'O'
        if 'U' in operands:
            return 'U'
        return 'Z'
## ----------------------------------------------------------- ##


res_e2_1 = evaluate_with_bindings(e2_1,d)
res_e2_2 = evaluate_with_bindings(e2_2,d)
res_e2_3 = evaluate_with_bindings(e2_3,d)

print(f'{e2_1} = {res_e2_1}')
print(f'{e2_2} = {res_e2_2}')
print(f'{e2_3} = {res_e2_3}')


# CSC421 ASSIGNMENT 2 - QUESTIONS 3,4

## ----------------------------------------------------------- ##
def recursive_eval(l):
    head, tail = l[0], l[1:]
    if head in ['&', '|']: 
        val1, tail = recursive_eval(tail)
        val2, tail = recursive_eval(tail)
        if head == '&': 
            return (evaluate('&' + val1 + val2), tail)
        elif head == '|':  
            return (evaluate('|' + val1 + val2), tail)
    # operator is a value 
    else:  
        return (head,tail)
    
def prefix_eval(input_str, d):
    for k, v in d.items():
        input_str = input_str.replace(k, v)

    for k, v in {'~ Z': "0", '~ U': "U", '~ O': "Z"}.items():
        input_str = input_str.replace(k, v)

    input_list = input_str.split(' ')
    res, tail = recursive_eval(input_list)
    return res
## ----------------------------------------------------------- ##

d = {'a': 'O', 'b': 'Z', 'c': 'U'}
e3_1 = "& a | Z O"
e3_2 = "& O | O b"
e3_3 = "| O & ~ b b"
e3_4 = "& ~ a & O O"
e3_5 = "| O & ~ b c"
e3_6 = "& ~ a & c O"
e3_7 = "& & c c & c c"

print(d)
for e in [e3_1,e3_2,e3_3,e3_4,e3_5,e3_6, e3_7]:
    print("%s \t = %s" % (e, prefix_eval(e,d)))

# EXPECTED OUTPUT
# & Z O = Z
# | O O = Z
# | Z Z = Z
# {'foo': 'Z', 'b': 'O'}
# & Z O = Z
# & foo O = Z
# & foo b = Z
# {'a': 'O', 'b': 'Z', 'c': 'U'}
# & a | Z O        = O
# & O | O b        = O
# | O & ~ b b      = O
# & ~ a & O O      = Z
# | O & ~ b c      = O
# & ~ a & c O      = Z
# & & c c & c c    = U
