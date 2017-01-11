# TreeMaker — [www.treemaker.xyz](http://www.treemaker.xyz)

TreeMaker is a program for creating tree models. The project is divided into a tree generator component and an editor component. The tree generator is written in C and handles generating geometry. The editor is written in C++ and facilitates handcrafting trees. Creating a tree model using the generator requires several steps:

1. Create a tree object and set its initial properties.
2. Generate an underlying tree structure with ``tmGenerateStructure(tree)``.
3. Generate vertices and triangles using ``tmGenerateMesh(tree)``.

![treemaker](http://www.treemaker.xyz/window.png)

## Installation
The editor requires Qt5 developer libraries and uses qmake to generate a makefile from `treemaker.pro`. To build the project:

```sh
cd ${project/directory}
make all
```
