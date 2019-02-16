# Plant Generator

Plant Generator generates plant geometry. The project is divided into a generator and editor component, where the generator is licensed under the Apache License (version 2.0) and the editor is licensed under the GPL 3.0.

![Plant Generator](https://www.fcreyf.com/static/pg.png)

## Instructions

__Mouse controls__
- _Middle mouse button:_ Rotate the camera
- _Middle mouse button + Shift:_ Pan the camera
- _Middle mouse button + Ctrl:_ Zoom with the camera
- _Left mouse button:_ Move items or add stems when no points are selected
- _Right mouse button (+ Ctrl):_ Select items in the scene

__Key controls__
- _E_ Extrude the active path from the selected point
- _R_ Rotate the path of stems
- _A_ Add a new stem
- _M_ Move stems along the parent path
- _Num (+ Ctrl)_ Modify selection

## Installation

```sh
sudo dnf install qt5-devel
sudo dnf install boost-devel
cd ~/plant
git submodule update --init --recursive
make all
```

## Todo
- Implement a flare (i.e. branch collar and branch ridge) component for stems
- Implement branch forking
- Implement asymmetrical cross sections
- Finish leaf implementation
