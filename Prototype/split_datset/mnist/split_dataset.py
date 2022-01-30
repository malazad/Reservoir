import os, shutil
import random

root = "train/"
os.makedirs("user00/train/")
os.makedirs("user11/train/")
class_names = os.listdir(root)

image_path  = []
image_class = []
for class_name in class_names:
    path = os.path.join(root, class_name)
    for img in os.listdir(path):
        if img.endswith(".jpg") :
            image_class.append(class_name)
            image_path.append(os.path.join(path, img))
'''
j = 7
print("img : " + image_path[j] + "  class : " + image_class[j])


j = 10
print("img : " + image_path[j] + "  class : " + image_class[j])

'''
random.seed(1)
for i in range(len(image_path)):
    j = random.randint(0,1)
    shutil.copy(image_path[i], "user" + str(j) + "/" + image_path[i])
    #print("img : " + image_path[j] + "  class : " + image_class[j])

