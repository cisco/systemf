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

## Code Coverage

Code coverage is only currently working in the ubuntu docker
container.  OSX has issues.  To run code coverage:

```
./configure --enable-code-coverage
make check-code-coverage
```
Alternatively:
```
(cd docker && COMMAND=/usr/bin/systemf-check docker-compose up)
```
The resulting html will be printed, but it will likely
point to [here](systemf-coverage/index.html) 
(this is not checked into git).

## Visual Studio and GDB in OSX Docker container

### OSX Installation

```
brew install gdb
```
* [Follow these signing instructions.](https://sourceware.org/gdb/wiki/PermissionsDarwin)
    * Note that there is a ./docker/gdb-entitlement.xml for that step in the instructions.

> Special thanks to [Spencer Elliott](https://medium.com/@spe_) for his blog post on [Debugging C/C++ Programs Remotely Using Visual Studio Code and gdbserver](https://medium.com/@spe_/debugging-c-c-programs-remotely-using-visual-studio-code-and-gdbserver-559d3434fb78)

## How to Create a Release

Currently creating a release is a manual process.  Once a consistent
process is created, a github action will be created.

1. Update AC_INIT in configure.ac with the [semantic version](http://semver.org/).
2. Start the docker container with latest code: 
    ```
    docker-compose -f docker/docker-compose.yml up --build -d
    ```
3. Run the script to do most of the work in the container: 
    ```
    docker-compose -f docker/docker-compose.yml exec systemf-test systemf-release-build
    ```
4. Tag this release and push the tag.
    For example:
    ```
    git tag -a V0.9.0 -m 0.9.0
    git push --follow-tags
    ```
    *The last line output from step 3 gives the exact command to use.*
5. You may have to get your branch merged from a fork into master.  Push your branch to cisco.
    ```
    git remote add cisco git@github.com:cisco/systemf.git
    git push --follow-tags cisco
    ```
6. Create a [github release](https://github.com/cisco/systemf/releases/new)
    1. Enter your tag version in `Tag Version`.
    2. For `Release title` enter a title with a format of "Release 0.9.0".
    3. For the description, give a bulleted list of all major changes.
    4. Attach the systemf-$version.tar.gz
    5. Click 'this is a pre-release' if it isn't yet ready for general consumption.
    6. `Publish release`


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

