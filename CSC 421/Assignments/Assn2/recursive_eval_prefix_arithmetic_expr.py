def recursive_eval(l):
    head, tail = l[0], l[1:]
    if head in ['+', '*']: 
        val1, tail = recursive_eval(tail)
        val2, tail = recursive_eval(tail)
        if head == '+': 
            return (int(val1)+int(val2), tail)
        elif head == '*':  
            return (int(val1)*int(val2), tail)
    # operator is a number 
    else:  
        return (int(head),tail)

def prefix_eval(input_str): 
    input_list = input_str.split(' ')
    res, tail = recursive_eval(input_list)
    return res

print(prefix_eval('1'))
print(prefix_eval('+ 1 2'))
print(prefix_eval('+ 1 * 2 3'))
print(prefix_eval('+ * 5 2 * 3 + 1 5'))