## Installing Dependencies
This follows the [frugally-deep install guide](https://github.com/Dobiasd/frugally-deep/blob/master/INSTALL.md).
- Install in order of FunctionalPlus, Eigen, json, then frugally-deep.
- In each, enter build folder (if there are contents inside the build folder, remove them)
- for each
    - create build folder in project `mkdir -p build && cd build`
    - `sudo cmake ..` from inside that folder
        - when making json, add flag `-DJSON_BuildTests=OFF` to avoid building tests
    - `make && sudo make install`

