
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

## Ubuntu Installation from Scratch

On a fresh ubuntu install you'll likely want to do this:
```
apt update
apt install build-essential autotools-dev automake libtool
autoreconf -f -i
./configure && make
```

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

