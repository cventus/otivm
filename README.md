OTIVM
=====

OTIVM is a small toy project with very vague goals in mind. It is an
environment to see how much abstraction functions, pointers and the simple
semantics of C can get me and how difficult it is to write maintainable and
robust code in C. Being educational in nature, almost everything is written
from scratch just for the exercise. The software itself is currently turning
into some kind of a 3D graphics application.

Project Structure
-----------------

The project mainly consists of C source files that are located in
subdirectories under `./src`. Each subdirectory here is called a *module*,
because the functionality implemented by the files should be related. A module
`X` can have a `./src/X/module.sh` configuration file, which produces a
fragment of a makefile.

The output of a module can be either an executable or a library, and this is
specified in the configuration. A module often depends on functionality
provided by another one, and module dependencies also need to be explicitly
specified in the configuration. If module *A* depends on module *B* then
whenever a source file in *A* is compiled, it gets passed include search
directories so that the interface of *B* becomes available. In case *A* has an
executable output, it is also linked with the object files of *B*.

The `./build.conf` file in the root project directory defines the list of
modules which are part of the project. 

Building & Testing
------------------

A custom shell script builds the binaries and tests, and it expects the
availability of common UNIX tools , such as `sed` and `awk`.  Although, I've
tried to remain portable, I've only run this on my own machine. To render
triangles, OTIVM needs OpenGL and X11 libraries and headers, so be sure to have
those installed. Running

    ./ms

produces binaries and some other build artifacts in several sub-directories of
`./target`. For instance, object files are placed under `./target/obj` and each
object of a module is combined into an archive and placed in the `./target/lib`
directory.

Tests can be compiled with the

    ./ms make:test

command. Tests are intentionally separate from `all` because tests are not
expected to always successfully compile. For instance, there might be a linker
error from missing function implementations. A test that doesn't compile is
test that has failed. It is useful to be able compile other modules and tests
even though one is not working.

By convention, tests are expected to output
[TAP](https://en.wikipedia.org/wiki/Test_Anything_Protocol). The `test.sh`
script in the project root directory and the scripts under `./taps` can be used
to make running tests a little more convenient.

The make-and-sh script `ms` behaves mostly like `make(1)`, i.e. flags, variable
assignments, and targets can be passed on the command line. It is a shell
script that generates a Makefile which is piped into make. It has various
utilities for inspection and selectively building targets, e.g.

    ./ms make:test/**/bin

will select all targets that matches the (unanchored) glob after "make:" and
pass them as additional command line arguments to make. Similarly, the are
pseudo targets "show:<glob>" prints all targets that match the pattern on
stdout, "clean:<glob>" deletes targets that match the pattern, and
"show:Makefile", which prints the generated makefile on stdout. 
