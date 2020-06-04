| Please Note: Systemf() is alpha code. |
| ------------------------------------- |
| Systemf() is at alpha version and should not yet be used in production.  It has been open source by Cisco in alpha state to allow collaborative development by the open-source community. |

# systemf
Prepared statement support for the system command.

# synopsis

    #include <systemf0.h>

    int systemf(const char *fmt, ...);

## Example, 

Consider a simple command that takes user input and calls system with it.  Without systemf() you would have to do this:

```
int example_func(char *user_input) {
   char fmt[] = "/bin/mymagicfunc %s";
   char *buf = malloc(sizeof(fmt) + strlen(user_input));
   int result;
   sprintf(buf, fmt, user_input);

   if (buf == NULL) {
       return -1;
   }
   result = system(buf);
   free(buf);
   return result;
}
```
With systemf, all you would have to do is this:
```
int example_func(char *user_input) {
   return systemf("/bin/mymagicfunc %s", user_input);
}
```

But that isn't the reason systemf() was created (but it is a great advantage).  There is a big security advantage.  
user_input is sent as a single argument and there is no /bin/sh involved.  So if they did something like, 
`user_input = "goodbye ; rm -rf /"`, the first example would try to execute the 'rm -rf /' while the 
second would just send the whole string as a single argument to /bin/mymagicfunc.

This doesn't solve everything.  If /bin/mymagicfunc had an injection issue, it might still cause the code 
to be run, but you can't prevent everything.

## Format String and Argument Parsing

The first argument to systemf systemf() specifies the how the code will be called and allows a convenient way to bring in parameterized user input.  Think of it as baby shell with most of what you would need when calling out to system, but protections from the common mistakes when calling system.

*The format string supports*:
* Redirection operators: `2>&1`, `2>`, `&>`, `>`, `2>>`, `&>>`, and `>>`
* Command chaining operators: `&&`, `|`, `||`, and `;`
* Limited file globbing: `*`, and `[a-z]`
* Argument expansion: Glob: `%g`, File: `%f`, Path: `%p`, String: `%s`, Integer: `%d`

## Return Values

The base systemf() will have the same return values as the system() function.

The following is copied from http://man7.org/linux/man-pages/man3/system.3.html :

*  If any child process could not be created, or its status could not
    be retrieved, the return value is -1 and errno is set to indicate
    the error.

*  If all spawned child processes succeed, then the return value is the
    termination status of the last spawned child process.

## Executable Paths (Still being designed)

Systemf by default only allows absolute paths and a very limited PATH parsing.

Allow absolute paths to any binary to run.

For relative path specifications, must be in a folder in global safe paths
* Programmer can add to safe paths at compile or run time
* Safe paths will default to /bin /sbin /usr/sbin /usr/bin etc

## File Sandboxing (Still being designed)

By default, if a file is specified, it must be located in the realpath of the current directory.  We are designing how
this can be expanded and expect alternate paths to be able to be specified in the format string, but that is still being
designed.

## Future Considerations
* Currently ignoring environment variables / environment altogether - may need to revisit

## Building

To build, run `./configure && make build`.

### Building from scratch

This uses gnu AutoTools to generate a configure and makefile.
To remove that dependency and allow building directly from a
git pull, the files generated from these are checked into this
repository.

#### OSX installation from Scratch
```
brew update
brew install autoconf automake
autoreconf -i
./configure && make
make checks
```

#### Ubuntu Installation from Scratch

On a fresh ubuntu install you'll likely want to do this:
```
apt update
apt install build-essential autotools-dev automake libtool
autoreconf -f -i
./configure && make
```

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

