# Plant Generator — [www.treemaker.xyz](http://www.treemaker.xyz)

Plant Generator facilitates modeling trees by automatically generating geometry of lines and curves. It implements both a recursive (lateral stems) and fractal (dichotomous stems) structure. The project is divided into a generator and editor component. The generator is licensed under the Apache License (version 2.0). The editor is licensed under the GPL 3.0.

![Plant Generator](http://www.treemaker.xyz/window.png)

## Instructions

__Mouse controls__
- _Middle mouse button:_ rotate
- _Middle mouse button + Shift:_ pan
- _Middle mouse button + Ctrl:_ zoom
- _Left mouse button:_ move items / add stem
- _Right mouse button:_ select items
- _Double left click (curve editor):_ add new control point

__Key controls__
- _E_ Extrude the active path from the selected point.

## Installation

```sh
sudo dnf install qt5-devel
cd ~/plant
git submodule update --init --recursive
make all
```

## Todo
- Implement serialization.
- Implement an undo/redo system.
- Improve seams for stems.
- Implement an outline shader.
- Implement a flare component for stems.
