from ndn.app import NDNApp
from ndn.types import InterestCanceled, InterestNack, InterestTimeout, ValidationFailure
from ndn.encoding import Name, Interest
import time
import HyperplaneHash
from imageprocessor import ImageProcessor
import numpy as np 
import cv2
import os, re
from datetime import datetime

root = "user0/train/"
class_names = os.listdir(root)

image_path  = []
image_class = []
for class_name in class_names:
    path = os.path.join(root, class_name)
    for img in os.listdir(path):
        if img.endswith(".png") :
            image_class.append(class_name)
            image_path.append(os.path.join(path, img))
j = 7
print("img : " + image_path[j] + "  class : " + image_class[j] + "  token : " + image_path[j].replace('/', '_'))


j = 10
print("img : " + image_path[j] + "  class : " + image_class[j])

app = NDNApp()

_file = open("user_log.txt", 'a')
_file.write('name,acc,latency\n')

async def main():
    
    #params = b"This is a parameter"
    for _image in range(len(image_path)):


        try:
            sendingTimeStamp = time.time()

            test = ImageProcessor()
            imagePath = image_path[_image]
            imageclass = image_class[_image]
            dataList, dataString = test.ImageToStringEncoded(imagePath, 50, 50)
            hash = HyperplaneHash.GetHash(dataList,32,1,16)
            #hash[0] = 2
            img_array_gray =  cv2.imread(imagePath, cv2.IMREAD_GRAYSCALE)
            img_array = cv2.resize(img_array_gray, (50, 50))
            bin = str(int(img_array_gray.mean()))
            print("hash = " + str(hash[0]) + "  mean : " + bin)
            name = Name.from_str('/service/1/' + str(hash[0] )+ "/" +  bin)
            #print("string len = " + str((dataString)))
            #test.EncodedStringToImage(dataString)
            dt = datetime.now()
            
            print("Sending interest :   " + Name.to_str(name) + " time stamp : " + str(time.time() * 1000) )
            data_name, meta_info, content = await app.express_interest(name, must_be_fresh=False, no_signature=True, need_final_name=False,
                                                                    #app_param=params, 
                                                                    lifetime=100000, can_be_prefix = False,
                                                                    digest_sha256 = False,
                                                                    #app_param= bytes.fromhex(dataString),
                                                                    forwarding_hint = [[0,Name.from_str("/service1/" )  ], [10,Name.from_str("/data/" + str(hash[0]) + '/' + dataString + '/' + imageclass + '/' + imagePath.replace('/', '_') )]],
                                                                    )
        
            print(f'Received Data Name: {Name.to_str(data_name)}')
            incoming_class = str((bytes(content) if content else None))
            incoming_class = incoming_class[2:-1]
            print("incoming_class : " + incoming_class)
            acc = 0
            if imageclass == incoming_class:
                acc = 1
                print("Acc : 1")
            RTT = time.time() - sendingTimeStamp
            print("Latency = " + str(RTT))
            _file.write(imagePath.replace('/', '_') + ',' + str(acc) + ',' + str(RTT) + '\n')
            break
        except InterestNack as e:
            print(f'Nacked with reason={e.reason}')
        except InterestTimeout:
            print(f'Timeout')
        except InterestCanceled:
            print(f'Canceled')
        except ValidationFailure:
            print(f'Data failed to validate')


    app.shutdown()


if __name__ == '__main__':
    app.run_forever(after_start=main())