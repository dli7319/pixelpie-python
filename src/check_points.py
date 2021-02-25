import numpy as np
from matplotlib import pyplot as plt


image = np.zeros((4096, 4096))
values = np.fromfile('filename.rps', dtype=np.float32)
values = values.reshape((-1, 2))
print("values", values.shape, values[:, 0].min(), values[:, 1].min())
values = values * 4096
values = values.astype(np.long)
image[values[:, 0], values[:, 1]] = 1.0
plt.imsave("values.png", image)

