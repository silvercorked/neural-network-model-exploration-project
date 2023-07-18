# Neural Network Model Exploration Project
This project aims to explore a neural network model in the interest of explainability.
To do so, this project divides the N dimensional space, where N is the number of input 'features'
of the model, in a 2 dimensional space. The two dimensions correspond to two of the features.
Walking along the domain of the two features (while allowing all other to be randomly set in their domains)
can be used to generate statistics, which can be used graphically. This process is repeated for every
combination of 2 input features.

Input model is converted from tensorflow into another format (frugally-deep) and placed in the /in folder
alongside a list of features, their domains, and some constraints between them.

Output and in-progress files are stored in /out and /out/working respectively.

A simple model trained on the Iris dataset is included in /demoMaterials (in the demoMaterials/in folder)
and the output from this project on that model (in the /demoMaterials/out folder)

## Installation
### wsl/ubnutu
- Install cmake
- Install dependencies (see README in /dependencies)
- Build (make in build folder)

## Use
- Place model (converted from tensorflow format to frugally-deep's format via a python script in /python) json file and features, domains, and constraint json file in /in.
- Set MODEL_PATH and FEATURE_DOMAIN_CONSTRAINT_PATH (additoinal changes currently required beyond this).
- Rebuild if any C++ file code was modified.
