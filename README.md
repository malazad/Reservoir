# Reservoir


### Requirements:
  1. Ubuntu 18.04 or above.
  2. The CN(s) should have at least 16GB memory.
  3. Skimage(version 0.16.2)
## To run the prototype:
Step 1: install ndn-cxx(0.7.0-8-gce9c1952) and NFD(0.7.0-7-g51cf75c8) on the routers and the CNs.

Step 2: Set the routing strategy to "/localhost/nfd/strategy/reservoir-strategy/%FD%05" for "/service" and "/cn".

Step 3: Download the demo model from [here](https://unomail-my.sharepoint.com/:u:/g/personal/malazad_unomaha_edu/EXcD8sGqlOdKgFvDYIFO7acBpNp1n1BfxyksPAIiKTqE7g?e=NC8nAg) and put it in the "/Reservoir/Prototype/ProducerApp". Then run the "ProducerApp/reuseProducer.py" on the CN(s).

Step 4: Run the "UserApp/userApp.py" on the user device(s). You may need to compile the "HyperplaneHash.so" to run the "userApp.py". 


Note: to run the prototype with other datasets, first split the dataset using "split_dataset/split_dataset.py" and then put the smaller datasets to each user app directory.

## Simulation: 
Run the "ndnSIM/reuse.cpp". To simplify the simulation, we used pre-processed data. All the pre-processed data is in the "ndnSIM/apps/simulation_data_preprocessed" directory. To select any specific dataset with a specific similarity threshold, change the "dataFilePath" variable in the "reuseConsumer.cpp". 

