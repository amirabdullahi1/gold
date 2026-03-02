import numpy as np
from scipy import stats

class Random_Variable: 
    
    def __init__(self, name, values, probability_distribution): 
        self.name = name 
        self.values = values 
        self.probability_distribution = probability_distribution
        if all(type(item) is np.int64 for item in values):
            self.type = 'numeric'
            self.rv = stats.rv_discrete(name = name, values = (values, probability_distribution))
        elif all(type(item) is str for item in values): 
            self.type = 'symbolic'
            self.rv = stats.rv_discrete(name = name, values = (np.arange(len(values)), probability_distribution))
            self.symbolic_values = values 
        else: 
            self.type = 'undefined'
            
    def sample(self,size): 
        if (self.type =='numeric'):
            return self.rv.rvs(size=size)
        elif (self.type == 'symbolic'): 
            numeric_samples = self.rv.rvs(size=size)
            mapped_samples = [self.values[x] for x in numeric_samples]
            return mapped_samples

    def get_name(self):
        return self.name


def dice_war(A,B, num_samples = 1000, output=True):
    # YOUR CODE GOES HERE

    res = prob > 0.5 
    
    if output: 
        if res:
            print('{} beats {} with probability {}'.format(A.get_name(),
                                                           B.get_name(),
                                                           prob))
        else:
            print('{} beats {} with probability {:.2f}'.format(B.get_name(),
                                                               A.get_name(),
                                                               1.0-prob))
    return (res, prob)


# Example: Create two dice from the example above A and B
values = np.arange(1,7,dtype=np.int64)
probabilities_A = np.array([1/6., 1/6., 1/6., 1/6., 1/6., 1/6.])
probabilities_B = np.array([0/6., 0/6., 0/6., 3/6., 3/6., 0/6.])

dieA = Random_Variable('DieA', values, probabilities_A)
dieB = Random_Variable('DieB', values, probabilities_B)

(res, prob)=dice_war(dieA,dieB)



# YOUR CODE GOES HERE 
# Add code here to show the non-transitive nature of Red, Green, and Blue dice 
# Create three dice Red Green Blue


# Your output from this cell should look something like this, NOTE that the numbers will differ because of sampling, but the outcome should be the same
# DieB beats DieA with probability 0.75
# Red beats Green with probability 0.545
# Green beats Blue with probability 0.551
# Blue beats Red with probability 0.58