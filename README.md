# injector-patch-thing
(Still working on the name)
This will inject/patch .c code and compiling into a binary.

The idea of this is to allow a user to give it a .c file, it will compile it (target arch in dev, right now its default to the running CPU), strip the binarys (.text section specifically), and shove them where ever the user wants.
