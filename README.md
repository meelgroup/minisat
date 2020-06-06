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
ln -s ../crystalball/* .
ln -s ../build_scripts/* .

# Let's get an unsatisfiable CNF
wget https://www.msoos.org/largefiles/goldb-heqc-i10mul.cnf.gz
gunzip goldb-heqc-i10mul.cnf.gz

# Gather the data, denormalize, label, output CSV,
# create the classifier, generate C++,
# and build the final SAT solver
./ballofcrystal.sh --csv goldb-heqc-i10mul.cnf
[...compilations and the full data pipeline...]
```


### Adding a feature
Want to extend it further? Here are the details of adding feature and assess their impact.

## More details
Some more information on building, construction and folder structure is available in the [older README](https://github.com/meelgroup/minisat/blob/master/README).
