import pandas as pd
import numpy as np 
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder
from sklearn.metrics import classification_report
from sklearn import datasets, svm
from sklearn import tree
from sklearn.naive_bayes import CategoricalNB
from sklearn.datasets import load_digits
from sklearn.naive_bayes import GaussianNB

# URL of the dataset
# url = "https://archive.ics.uci.edu/ml/machine-learning-databases/mushroom/agaricus-lepiota.data"

# Column names as per the dataset description
columns = [
    "class", "cap-shape", "cap-surface", "cap-color", "bruises", "odor",
    "gill-attachment", "gill-spacing", "gill-size", "gill-color", "stalk-shape",
    "stalk-root", "stalk-surface-above-ring", "stalk-surface-below-ring",
    "stalk-color-above-ring", "stalk-color-below-ring", "veil-type", "veil-color",
    "ring-number", "ring-type", "spore-print-color", "population", "habitat"
]

# Load the dataset directly into memory
# data = pd.read_csv(url, header=None, names=columns)
# data.to_csv('mushroom.csv', index=False)


data = pd.read_csv('mushroom.csv')

# Encode categorical features and labels using LabelEncoder
encoder = LabelEncoder()
for col in data.columns:
    data[col] = encoder.fit_transform(data[col])

    
# Separate features (X) and target (y)
X = data.drop("class", axis=1)  # Features
y = data["class"]              # Target

# Split into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Print shapes to confirm loading and splitting worked
print(f"X_train shape: {X_train.shape}")
print(f"X_test shape: {X_test.shape}")
print(f"y_train shape: {y_train.shape}")
print(f"y_test shape: {y_test.shape}")

# QUESTION 4.1 

# Train the CategoricalNB classifier, predict the test set
# and get the corresponding classification report as a dictionary


# YOUR CODE GOES HERE 
categoricalNB = CategoricalNB()
categoricalNB.fit(X_train, y_train)
y_pred = categoricalNB.predict(X_test)
cl_report = classification_report(y_test, y_pred, output_dict=True)


# Create training and testing datasets with just the cap-shape

# YOUR CODE GOES HERE 
X_cap = data[["cap-shape"]]
y_cap = y         

X_cap_train, X_cap_test, y_cap_train, y_cap_test = train_test_split(X_cap, y_cap, test_size=0.2, random_state=42)

# print(X_train[0:10])
# print(X_train_cap[0:10]) 

# Train the CategoricalNB classifier with just the cap-shape data, predict the test set
# and get the corresponding classification report as a dictionary


# YOUR CODE GOES HERE 
categoricalNB_cap = CategoricalNB()
categoricalNB_cap.fit(X_cap_train, y_cap_train)
y_cap_pred = categoricalNB_cap.predict(X_cap_test)
cl_cap_report = classification_report(y_cap_test, y_cap_pred, output_dict=True)

# Uncomment the lines below when you have the classifiers trained

cb_accuracy = np.round(cl_report["accuracy"],2)
cb_cap_accuracy = np.round(cl_cap_report["accuracy"],2)
cb_drop = np.round(cb_accuracy - cb_cap_accuracy, 2)
# print("Naive Bayes Categorical Accuracy drop: ", cb_accuracy, " - ", cb_cap_accuracy, " = ", cb_drop)


# QUESTION 4.2  


# Repeat the process for the decision tree classifier with max_depth=2 and random_state=0
# for the whole dataset as well as just the cap_shape

# YOUR CODE GOES HERE
dt = tree.DecisionTreeClassifier(max_depth=2, random_state=0)
dt.fit(X_train, y_train)
y_pred = dt.predict(X_test)
cl_report = classification_report(y_test, y_pred, output_dict=True)

dt_cap = tree.DecisionTreeClassifier(max_depth=2, random_state=0)
dt_cap.fit(X_cap_train, y_cap_train)
y_cap_pred = dt_cap.predict(X_cap_test)
cl_cap_report = classification_report(y_cap_test, y_cap_pred, output_dict=True)

# Uncomment the lines below when you have the classifiers trained

dt_accuracy = np.round(cl_report["accuracy"],2)
dt_cap_accuracy = np.round(cl_cap_report["accuracy"],2)
dt_drop = np.round(dt_accuracy - dt_cap_accuracy, 2)
# print("Decision Tree Accuracy drop: ", dt_accuracy, " - ", dt_cap_accuracy, " = ", dt_drop)

