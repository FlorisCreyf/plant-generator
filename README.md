# BlueTree
(_work in progress_)

BlueTree is a procedural tree generator. The editor uses Qt and renders the tree with OpenGL so that the source is compilable on most operating systems. The component that does the tree generation is written in C and is compiled as a static library.

## Outline
The diagram bellow is a rough overview of how the generator is structured.

![outline](http://www.fcreyf.com/img/btoutline.png)

The vertex buffer is an interleaved buffer containing positions and normals.

## Installation
The editor requires Qt4 libraries and uses qmake to generate a makefile from `bluetree.pro`. To build the project:
```
cd ${project/directory}
make all
```

![bluetree](http://www.fcreyf.com/img/bt.png)

