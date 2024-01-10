One-dimensional Midpoint Displacement UGen with oversampling
Based on [OversampledOscillators repo](https://github.com/spluta/OversamplingOscillators) by Sam Pluta, which uses the VariableOversampling class from Jatin Chowdhury's ChowDSP library.

```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSC_PATH=<PATH TO SC SOURCE> 
cmake --build . --config Release
```