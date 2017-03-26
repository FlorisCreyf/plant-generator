# TreeMaker — [www.treemaker.xyz](http://www.treemaker.xyz)

TreeMaker facilitates modeling trees by automatically generating geometry of lines and curves. It implements both a recursive (lateral stems) and fractal (dichotomous stems) structure. The project is divided into a generator (`./lib`) and editor component (`./editor`). The generator is licensed under the Apache License (version 2.0). The editor is licensed under the GPL 3.0.

![treemaker](http://www.treemaker.xyz/window.png)

## Instructions

__Mouse controls__
- _Middle mouse button_: Rotate.
- _Middle mouse button + Shift_: Pan.
- _Middle mouse button + Ctrl_: Zoom.
- _Left mouse button_: Move items.
- _Right mouse button_: Select items.

__Key controls__
- _E_: Extrude the active path from the selected point.
- _A_: Add a new lateral stem to the selected stem.

## Installation

```sh
sudo dnf install qt5-devel
cd ~/treemaker
git submodule update --init --recursive
make all
```

## Todo
- Use an improved collision detection algorithm for object picking.
- Implement serialization.
- Implement an undo/redo system.
- Improve seams for dichotomous stems.
- Implement an outline shader.
- Grow stems outward from the trunk.
- Make the stem path length independent from the stem radius.
- Implement a flare component for stems.

