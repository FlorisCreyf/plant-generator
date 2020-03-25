# Plant Generator

Plant Generator generates plant geometry. The project is divided into a generator and editor component, where the generator is licensed under the Apache License (version 2.0) and the editor is licensed under the GPL 3.0. The editor depends on Qt5, but the project can be compiled without a GUI using the Makefile in _plant_generator/plant_generator_. Both components depend on Boost for serialization and parsing command line arguments, but Boost is not a critical component of the program.

## Screenshot

![Plant Generator](https://www.fcreyf.com/static/plant/pg.png)

## Instructions

__Mouse Controls__

- _Middle mouse button:_ Rotate the camera
- _Middle mouse button + Shift:_ Pan the camera
- _Middle mouse button + Ctrl:_ Zoom with the camera
- _Left mouse button:_ Move items or add stems when no points are selected
- _Right mouse button (+ Ctrl):_ Select items in the scene

__Key Controls__

Commands and key bindings can be viewed and edited in _keymap.xml_.

## Installation

```sh
sudo dnf install qt5-devel
sudo dnf install boost-devel
cd ~/plant
git submodule update --init --recursive
make all
```

## TODO

- Branch forking
- Asymmetrical cross sections
- Enforce a maximum radius for stems
- Roots
- Light detection
- Outward stem growth
- Tessellation
- Animation
- More shaders
