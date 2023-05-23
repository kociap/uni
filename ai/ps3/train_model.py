import tensorflow as tf
from tensorflow import keras
from keras.models import Sequential
from keras.layers import Dense, Dropout, Activation, Flatten, Conv2D, MaxPooling2D

(data_images, data_labels), (test_images, test_labels) = keras.datasets.mnist.load_data()

data_images = keras.utils.normalize(data_images, axis = 1)
test_images = keras.utils.normalize(test_images, axis = 1)

shape = (-1, 28, 28, 1)
data_images.reshape(*shape)
test_images.reshape(*shape)

model = keras.models.load_model("model")

# model = Sequential()

# # Convolution layer
# # 64 is the number of filters. (3, 3) is the size of the kernel.
# model.add(Conv2D(64, (3, 3), activation = "relu", input_shape = shape[1:]))
# model.add(MaxPooling2D(pool_size = (2, 2)))
# model.add(Conv2D(64, (3, 3), activation = "relu", input_shape = shape[1:]))
# model.add(MaxPooling2D(pool_size = (2, 2)))
# model.add(Flatten())
# model.add(Dense(64, activation = "relu"))
# model.add(Dense(32, activation = "relu"))
# # Output layer
# model.add(Dense(10, activation = "softmax"))

# model.compile(optimizer = "adam",
#               loss = "sparse_categorical_crossentropy",
#               metrics = ["accuracy"])

# model.fit(data_images, data_labels, epochs = 6, batch_size = 32, validation_data = (test_images, test_labels))

test_loss, test_accuracy = model.evaluate(test_images, test_labels, verbose = 2)
print(f'loss: {test_loss}; accuracy: {test_accuracy}')

keras.models.save_model(model, "model")
