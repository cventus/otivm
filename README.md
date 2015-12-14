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
`X` can have a `./src/X/module.mk` configuration file, a public interface of
header files in `./src/X/include`, and a suite of tests under `./src/X/test`.
Tests are any C sources in this directory, and each of them are compiled into a
separate binary. For this reason they should have `main()` function.

The output of a module can be either an executable or a library, and this is
specified in the configuration. A module often depends on functionality
provided by another one, and module dependencies also need to be explicitly
specified in the configuration. If module *A* depends on module *B* then
whenever a source file in *A* is compiled, it gets passed include search
directories so that the interface of *B* becomes available. In case *A* has an
executable output, it is also linked with the object files of *B*-

The `./config.mk` file in the root project directory defines the list of
modules which are part of the project. 

Building & Testing
------------------

A single, non-recursive GNU Makefile builds the binaries and tests, and it
expects the availability of common UNIX tools , such as `sed` and `awk`.
Although, I've tried to remain portable, I've only run this on my own machine.
To render triangles, OTIVM needs OpenGL and X11 libraries and headers, so be
sure to have those installed. Running

    make

or `make all` produces binaries and some other build artifacts in several
sub-directories of `./build`. For instance, object files are placed under
`./build/object` and each object of a module is combined into an archive and
placed in the `./build/lib` directory.

Tests can be compiled with the

    make test

command. The `test` rule is intentionally separate from `all`, because tests
are not expected to always successfully compile. For instance, there might be a
linker error from missing function implementations. While all tests should
compile cleanly and a test that doesn't compile is test that has failed, it is
useful to be able compile other modules and tests even though one is not
working. The `make -k` option is only for special circumstances, and a test for
one module might stay un-buildable until a major feature is implemented in
another module, and its tests all pass.

By convention, tests are expected to output TAP (or someting close enough to
it). The `test.sh` script in the project root directory and the scripts under
`./taps` can be used to make running tests a little more convenient.

There are two rules for each module to to compile only the module (and the
modules it depends on) and its tests. For instance, for module `base`

    make base

and

    make test-base

can be used to build the module (i.e. an archive) or its tests, respectively.

