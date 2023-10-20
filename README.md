# Setup

## Prerequisites: Install & setup Conan package manager:
Requires Python. You might want to use an Anaconda environment if your global Python installation is messed up.
```bash
# if not yet installed
pip install conan
conan --version # should be 2.0.13 (or higher)
conan profile detect
```
Open the profile file in the last line of the output or run `conan profile path default` to see the same file path. Update the following line to 20:
```
compiler.cppstd=20
```
Install CMake if not already installed (check with `cmake --version`).

Download Premake from https://premake.github.io/download and put the `premake5.exe` into the root folder or your PATH. You don't need the other files from the downloaded zip archive.

## Method 1: Auto-install repository (recommended)
To use the auto-install script, run:
```
python init.py
```
Make sure to use the same Python environment you installed Conan in! This usually takes a while (> 20 min), mostly because of the debug build.


## Method 2: Manual installation
### Install packages using Conan:
Run **both** commands, one for the Release and one for the Debug setting:
```bash
conan install . --build missing --output-folder=./dependencies
conan install . --build missing --output-folder=./dependencies --settings=build_type=Debug
```
As a result, a lot of things should happen in the console and the dependencies folder should be generated and populated with `.lua` and `.bat` files. Note that the debug command usually takes a lot longer to build on first execution.

Possible reasons for failure:
- CMake is not installed or too old -> check if that is the case using `cmake --version` and if applicable, install CMake for your system (Windows: google and download the installer).
- You used an Anaconda Python to install Conan but are not in an Anaconda environment -> usually no response. Make sure your Anaconda environment is active.

### Build a Visual Studio 2022 project:
```bash
premake5 vs2022
```
This command is safe to repeat. It should particularly be repeated after any premake5/lua files were updated.

### Problems?
The whole Conan setup is based on https://www.youtube.com/watch?v=7sLeMVUo8Kg.