
## Getting Started
2dgloids depends on OpenGL 4.1 and above, OpenMP, ISPC, and cmake. Start by cloning this repository, making sure to pass the `--recursive` flag to grab all the dependencies. If you forgot, then you can `git submodule update --init` instead.

Each branch contains a different accelerating implementation. For all branches to build, run 

```
mkdir build
cd build
cmake ..
make
```

To run after buiding from your build directory, run

```
./Glitter/Glitter
```

The boid controls are:

W: increase maximum velocity
S: decrease maximum velocity

Up arrow: increase boid view distance
Down arrow: decrease boid view distance

1: increase collision avoidance weight
2: decrease collision avoidance weight

3: increase alignment weight
4: decrease alignment weight

5: increase position weight
6: decrease position weight
