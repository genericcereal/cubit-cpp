# ScriptCompiler Tests

This directory contains unit tests for the ScriptCompiler functionality that compiles visual node graphs into executable JSON.

## Test File

- `test_script_compiler.cpp` - Comprehensive unit tests for the ScriptCompiler class

## Building and Running

```bash
# Build the test
qmake6 -o Makefile.scriptcompiler test_script_compiler.pro
make -f Makefile.scriptcompiler clean
make -f Makefile.scriptcompiler -j8

# Run the test
./test_script_compiler.app/Contents/MacOS/test_script_compiler
```

## Test Coverage

The test suite covers:

1. **testEmptyScripts** - Verifies compiler handles empty scripts gracefully
2. **testSingleEventNode** - Tests compilation of a single event node
3. **testConsoleLogWithValue** - Verifies node values are captured and included in params
4. **testConsoleLogWithEdgeConnection** - Tests data flow through edges
5. **testMultipleNodes** - Verifies compilation of multi-node graphs
6. **testNodeValueProperty** - Tests the Node class value property functionality

## Key Features Tested

- Node value property storage and retrieval
- Compilation of node graphs to JSON format
- Proper handling of flow vs data edges
- Output generation for node values
- Parameter passing between nodes
- Function deduplication in compiled output

## Example Compiled Output

When a console log node with value "Hello from test!" is compiled:

```json
{
    "oneditorload": {
        "functions": {
            "consoleLog_0": "(params) => console.log(params[0].value || params[0].output)"
        },
        "invoke": [
            {
                "function": "consoleLog_0",
                "id": "console1",
                "params": [
                    {"output": 0}
                ]
            }
        ],
        "outputs": [
            {
                "id": 0,
                "nodeId": "console1",
                "type": "literal",
                "value": "Hello from test!"
            }
        ]
    }
}
```