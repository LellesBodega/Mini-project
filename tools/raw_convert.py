# Jacob

from PIL import Image
import numpy as np

# Öppna bilden, konvertera till gråskala och ändra storlek
img = Image.open("OG cat.png").convert("L").resize((256, 256))

# Konvertera till numpy-array (256x256)
data = np.array(img, dtype=np.uint8)

# Skriv direkt till rådatafil
data.tofile("OG cat.raw")

print(f"Skapade cat.raw ({data.shape[0]}x{data.shape[1]} pixels, gråskala)")
