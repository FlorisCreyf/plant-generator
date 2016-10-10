# TreeMaker
TreeMaker is a program for creating tree models in games. The project is divided into a tree generator component and an editor component. The tree generator is written in C and is compiled as a library. The editor is written in C++ and facilitates handcrafting trees. Creating a tree model using the generator requires several steps:

1. Create a tree object and define its initial properties.
2. Generate an underlying tree structure with ``tm_generate_structure(tree)``.
3. Generate vertices and triangles using ``tm_generate_mesh(tree, etc)``.

There are a lot of things that still need to be done, such as:

1. Preventing branches from colliding with each other.
2. Textures and UV editors.
3. Wind animations.
4. Growing branches towards light and open spaces.

More information will be posted on the project page [treemaker.xyz](http://www.treemaker.xyz).

![treemaker](http://www.fcreyf.com/img/bt.png)

## Installation
The editor requires Qt5 developer libraries and uses qmake to generate a makefile from `treemaker.pro`. To build the project:
```sh
cd ${project/directory}
make all
```

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

