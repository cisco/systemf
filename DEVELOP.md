# Developer notes

This uses gnu AutoTools to generate a configure and makefile.
To remove that dependency and allow building directly from a
git pull, the files generated from these are checked into this
repository.

## OSX installation from Scratch
```
brew update
brew install autoconf automake
autoreconf -i
./configure && make
make checks
```

**Note:** There is a configure option of --enable-code-coverage,
but currently the `make check` complains, `ld: library not found for -lgcov`
Instead, we are calculating code coverage using the docker build with
[github actions](https://github.com/yonhan3/systemf/actions).

## Ubuntu Installation from Scratch

The easiest way to use ubuntu-like is to use the latest docker meklund/systemf-gh-action.

If you want to build from scratch, use the files in Docker for guidence:

For candidate RPMS, look at the Dockerfile:
```
grep apt-get docker/Dockerfile
```
For building for development, either run `docker/systemf-build` or use it as guidence.

## Visual Studio and GDB in OSX Docker container

### OSX Installation

```
brew install gdb
```
* [Follow these signing instructions.](https://sourceware.org/gdb/wiki/PermissionsDarwin)
    * Note that there is a ./docker/gdb-entitlement.xml for that step in the instructions.

> Special thanks to [Spencer Elliott](https://medium.com/@spe_) for his blog post on [Debugging C/C++ Programs Remotely Using Visual Studio Code and gdbserver](https://medium.com/@spe_/debugging-c-c-programs-remotely-using-visual-studio-code-and-gdbserver-559d3434fb78)


## Appendix A - printf options

Some of these options were used as the basis for the format strings
in the systemf() code.  They are here for reference.

| option | data type | option | data type |
| ------ | --------- | ------ | --------- |
| d, i, o, u, x, X | int | e, E, f, F, g, G, a A | double |
| c | char | s | string |
| C, S, | Not C99, "Don't use" | p | pointer |
| n | count of chars written so far |  m | GlibC strerror(errno) |
| % | % | | |

