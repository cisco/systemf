| Please Note: Systemf() is alpha code. |
| ------------------------------------- |
| Systemf() is at alpha version and should not yet be used in production.  It has been open source by Cisco in alpha state to allow collaborative development by the open-source community. |

# systemf
![C/C++ CI](https://github.com/yonhan3/systemf/workflows/C/C++%20CI/badge.svg) Prepared statement support for the system command.

## synopsis

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

But that isn't the reason systemf() was created (but it is a great advantage).  
There is a big security advantage.  user_input is sent as a single argument and
there is no /bin/sh involved.  So if they did something like, 
`user_input = "goodbye ; rm -rf /"`, the first example would try to execute the 
'rm -rf /' while the second would just send the whole string as a single argument 
to /bin/mymagicfunc.

This doesn't solve everything.  If /bin/mymagicfunc had an injection issue, it might still cause the code to be run, but you can't prevent everything.

## Quict Tour of Through Examples

The easiest way to explain the basics how `systemf()` works is through a few examples.

**Example 1: Basics**

* `systemf("/bin/echo The cat%s %d tail%s.", "'', tails, tails == 1 ? "" : "s");`

In the above example, systemf takes the format input, breaks it into parameters
by the spaces in each command, and sends it to execv.  For example if `tails` 
were 2, it would call execv with these arguments:  
`["/bin/echo", "The", "cat's", "2", "tails."]`.  Also, note that `systemf()` 
doesn't support `'` in the format string.  This is becuase `systemf()` supports
no quoting or escaping.

**Example 2: Parameter Splitting**

Now take the following call to `systemf()` into account:

`systemf("/bin/echo %s", "this line has spaces");`

`systemf()` only breaks lines into parameters by spaces and glob expansion. So in the above case, the arguments to execv will be: ["/bin/echo", "this line has spaces"] and it would **not** break the %s into spaces.

**Example 3: File Globbing**

`systemf()` also supports file globbing in the format string and as a glob paramerter, but not as a string.  Consider these three variations:

1. `systemf("/bin/echo *.c");`
2. `systemf("/bin/echo %*p", "*.c");`
3. `systemf("/bin/echo %s", "*.c");`

The first two will find every `c` file in the current directory and pass those as individual parameters to execv. `["/bin/echo", "a.c", "b.c", "c.c"]`.  While the third will send the text in verbatim: `["/bin/echo", "*.c"]` and since `echo` does not do glob expansion, literally `*.c` will be printed.

There are a caveats to the above.  If the glob pattern matches nothing, the
processing will stop, an error message will be printed, and `-1` will be returned.

Also, note that `systemf()` supports file path sandboxing.  That is a more advanced
subject than this introducton.  For more information see [File Sandboxing](#file-sandboxing-still-being-designed) below.




## Format String and Argument Parsing

The `fmt` argument to systemf() specifies the how the code will be called and 
allows a convenient way to bring in parameterized user input.  Think of it as 
limited shell with most of what you would need when calling out to system, but 
protections from the common mistakes when calling system.  The below table
summarizes which characters are allowed in the format string and their meanings.

| Token        | Meaning |
|:------------:| ------- |
| `a-z` `A-Z` ` 0-9` `.` `-` | Nonspecial characters allowed in `fmt`. (0) |
| *space tab*  | *Spaces* and *tabs* are interpreted as parameter separators. |
| `%s`         | Replace this with the string in the next available argument. (5) |
| `%d`         | Replace this with the integer in the next available argument. (5) |
| `%p`         | Like `%s`, but also [file sandboxed](#file-sandboxing-still-being-designed) |
| `%*p`        | Interpret the supplied parameter as a file glob. |
| `%!p`        | Like `%s`, but a trusted parameter for [file sandboxed](#file-sandboxing-still-being-designed) |
| `;`          | Command separator run if previous command exits cleanly. |
| `|`          | Command separator like `;` but also pipes stdout from prev into stdin |
| `&&`         | Command separator run if previous command exits cleanly with zero status. |
| `||`         | Command separator run if previous command exits cleanly with nonzero status. |
| `<`*file*    | Supply the stdin from the specified *file*. (1)(2) |
| `>`*file*    | Redirect the stdout into the specified *file*. (1)(2) |
| `>>`*file*   | Append the stdout into the specified *file*. (1)(3) |
| `2>`*file*   | Redirect the stderr into the specified *file*. (1)(2) |
| `2>>`*file*  | Append the stderr into the specified *file*. (1)(3) |
| `>&2`        | Redirect the stdout into the stderr. (4) |
| `2>&1`       | Redirect stderr into stdout. (4) |
| `&>`*file*   | Redirect stderr and stdout into the specified *file*. (1)(2) |
| `&>>`*file*  | Append stderr and stdout into the specified *file*. (1)(2) |

- (0) All tokens below in the table take precedence during parsing.
- (1) There is an optional space between the redirect and the filename.
- (2) Replace the file if it exists.
- (3) Create the file if it does not exist.
- (4) `systemf()` currently has no support of swapping the stdout and stderr.
- (5) Currently, no formatting specifiers are supported (like `%5d` or `%-10s`)

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

If you are a developer of systemf, see the [developer instructions](DEVELOP.md).

## Issues and feature requests.

Systemf is currently in a [temporary location](https://github.com/yonhan3/systemf). 
Issues may be raised [there](https://github.com/yonhan3/systemf/issues), but may 
not get transferred to its [permanant home](https://github.com/cisco/systemf).
