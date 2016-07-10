# BlueTree
BlueTree is a procedural tree generator. The project is divided into a tree generator component and an editor component. The tree generator is written in C and is compiled as a static library. The editor is written in C++ and facilitates handcrafting trees. More information will be posted on the project page [treemaker.xyz](http://www.treemaker.xyz).

![bluetree](http://www.fcreyf.com/img/bt.png)

## Installation
The editor requires Qt4 libraries and uses qmake to generate a makefile from `bluetree.pro`. To build the project:
```sh
cd ${project/directory}
make all
```

## Overview
The diagram bellow is a rough overview of how the generator is structured.

![outline](http://www.fcreyf.com/img/btoutline.png)

The vertex buffer is an interleaved buffer containing positions and normals.

## Style guide
- Use 8 space hard tabs.
- No more than 80 characters per line.
- Use camel casing for C++ code and underscores for C code.
- Function and class opening brackets should be placed on a new line, and all other opening brackets should be placed on the same line. For example:
```C
int func(int value)
{
	if (value == 0) {
		int sum = 10 + 8;
		return sum;
	} else
		return 10;
}
```
- Continue an expression on a new line with two extra indentations if the expression is longer than 80 characters.
