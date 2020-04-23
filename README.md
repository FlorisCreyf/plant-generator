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

## Growth Model

A plant is grown over a certain number of growth cycles, with the first cycle consisting of a seedling. A bounding box is generated at the beginning of each cycle and is used to construct a dome (the sky). Rays (light) originating on the dome intersect leaves to determine the efficiency of each stem in the plant. A stem is efficient if many intersections occur with its leaves and is grown by a random growth factor. Otherwise, if a stem receives no or little ray intersections, then the stem is shedded (known as Cladoptosis) by the plant. Remaining stems will grow in the average direction of the rays that intersected with its leaves.

Each stem develops buds at the beginning of each iteration. Leaves form along the dormant nodes in the current iteration, and stems grow from the nodes in the following iteration. As stems grow older, a branch collar (swelling) forms at the base to secure the stem to the parent stem. Additionally, growth of lateral stems is suppressed by some factor depending on the distance to the apex of the parent stem. This suppression factor is intended to approximate the effects of Auxin hormones, which determine the apical dominance of the plant.

## TODO

- Branch forking
- Asymmetrical cross sections
- Leaf angle distribution
- Roots
- Light detection
- Outward stem growth
- Tessellation
- Animation
- More shaders
