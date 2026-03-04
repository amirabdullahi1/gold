# Modify this code as needed 

import numpy as np
from scipy import stats
from collections import defaultdict, Counter


rng = np.random.default_rng(10)


class Random_Variable: 
    
    def __init__(self, name, values, probability_distribution): 
        self.name = name 
        self.values = values 
        self.probability_distribution = probability_distribution
        if all(issubclass(type(item), np.integer) for item in values):
            self.type = 'numeric'
            self.rv = stats.rv_discrete(name = name, values = (values, probability_distribution), seed=rng)
        elif all(type(item) is str for item in values): 
            self.type = 'symbolic'
            self.rv = stats.rv_discrete(name = name, values = (np.arange(len(values)), probability_distribution), seed=rng)
            self.symbolic_values = values 
        else: 
            self.type = 'undefined'

    def __str__(self):
        sres = "values: " + str(self.values) + "\n" + "probs :"  + str(self.probability_distribution)
        return sres
    
    def sample(self,size): 
        if (self.type =='numeric'):
            return self.rv.rvs(size=size)
        elif (self.type == 'symbolic'): 
            numeric_samples = self.rv.rvs(size=size)
            mapped_samples = [self.values[x] for x in numeric_samples]
            return mapped_samples

    def get_name(self):
        return self.name

    def get_values(self):
        return self.values

    def get_probability_distribution(self):
        return self.probability_distribution  


vals_coin = np.array([1,2])
probs_coin  = np.array([0.5, 0.5])
coin = Random_Variable("coin", vals_coin, probs_coin)
vals_die = np.array([1,2,3,4,5,6])
probs_die = np.array([1/6., 1/6., 1/6., 1/6., 1/6., 1/6.])
die1 = Random_Variable("die1", vals_die, probs_die)
die2 = Random_Variable("die2", vals_die, probs_die)


# APPROXIMATE INFERENCE
# Calculate using approximate inference the values and associated probabilities
# for the random variable CD = C * (D1 + D2) where C is a coin random variable
# and D1 and D2 are die random variables. Do so by generating 10000 samples
# of each random variable calculating the corresponding 10000 values of CD
# and then using frequency counting to estimate the values and probabilities
# of the coin-dice random variable.


# YOUR CODE GOES HERE 


# Uncomment the following lines once you have approximate inference working
# The approximate_vals_coin_dice and approximate_probs_coin_dice variables
# should be np.arrays

#approximate_coin_dice = Random_Variable("approximate_coin_dice", approximate_vals_coin_dice,
#                                        approximate_probs_coin_dice)

# print("APPROXIMATE INFERENCE")
# print(approximate_vals_coin_dice)
# print(approximate_probs_coin_dice) 
# print(approximate_coin_dice)
# print(approximate_coin_dice.sample(20))

# EXACT INFERENCE 
# Calculate using exact inference the values and probabilities of CD.
# You will need to calculate all the possible combinations of values
# and look up the associated probability distributions.
# Write a function rv_op that takes as argument two random variables
# and returns a new random variable that is the result of applying the
# operator f to their values (you will need a sum and prod version)
# There are different ways of implementing this function. I suggest
# you use a default dictionary with keys the values of the resulting
# random variable and as dictionary value the associated probaibities.  


# YOUR CODE GOES HERE 
def rv_op(rv1, rv2, f):
    pass


# Uncomment these lines once you have a functionining rv_op 
# dice = rv_op(die1, die2, lambda a,b:a+b)
# coin_dice = rv_op(coin, dice, lambda a,b:a*b)



print("EXACT INFERENCE")

# vals_coin_dice = coin_dice.get_values()
# probs_coin_dice = coin_dice.get_probability_distribution()
# print(vals_coin_dice)
# print(probs_coin_dice)
# print(coin_dice)
# print(coin_dice.sample(20))


