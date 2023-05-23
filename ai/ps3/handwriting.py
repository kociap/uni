import tensorflow as tf
from tensorflow import keras
import numpy as np
import os
import cv2

model = keras.models.load_model("sigmoid")

images = []
labels = []
handwriting_path = "./handwriting"
for file_name in os.listdir(handwriting_path):
    image_path = os.path.join(handwriting_path, file_name)
    image = cv2.imread(image_path, cv2.IMREAD_GRAYSCALE)

    images.append(image)

    label = int(file_name[0])
    labels.append(label)

images = np.array(images)
labels = np.array(labels)
images = keras.utils.normalize(images, axis = 1)
images = images.reshape(-1, 28, 28, 1)

test_loss, test_accuracy = model.evaluate(images, labels, verbose = 2)
print(f'loss: {test_loss}; accuracy: {test_accuracy}')
