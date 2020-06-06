# MiniSat
MiniSat is an "Extensible SAT solver." In this work we extend MiniSat with [CrystalBall](https://github.com/msoos/cryptominisat/tree/crystalball).

## CrystalBall
Read the [paper](https://www.comp.nus.edu.sg/~meel/Papers/sat19skm.pdf) or the [associated blog post](https://www.msoos.org/2019/06/crystalball-sat-solving-data-gathering-and-machine-learning/) for more details. Here are details to build.

### Compiling in Linux
To build and install, issue:
```
git clone https://github.com/meelgroup/minisat
cd minisat
git checkout crystalball
git submodule update --init
mkdir build & cd build
ln -s ../build_scripts/* .
./build_stats.sh
```

### Adding a feature
Want to extend it further? Here are the details of adding feature and assess their impact.

## More details
Some more information on building, construction and folder structure is available in the [older README](https://github.com/meelgroup/minisat/blob/master/README).
