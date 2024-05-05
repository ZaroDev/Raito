import cv2
import numpy as np
from datetime import datetime
import os
import imagehash
from PIL import Image

CYCLES_IMG_PATH = 'data/cycles/'
RAITO_IMG_PATH = 'data/raito/'

class ImageC:
    def __init__(self, image, path):
        self.image = image
        self.path = path

def mse(img1, img2):
   diff = cv2.absdiff(img1.image, img2.image)
   hash0 = imagehash.average_hash(Image.open(img1.path))
   hash1 = imagehash.average_hash(Image.open(img2.path))
   mse = hash0 - hash1 
   return mse, diff

def load_images(path):
    images = []
    for img_name in os.listdir(path):
        img = cv2.imread(path + img_name)
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        images.append(ImageC(img, path + img_name))
    return images


raito_images = load_images(RAITO_IMG_PATH)
cycles_images = load_images(CYCLES_IMG_PATH)
errors = []

dt_string = datetime.now().strftime("%d-%m-%Y_%H-%M-%S")

report_path = "results/" + dt_string

for i in range(len(raito_images)):
    c_img = cycles_images[i]
    r_img = raito_images[i]
    error, res = mse(c_img, r_img)
    errors.append("Image comparison number " + str(i) + " error: " + str(error))
    cv2.imwrite(report_path + "_img_" + str(i) + ".png", res)
        

report = open(report_path + ".txt", "x")
for err in errors:
    report.write(err)
report.close()



