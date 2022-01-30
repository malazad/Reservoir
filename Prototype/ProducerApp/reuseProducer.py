from typing import Optional
from ndn.app import NDNApp
from ndn.encoding import Name, InterestParam, BinaryStr, FormalName, MetaInfo
import logging
import time
import numpy as np
import cv2
import falconn
import HyperplaneHash
import os
import itertools
import asyncio
from threading import Thread
import queue
from skimage.measure import compare_ssim


from tensorflow import keras
import numpy as np
import os
import matplotlib.pyplot as plt
import itertools
import cv2
from sklearn.utils import shuffle 
from sklearn.metrics import confusion_matrix
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense, Activation, Flatten, Conv2D, MaxPooling2D, Dropout

model = keras.models.load_model('model224')

X_test_path = '14.jpg'
X_img_array_gray = cv2.imread(X_test_path)
X_img_array = cv2.resize(X_img_array_gray, (224, 224))
X_test = np.array([X_img_array])

logging.basicConfig(format='[{asctime}]{levelname}:{message}',
                    datefmt='%Y-%m-%d %H:%M:%S',
                    level=logging.INFO,
                    style='{')

que = queue.Queue()


CNLog = open('CNLog.txt', 'a')
CNLog.write('image,exe\n')
CNLog.close()


root = "mnist_initial_samples/initial_images"
class_names = os.listdir(root)
#print(class_names)
def load_dataset(data_dir):
    images_gray = []
    images = []
    labels = []
    image_path = []
    downsampling_time = 0
    count = 0
    for class_name in class_names:
        path = os.path.join(root, class_name)
        for img in os.listdir(path):
            if img.endswith(".jpg") :
                image_path.append(os.path.join(path, img))
                img_array_gray = cv2.imread(os.path.join(path, img), cv2.IMREAD_GRAYSCALE)

                downsampling_time_start = time.time()
                img_array = cv2.resize(img_array_gray, (50, 50))
                images_gray.append(img_array)

                img_array = img_array.reshape(-1)
                img_array = np.float32(img_array)
                downsampling_time_end = time.time()
                downsampling_time = downsampling_time + (downsampling_time_end - downsampling_time_start)

                images.append(img_array)
                labels.append(class_name)
        count += 1
    images_gray = np.array(images_gray)
    images = np.array(images)
    labels = np.array(labels)
    print("average downsampling = " + str(downsampling_time/labels.shape[0]))
    return (images_gray,images, labels, image_path)


(data_gray, data, data_label, data_image_path) = load_dataset(root)

data = data / 255.0
print('number of training images = '+ str(data_label.shape[0]))


parameters = falconn.LSHConstructionParameters()
parameters.dimension = data.shape[1]
parameters.lsh_family = falconn.LSHFamily.Hyperplane
parameters.distance_function = falconn.DistanceFunction.EuclideanSquared
parameters.l = 1
parameters.num_setup_threads = 1
parameters.num_rotations = 1
parameters.seed = 16
parameters.storage_hash_table = falconn.StorageHashTable.BitPackedFlatHashTable
parameters.k = 30
hp_construct_time_start = time.time()
index = falconn.LSHIndex(parameters)
index.setup(data)
query_object = index.construct_query_object()
query_object.set_num_probes(1)

print("construction is done .........")
new_image_count = 0
app = NDNApp()

@app.route('/service')
@app.route('/cn/1') # For CN 2 '/cn/2', for CN 3 '/cn/3', and so on. 
@app.route('/service1')
def on_interest(name: FormalName, param: InterestParam, _app_param: Optional[BinaryStr]):
    global data, query_object, data_label
    global new_image_count
    CNLog = open('CNLog.txt', 'a')
    print(f'>> I: {Name.to_str(name)}')

    start = time.time()
    hash = int(param.forwarding_hint[1][1][1].tobytes().decode('utf8')[2:])
    image_data = (param.forwarding_hint[1][1][2].tobytes()[4:]).decode('utf8')
    content = param.forwarding_hint[1][1][3].tobytes().decode('utf8')[2:].encode()
    incoming_image_name = param.forwarding_hint[1][1][4].tobytes().decode('utf8')[2:]
    
    nparr = np.fromstring(bytes.fromhex(image_data), np.uint8)
    img = cv2.imdecode(nparr, cv2.IMREAD_GRAYSCALE)
    img = cv2.resize(img, (50, 50))
    #print("converstion time = " + str(time.time() - start))
    img_array_1d = img.reshape(-1)
    img_array_1d = img_array_1d/255.0
    img_array_1d = np.float32(img_array_1d)

    match = query_object.find_nearest_neighbor(img_array_1d)
    print("match= " + str((match)))
    
    #print("forwarding hint = " + str(fh.tobytes()))
    #print(_app_param.tobytes())
    #time.sleep(5)
    #print("converstion time 2= " + str(time.time() - start))
    
    #print(MetaInfo(freshness_period=10000))
    #print(f'Content: (size: {len(content)})')
    foundMatch = False
    if match == -1:
        print("Match is : -1")
        data = np.concatenate((data,np.array([img_array_1d])))
        data_label = np.concatenate((data_label, np.array([content])))
        start_pred = time.time()
        Y_predict = model.predict(X_test)
        print("Prediction time : " + str(time.time() - start_pred))
        new_image_count = new_image_count + 1
    else:
        ssim = compare_ssim(img_array_1d.reshape(-1, 50), data[match].reshape(-1, 50), full=False)
        if ssim >= 0.6: # similarity threshold
            foundMatch = True
            print("match found : " + str(ssim)  )
            content = data_label[match].encode()
        else:
            print("similarity is not enough : " + str(ssim))
            data = np.concatenate((data,np.array([img_array_1d])))
            data_label = np.concatenate((data_label, np.array([content])))
            start_pred = time.time()
            Y_predict = model.predict(X_test)
            print("Prediction time : " + str(time.time() - start_pred))
            new_image_count = new_image_count + 1
            print("new_image_count : " + str(new_image_count))
    app.put_data(name, content=content, freshness_period=10000)
    print(f'<< D: {Name.to_str(name)}')
    #print("data shape : " + str(data.shape) + " label shape = " + str(data_label.shape))
    print('')
    CNLog.write(incoming_image_name + ',' + str(foundMatch) + '\n')
    CNLog.close()
    if new_image_count == 5:
        query_object = train(data)
        new_image_count = 0
def train(Data):
    global parameters, index
    print("training shape : " +  str(Data.shape))
    index2 = falconn.LSHIndex(parameters)
    index2.setup(Data)
    query_object2 = index2.construct_query_object()

    query_object2.set_num_probes(1)
    print("Training is done.........")
    return query_object2

if __name__ == '__main__':
    app.run_forever()