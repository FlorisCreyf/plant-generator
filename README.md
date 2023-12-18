# Plant Generator

Plant Generator generates plant geometry. The project is divided into a generator and editor component. The generator is licensed under the Apache License (version 2.0) and the editor is licensed under the GPL 3.0. The editor depends on Qt6, but the project can be partially built without a GUI. Both components depend on Boost for serialization and parsing command line arguments, but Boost is not a critical component of the program.

## Design Document

The design document is intended to provide some intuition on how the program works.

```sh
pdflatex --shell-escape documentation.tex
```

## Instructions

### Mouse Controls

- Middle mouse button: Rotate the camera
- Middle mouse button + Shift: Pan the camera
- Middle mouse button + Ctrl: Zoom with the camera
- Left mouse button: Move points
- Right mouse button (+ Ctrl): Select stems and points
- Scroll + Shift: Change the value for all nodes

### Key Controls

Commands and key bindings can be viewed and edited in _keymap.xml_.

## Installation

### Linux

```sh
sudo dnf install qt6-devel boost-devel
cd ~/plant-generator
cmake -S . -B release -DCMAKE_BUILD_TYPE=RELEASE
cd release
make
```

### Windows

1. Download and install Visual Studio, Qt6, and Boost.
2. Set environment variables to point to the locations of Qt and Boost. (System -> Advanced System Settings)
    - Qt6_ROOT
    - Boost_ROOT
3. Open the project directory in Visual Studio and build the project.
