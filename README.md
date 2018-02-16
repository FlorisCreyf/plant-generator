# Plant Generator — [www.treemaker.xyz](http://www.treemaker.xyz)

Plant Generator facilitates modeling trees by automatically generating geometry of lines and curves. It implements both a recursive (lateral stems) and fractal (dichotomous stems) structure. The project is divided into a generator and editor component. The generator is licensed under the Apache License (version 2.0). The editor is licensed under the GPL 3.0.

![Plant Generator](http://www.treemaker.xyz/window.png)

## Instructions

__Mouse controls__
- _Middle mouse button:_ Rotate the camera.
- _Middle mouse button + Shift:_ Pan the camera.
- _Middle mouse button + Ctrl:_ Zoom with the camera.
- _Left mouse button:_ Move items or add stems when no points are selected.
- _Right mouse button:_ Select items in the scene.
- _Double left click (curve editor):_ Add new control points.

__Key controls__
- _E_ Extrude the active path from the selected point.
- _R_ Rotate the path of stems.

## Installation

```sh
sudo dnf install qt5-devel
cd ~/plant
git submodule update --init --recursive
make all
```

## Todo
- Improve seams for stems.
- Implement an outline shader.
- Implement a flare component for stems.
