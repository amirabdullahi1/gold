import os
import numpy as np

# YOUR CODE GOES HERE 
def get_feature_vector(path: str):
    dataset = np.array(["awful", "bad", "boring", "dull", "effective", "enjoyable", "great", "hilarious", "adoxography"])

    f = open(path, "r")
    contents = np.array(f.read().split())
    bin_feature_vector = np.isin(dataset, list(contents)).astype(int) # binary vector
    return bin_feature_vector

def word_probabilities(directory: str): 
    dataset = np.array(["awful", "bad", "boring", "dull", "effective", "enjoyable", "great", "hilarious", "adoxography"])
    Nwc = np.zeros(len(dataset))
    Nc = 0
    alpha = 1

    for filename in os.scandir(directory):
        if filename.is_file():
            f = open(filename.path, "r")
            contents = np.array(f.read().split())
            Nwc += np.isin(dataset, list(contents)).astype(int)
            Nc += 1

    laplace_smoothing = (Nwc + alpha) / (Nc + len(dataset) * alpha)
    return laplace_smoothing

# Test cases
example_vec = get_feature_vector('review_polarity/txt_sentoken/pos/cv996_11592.txt')
neg_probs = word_probabilities('review_polarity/txt_sentoken/neg')
pos_probs = word_probabilities('review_polarity/txt_sentoken/pos')

print("Example pos/cv996_11592.txt: ", example_vec)
print("Negative vocabulary probabilities: ", neg_probs)
print("Positive vocabulary probabilities: ", pos_probs)

# Expected output - note that numbers may vary depending on how you parse but it should not be by much
# [0 0 0 0 1 0 1 0 0]
# [0.12190287 0.54112983 0.17443013 0.10109019 0.08622398 0.05450942
#  0.31813677 0.05946482 0.00099108]
# [0.03468781 0.27849356 0.05450942 0.02576809 0.15361744 0.09613479
#  0.48166501 0.13181368 0.00099108]
