## Running Tests

To run tests, either:
1. `make check` to run the automated framework
2. `python3 tests/test.test` to do a manual run

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

