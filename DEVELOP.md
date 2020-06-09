## Visual Studio and GDB in OSX Docker container

### OSX Installation



```
brew install gdb
```
* [Follow these signing instructions.](https://sourceware.org/gdb/wiki/PermissionsDarwin)
    * Note that there is a ./docker/gdb-entitlement.xml for that step in the instructions.

> Special thanks to [Spencer Elliott](https://medium.com/@spe_) for his blog post on [Debugging C/C++ Programs Remotely Using Visual Studio Code and gdbserver](https://medium.com/@spe_/debugging-c-c-programs-remotely-using-visual-studio-code-and-gdbserver-559d3434fb78)