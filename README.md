# Plant Generator

Plant Generator generates plant geometry. The project is divided into a generator and editor component. The generator is licensed under the Apache License (version 2.0) and the editor is licensed under the GPL 3.0. The editor depends on Qt5, but the project can be compiled without a GUI using the Makefile in _plant_generator/plant_generator_. Both components depend on Boost for serialization and parsing command line arguments, but Boost is not a critical component of the program.

## Screenshot

![Plant Generator](https://www.fcreyf.com/static/plant/pg.png)

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
make all
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

## Growth Model

A plant is grown over a certain number of growth cycles. A bounding box is generated at the beginning of each cycle and is used to construct a dome (the sky). Rays (light) originating on the dome intersect leaves to determine the efficiency of each stem in the plant. A stem is efficient if many intersections occur with its leaves and is grown by a random growth factor. Otherwise, if a stem receives no or little ray intersections, then the stem is shedded by the plant (known as cladoptosis). Remaining stems will grow in the average directions of rays that intersect with their leaves.

Each stem develops buds at the beginning of each iteration. Leaves form along the dormant nodes in the current iteration, and stems grow from the nodes in the following iteration. As stems grow older, a branch collar (swelling) forms at the base to secure the stem to the parent stem. Additionally, growth of lateral stems is suppressed by some factor depending on the distance to the apex of the parent stem. This suppression factor is intended to approximate the effects of auxin hormones that control the apical dominance of the plant.

Auxin (IAA) lengthens cells by increasing cell wall elasticity. Auxin is transported away from light through the shifting of PIN proteins in cell membranes. This process results in stems bending towards light. A common hypothesis is that auxin determines the apical dominance of the plant through a source-sink model. Fully saturating stems with auxin prevents axillary buds from releasing auxin into the stem and prevents further growth. The effects of auxin are countered with cytokinins (CK) that stimulate growth.

Formation of reaction wood is required for stems to grow upward or otherwise stems will bend down as their mass increases. Angiosperms produce tension wood in the upper part of the stem. Gymnosperms produce compression wood in the lower part of the stem. A stem under apical control develops only enough reaction wood to retain its angle but not enough to grow upward. Reaction wood enables one of the axillary stems to become the new central leader if the apical node is removed.

The apical meristem exhibits determinate growth if it produces a flower. The loss of apical control causes sympodial branching. Monopodial branching occurs when the apical meristem exhibits indeterminate growth. Dichotomous branching occurs when the apical node splits in two.

Growth is limited by the nitrogen concentration of the soil. More energy is put into roots when nitrogen is scarce. More stems are grown when nitrogen is abundant.

### Terms

- Phytomer: a unit containing a node, internode, and axillary bud
- Phyllotaxis: the arrangement of leaves
- Acropetal: leaves and flowers start developing at the top
- Basipetal: leaves and flowers start developing at the bottom
- Orthotropic: vertical growth
- Plagiotropic: horizontal growth
- Anisotropic: unequal growth in different directions
- Isotropic: equal growth in all directions
- Excurrent: the plant maintains a single central leader
- Decurrent: lateral stems compete with the central leader
- Acrotony: stems first develop near the apex
- Mesotony: stems first develop in the center
- Basitony: stems first develop at the base
- Inflorescence: a cluster of flowers including the stem

## TODO

- Branch forking
- Asymmetrical cross sections
- Leaf angle distribution
- Roots
- Tessellation
- Animation
- More shaders
