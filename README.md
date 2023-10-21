# Real-Time Webcam Pose Estimation using OpenCV, TFLite, and MoveNet
This is a minimal C++ project for performing real-time pose estimation on your webcam based on OpenCV, TFLite and MoveNet. It's purpose is not to be useful but to get started with OpenCV and TFLite in C++.

## Features
- Webcam capture and preview using OpenCV
- Choose between [MoveNet SinglePose](https://tfhub.dev/google/lite-model/movenet/singlepose/lightning/tflite/float16/4) and [MoveNet Multipose](https://tfhub.dev/google/movenet/multipose/lightning/1)
- Select input size for multi-pose model (speed vs. accuracy)
- FPS counter

## Installation

### Prerequisites
Requirements:
- Python 3.9+
- Conan 2.0.4+
- CMake
- PreMake
- Visual Studio 2022 (for other IDEs, adjust the `premake5` command in `init.py`)

**Installing Conan:** You might want to use an Anaconda environment if your global Python installation is messed up. Make sure that you are always in the same Python environment when using Conan.
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
**Install CMake** if not already installed (check with `cmake --version`). For Windows, download the installer from https://cmake.org/download/.

**Install Premake:** Download Premake from https://premake.github.io/download and put the `premake5.exe` into the root folder or your PATH. You don't need the other files from the downloaded zip archive. Check premake5 is accessible from your console by typing `premake5 --version`.

### Method 1: Auto-install repository (recommended)
To use the auto-install script, run:
```
python init.py
```
Make sure to use the same Python environment you installed Conan in! This usually takes a while (> 20 min), mostly because of the debug build.

You can open the generated `WebcamPoseEstimation.sln` file in Visual Studio 2022.

### Method 2: Manual installation
#### 1. Install packages using Conan:
Run **both** commands, one for the Release and one for the Debug setting:
```bash
conan install . --build missing --output-folder=./dependencies
conan install . --build missing --output-folder=./dependencies --settings=build_type=Debug
```
As a result, a lot of things should happen in the console and the dependencies folder should be generated and populated with `.lua` and `.bat` files. Note that the debug command usually takes a lot longer to build on first execution.

Possible reasons for failure:
- CMake is not installed or too old -> check if that is the case using `cmake --version` and if applicable, install CMake for your system (Windows: google and download the installer).
- You used an Anaconda Python to install Conan but are not in an Anaconda environment -> usually no response. Make sure your Anaconda environment is active.

#### 2. Build a Visual Studio 2022 project:
```bash
premake5 vs2022
```
This generates the `WebcamPoseEstimation.sln` file, which can be opened in Visual Studio 2022. The command is safe to repeat. It should particularly be repeated after any premake5/lua files were updated.

### Acknowledgements
- The Conan setup is based on Lötwig Fusel's video: https://www.youtube.com/watch?v=7sLeMVUo8Kg.
- Source code is inspired by https://github.com/conan-io/examples2/tree/main/examples/libraries/tensorflow-lite/pose-estimation
