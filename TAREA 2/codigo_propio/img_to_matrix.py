import sys
from PIL import Image
import numpy as np

# === USER CONFIGURABLE PARAMETERS ===
TARGET_WIDTH = 32
TARGET_HEIGHT = 32
BW_THRESHOLD = 128  # Ajusta este valor para cambiar el umbral de blanco y negro (0-255)

def image_to_bw_matrix(image_path, width=TARGET_WIDTH, height=TARGET_HEIGHT, threshold=BW_THRESHOLD):
    img = Image.open(image_path).convert('L')
    img = img.resize((width, height), Image.Resampling.LANCZOS)
    mat = np.array(img)
    # Convertir a blanco y negro: 0 (negro) si < threshold, 1 (blanco) si >= threshold
    bw_mat = (mat >= threshold).astype(int)
    return bw_mat

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('Usage: python img_to_bw_matrix.py <image.png>')
        sys.exit(1)
    image_path = sys.argv[1]
    matrix = image_to_bw_matrix(image_path)
    np.set_printoptions(threshold=np.inf)
    np.savetxt(sys.stdout, matrix, fmt='%d', delimiter=',')