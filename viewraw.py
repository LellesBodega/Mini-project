# Jacob
from PIL import Image
import numpy as np

width, height = 256, 256
data = np.fromfile("cat.raw", dtype=np.uint8)
data = data.reshape((height, width))

img = Image.fromarray(data, 'L')  # 'L' = gråskala
img.show()  # öppnar i standardvisare
