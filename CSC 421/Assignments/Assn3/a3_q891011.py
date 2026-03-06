# YOUR CODE GOES HERE 
import os
import numpy as np

def word_probabilities(directory: str): 
    dataset = np.array(["awful", "bad", "boring", "dull", "effective", "enjoyable", "great", "hilarious", "adoxography"])
    Nwc = np.zeros(len(dataset))
    Nc = 0
    alpha = 1

    for filename in os.scandir(directory):
        if filename.is_file():
            with open(filename.path, "r") as f:
                contents = f.read().lower()
            Nwc += np.array([1 if word in contents else 0 for word in dataset]) # binary vector
            Nc += 1

    laplace_smoothing = (Nwc + alpha) / (Nc + len(dataset) * alpha)
    return laplace_smoothing

def likelihood(path, probs): 
    dataset = np.array(["awful", "bad", "boring", "dull", "effective", "enjoyable", "great", "hilarious", "adoxography"])
    with open(path, "r") as f:
        contents = f.read().lower()
    bin_feature_vector = np.array([1 if word in contents else 0 for word in dataset]) # binary vector
    return np.prod(probs ** bin_feature_vector * (1 - probs) ** (1 - bin_feature_vector)) # p(x|Ck)

def class_priors(class_dir_1, class_dir_2): 
    len_of_C1 = len([filename for filename in os.scandir(class_dir_1) if filename.is_file()])
    len_of_C2 = len([filename for filename in os.scandir(class_dir_2) if filename.is_file()])
    p_of_C1 = len_of_C1 / (len_of_C1 + len_of_C2)
    p_of_C2 = len_of_C2 / (len_of_C1 + len_of_C2)
    return [p_of_C1, p_of_C2]

def predict(path, neg_probs, pos_probs, neg_prior, pos_prior):
    if neg_prior * likelihood(path, neg_probs) < pos_prior * likelihood(path, pos_probs):
        return 'positive'
    return 'negative'

def accuracy(neg_dir, pos_dir): 
    neg_prior, pos_prior = class_priors(neg_dir, pos_dir) 
    neg_probs = word_probabilities(neg_dir)
    pos_probs = word_probabilities(pos_dir)

    tp = tn = fp = fn = 0

    for filename in os.scandir(neg_dir):
        if filename.is_file():
            if predict(filename.path, neg_probs, pos_probs, neg_prior, pos_prior) == 'negative':
                tn += 1
            else:
                fp += 1

    for filename in os.scandir(pos_dir):
        if filename.is_file():
            if predict(filename.path, neg_probs, pos_probs, neg_prior, pos_prior) == 'positive':
                tp += 1
            else:
                fn += 1
    return (tp + tn) / (tp + tn + fp + fn)    

# neg_probs = word_probabilities('review_polarity/txt_sentoken/neg')
# pos_probs = word_probabilities('review_polarity/txt_sentoken/pos')
# neg_prior = 0.5
# pos_prior = 0.5 

# # Some testcases with correct and incorrect classifications
# path = 'review_polarity/txt_sentoken/pos/cv996_11592.txt'
# print(path, 'classified as', predict(path, neg_probs, pos_probs, neg_prior, pos_prior))

# path = 'review_polarity/txt_sentoken/pos/cv000_29590.txt'
# print(path, 'classified as', predict(path, neg_probs, pos_probs, neg_prior, pos_prior))

# path = 'review_polarity/txt_sentoken/neg/cv001_19502.txt'
# print(path, 'classified as', predict(path, neg_probs, pos_probs, neg_prior, pos_prior))

# path = 'review_polarity/txt_sentoken/neg/cv000_29416.txt'
# print(path, 'classified as', predict(path, neg_probs, pos_probs, neg_prior, pos_prior))

# print("Overall Accuracy: ", accuracy('review_polarity/txt_sentoken/neg', 'review_polarity/txt_sentoken/pos') * 100., "%")

# Expected Output
# review_polarity/txt_sentoken/pos/cv996_11592.txt classified as positive
# review_polarity/txt_sentoken/pos/cv000_29590.txt classified as negative
# review_polarity/txt_sentoken/neg/cv001_19502.txt classified as positive
# review_polarity/txt_sentoken/neg/cv000_29416.txt classified as negative
# Overall Accuracy:  67.05 %