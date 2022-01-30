import numpy as np
import cv2
import time

class ImageProcessor():

    def __init__(self):
        self.DataList = []

    def ImageToString(self, path, dimx, dimy):

        img_array_gray = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        img_array = cv2.resize(img_array_gray, (dimx, dimy))

        img_array = img_array.reshape(-1)
        #print(type(img_array))
        #print(img_array.shape)

        #img_array = img_array/255.0
        img_str = ''
        for i in img_array:
            img_str = img_str + str(i) + '-'
        #img_array = np.float32(img_array)
        #img_str = ["-".join(str(item)) for item in img_array]
        #print(img_array)
        return (list(img_array/255.0),img_str[:-1])
    def StringToList(self, String):
        #start = time.time()
        self.DataList = []
        element = ''
        for i in String:
            if i != '-':
                element = element + i
            if i == '-':
                self.DataList.append(float(element))
                element = ''
        #print("time = " + str(time.time() - start))
        return self.DataList
    
    def ImageToStringEncoded(self, path, dimx, dimy):
        img_array_gray = cv2.imread(path, cv2.IMREAD_GRAYSCALE)
        img_array = cv2.resize(img_array_gray, (dimx, dimy))
        image_array_encoded = cv2.imencode('.JPG', img_array)
        img_array_1d = img_array.reshape(-1)
        #print((image_array_encoded[1].tostring().hex()))
        return list(img_array_1d/255.0), (image_array_encoded[1].tostring().hex())

    def EncodedStringToImage(self,EncodedString):

        nparr = np.fromstring(bytes.fromhex(EncodedString), np.uint8)
        img = cv2.imdecode(nparr, cv2.IMREAD_GRAYSCALE)
        img_array_1d = img.reshape(-1)
        cv2.imwrite("fromhexstr.jpg", img)
        return img, img_array_1d
