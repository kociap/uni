import numpy as np

def generate_data(size):
  data = np.random.rand(2, size) * 2 - 1;
  labels = (np.sign(data[0]) == np.sign(data[1])).astype(int)
  return data, labels


def sigmoid(x):
  return 1 / (1 + np.exp(-x))

def sigmoid_derivative(x):
  return sigmoid(x) * (1 - sigmoid(x))

def relu(x):
  return np.maximum(0, x)

def relu_derivative(x):
  return np.where(x <= 0, 0, 1)

class TwoLayerNN:
  def __init__(self, activation, activation_derivative):
    self.activation = activation
    self.activation_derivative = activation_derivative
    # Hidden layer weights
    self.W1 = np.random.randn(4, 2)
    # Hidden layer bias
    self.b1 = np.zeros((4, 1))
    # Output layer weights
    self.W2 = np.random.randn(1, 4)
    # Output layer bias
    self.b2 = np.zeros((1, 1))

  def forward_propagation(self, X):
      self.Z1 = np.dot(self.W1, X) + self.b1
      self.A1 = self.activation(self.Z1)
      self.Z2 = np.dot(self.W2, self.A1) + self.b2
      self.A2 = self.activation(self.Z2)
      return self.A2, self.A1

  def backward_propagation(self, X, Y, learning_rate):
    m = X.shape[1]

    # Gradient for output layer
    dZ2 = self.A2 - Y
    dW2 = np.dot(dZ2, self.A1.T) / m
    db2 = np.sum(dZ2, axis = 1, keepdims = True) / m

    # Gradient for hidden layer
    dZ1 = np.dot(self.W2.T, dZ2) * self.activation_derivative(self.Z1)
    dW1 = np.dot(dZ1, X.T) / m
    db1 = np.sum(dZ1, axis = 1, keepdims = True) / m

    self.W2 -= learning_rate * dW2
    self.b2 -= learning_rate * db2
    self.W1 -= learning_rate * dW1
    self.b1 -= learning_rate * db1

  def train(self, X, Y, learning_rate = 0.01, num_iterations = 1000):
    for i in range(num_iterations):
      predicted, hidden = self.forward_propagation(X)
      self.backward_propagation(X, Y, learning_rate)
      if (i + 1) % 100 == 0:
        loss = self.calculate_loss(predicted, Y)
        print(f"iteration {i + 1}; loss {loss}", flush = True)

  def predict(self, X):
    self.forward_propagation(X)
    predictions = np.where(self.A2 >= 0.5, 1, 0)
    return predictions

  def calculate_loss(self, y_pred, y_true):
    return np.mean(np.square(y_pred - y_true))

def calculate_accuracy(result, verification):
  values, counts = np.unique(result == verification, return_counts = True)
  return counts[1] / (counts[0] + counts[1])

training_data, training_labels = generate_data(50000)

model = TwoLayerNN(relu, relu_derivative)
model.train(training_data, training_labels, learning_rate = 0.7, num_iterations = 2500)

test_data, test_labels = generate_data(10000)

# Unnormalised data
predictions = model.predict(test_data)
unnormalised_accuracy = calculate_accuracy(predictions, test_labels)
print(f"unnormalised accuracy {unnormalised_accuracy}")

# L1 norm data
training_data_L1 = test_data / np.linalg.norm(test_data, ord = 1)
predictions_L1 = model.predict(training_data_L1)
L1_accuracy = calculate_accuracy(predictions_L1, test_labels)
print(f"L1 accuracy {L1_accuracy}")

# L2 norm data
training_data_L2 = test_data / np.linalg.norm(test_data, ord = 2)
predictions_L2 = model.predict(training_data_L2)
L2_accuracy = calculate_accuracy(predictions_L2, test_labels)
print(f"L2 accuracy {L2_accuracy}")

