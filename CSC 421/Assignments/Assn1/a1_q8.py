def isComplete(assignment):
    return None not in (assignment.values())

def select_unassigned_variable(variables, assignment):
    for var in variables:
        if assignment[var] is None:
            return var

def is_consistent(assignment, constraints):
    for constraint_violated in constraints:
        if constraint_violated(assignment):
          return False
    return True

def init_assignment(csp):
    assignment = {}
    for var in csp["VARIABLES"]:
        assignment[var] = None
    return assignment

def blue_assignment(csp):
    assignment = {}
    for var in csp["VARIABLES"]:
        if var == "WA": 
            assignment[var] = "blue"
        else: 
            assignment[var] = None
    return assignment

def add_constraint(csp, constraint): 
    csp['CONSTRAINTS'].append(constraint)
    
def recursive_backtracking(assignment, csp):
    if isComplete(assignment):
        return assignment
    var = select_unassigned_variable(csp["VARIABLES"], assignment)
    for value in csp["DOMAINS"]:
        assignment[var] = value
        if is_consistent(assignment, csp["CONSTRAINTS"]):
            result = recursive_backtracking(assignment, csp)
            if result != "FAILURE":
                return result
        assignment[var] = None
    return "FAILURE"


def binary_constraint(var_pair, violations):
    (v1,v2) = var_pair
    return lambda asmt: (asmt[v1], asmt[v2]) in violations
    
def unary_constraint(var, violations):
    return lambda asmt: (asmt[var]) in violations

csp1 = {"VARIABLES": ["WA", "NT", "Q", "NSW", "V", "SA", "T"],
        "DOMAINS": ["red", "green", "blue"],
        "CONSTRAINTS": []}

violations = {('red','red'), ('green','green'), ('blue','blue')}

for (v1,v2) in [('WA', 'NT'), ('WA', 'SA'),
                ('NT', 'SA'), ('NT', 'Q'),
                ('SA', 'Q'), ('SA', 'NSW'),
                ('SA', 'V'),('Q', 'NSW'),
                ('V', 'T')]:
    add_constraint(csp1, binary_constraint((v1,v2), violations))  

  
# You will need to define appropriately the blue_assignment function to create 
# an initial assignment as well as the unary_costraint function so that the code below works as described 
# in the assignment specification 


result_original = recursive_backtracking(init_assignment(csp1), csp1)    
print(result_original)

result_init = recursive_backtracking(blue_assignment(csp1), csp1)
print(result_init)

add_constraint(csp1, unary_constraint('WA', ['red','green']))
result_unary = recursive_backtracking(init_assignment(csp1), csp1)
print(result_unary) 
