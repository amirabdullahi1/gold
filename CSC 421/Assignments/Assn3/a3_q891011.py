# YOUR CODE GOES HERE 



# Some testcases with correct and incorrect classifications
path = 'review_polarity/txt_sentoken/pos/cv996_11592.txt'
print(path, 'classified as', predict(path, neg_probs, pos_probs, neg_prior, pos_prior))

path = 'review_polarity/txt_sentoken/pos/cv000_29590.txt'
print(path, 'classified as', predict(path, neg_probs, pos_probs, neg_prior, pos_prior))

path = 'review_polarity/txt_sentoken/neg/cv001_19502.txt'
print(path, 'classified as', predict(path, neg_probs, pos_probs, neg_prior, pos_prior))

path = 'review_polarity/txt_sentoken/neg/cv000_29416.txt'
print(path, 'classified as', predict(path, neg_probs, pos_probs, neg_prior, pos_prior))

print("Overall Accuracy: ", accuracy('review_polarity/txt_sentoken/neg', 'review_polarity/txt_sentoken/pos') * 100., "%")

# Expected Output
# review_polarity/txt_sentoken/pos/cv996_11592.txt classified as positive
# review_polarity/txt_sentoken/pos/cv000_29590.txt classified as negative
# review_polarity/txt_sentoken/neg/cv001_19502.txt classified as positive
# review_polarity/txt_sentoken/neg/cv000_29416.txt classified as negative
# Overall Accuracy:  67.05 %