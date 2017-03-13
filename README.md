# TreeMaker — [www.treemaker.xyz](http://www.treemaker.xyz)

TreeMaker is a program for creating tree models. The project is divided into a tree generator component and an editor component. The tree generator handles generating geometry and is implemented by the editor. Creating a tree model using the generator library requires several steps:

1. Create a tree object and set its initial properties.
2. Generate an initial tree structure with ``tree.generateTree()``.
3. Generate vertices and triangle indices using ``tree.generateMesh()``.

![treemaker](http://www.treemaker.xyz/window.png)

## Installation
Qt5 developer libraries are required to build the editor. Furthermore, the project uses qmake to generate a makefile from `treemaker.pro`, and images used by the application are stored in a separate repository. To build the project:

```sh
sudo dnf install qt-devel
cd ~/treemaker
git submodule update --init --recursive
make all
```
