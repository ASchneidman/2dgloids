
## Getting Started
Glitter has a single dependency: [cmake](http://www.cmake.org/download/), which is used to generate platform-specific makefiles or project files. Start by cloning this repository, making sure to pass the `--recursive` flag to grab all the dependencies. If you forgot, then you can `git submodule update --init` instead.

```bash
git clone --recursive https://github.com/Polytonic/Glitter
cd Glitter
mkdir Build
cd Build
```

Now generate a project file or makefile for your platform. If you want to use a particular IDE, make sure it is installed; don't forget to set the Start-Up Project in Visual Studio or the Target in Xcode.

```bash
# UNIX/Mac OSX Makefile
cmake ..

# Mac OSX (only if using XCode)
cmake -G "Xcode" ..

# Microsoft Windows
cmake -G "Visual Studio 14" ..
cmake -G "Visual Studio 14 Win64" ..
...
```

Then run 
```
make
```

If you compile and run, you should now be at the same point as the [Hello Window](http://www.learnopengl.com/#!Getting-started/Hello-Window) or [Context Creation](https://open.gl/context) sections of the tutorials. Open [main.cpp](https://github.com/Polytonic/Glitter/blob/master/Glitter/Sources/main.cpp) on your computer and start writing code!
