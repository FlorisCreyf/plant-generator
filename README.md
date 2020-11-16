# Plant Generator

Plant Generator generates plant geometry. The project is divided into a generator and editor component. The generator is licensed under the Apache License (version 2.0) and the editor is licensed under the GPL 3.0. The editor depends on Qt5, but the project can be compiled without a GUI using the Makefile in _plant_generator/plant_generator_. Both components depend on Boost for serialization and parsing command line arguments, but Boost is not a critical component of the program.

## Screenshot

![Plant Generator](https://www.fcreyf.com/static/plant/pg.jpg)

## Design Document

The design document is intended to provide some intuition on how the program works. A compiled LaTeX document can be found at [www.fcreyf.com/static/plant/documentation.pdf](https://www.fcreyf.com/static/plant/documentation.pdf).

```sh
pdflatex --shell-escape documentation.tex
```

## Instructions

### Mouse Controls

- Middle mouse button: Rotate the camera
- Middle mouse button + Shift: Pan the camera
- Middle mouse button + Ctrl: Zoom with the camera
- Left mouse button: Move items or add stems when no points are selected
- Right mouse button (+ Ctrl): Select items in the scene

### Key Controls

Commands and key bindings can be viewed and edited in _keymap.xml_.

## Installation

### Linux

```sh
sudo dnf install qt5-devel
sudo dnf install boost-devel
cd ~/plant
git submodule update --init --recursive
make release
```

### Windows (VS)

1. Download Visual Studio, Qt VS Tools, Qt5, and Boost.
2. Move the Boost directory to "C:\\Program Files\\boost" and/or modify the Qt project file.
3. Use Qt VS Tools to generate a VS project.
4. Build the project.
5. Copy files into the debug/release directory.
    - keymap.xml
    - resources
    - shaders
    - Qt5Core.dll
    - Qt5Gui.dll
    - Qt5Widgets.dll
    - Qt5OpenGL.dll
    - Qt5Xml.dll
    - platforms/qwindows.dll
