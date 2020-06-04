## Running Tests

To run tests, either:
1. `make check` to run the automated framework
2. `python3 tests/test.test` to do a manual run

Running manually will give you the ability to see stderr and stdout output.

## JSON File Spec


test.json is a list of tests in JSON format that will run.

A test is defined as a JSON object looking like this:
```
{
    "description": "*test description*",
    "command": [ "*binary*", "*arg1*", "*arg2*" ],
    "stdout": ["*operator*", "*expected stdout including newlines*"],
    "stderr": null,
    "return_code": null
}
```

Note that "stdout", "stderr", and "return_code" can all be null if it doesn't matter
what they are.  Otherwise, they should be an array of "operator", "expected
value".

Operator can be: "==", "!=", ">", "<", "contains"

These objects are defined in a JSON list.

## Debugging

**Note**: If you want to debug, you may want to set `SHOULD_FORK = False` in
tests/test.test.  Otherwise, it will spawn child processes for systemf, which
makes it harder to debug.

This appears to work for running in GDB:
```
jaspadar@jaspadar-ubuntu:~/systemf$ gdb python3
Reading symbols from python3...(no debugging symbols found)...done.
(gdb) break systemf1
Function "systemf1" not defined.
Make breakpoint pending on future shared library load? (y or [n]) y
Breakpoint 1 (systemf1) pending.
(gdb) run tests/test.test
Starting program: /usr/bin/python3 tests/test.test
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
1..2
Trying ['binarydoesntexist']

Breakpoint 1, systemf1 (fmt=0x7ffff6580540 "b") at src/systemf.c:15
15      {
```
