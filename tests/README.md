# Cubit Qt Quick Tests

This directory contains a simplified test structure for testing the Cubit Qt Quick application without requiring Qt Quick Test module.

## Test Structure

The test suite uses standard Qt Test framework with custom QML testing capabilities:

- **test_main.cpp**: Main test runner that registers QML types and executes test suites
- **QmlTestHelper.h**: Helper class for creating and testing QML objects
- **tst_FrameOperations.cpp**: Tests for Frame element creation, modification, and signals
- **tst_SelectionManager.cpp**: Tests for selection operations (single, multiple, bounds)
- **tst_ElementModel.cpp**: Tests for element model operations and QML integration

## Key Features

### QML Testing Without Qt Quick Test
- Uses a custom QmlTestHelper class to create QML objects in tests
- Provides helper methods for property access and method invocation
- Supports both C++ object testing and QML integration testing

### Test Categories
1. **C++ Backend Testing**: Direct testing of C++ classes (Element, Frame, SelectionManager, etc.)
2. **QML Integration Testing**: Testing QML objects that use registered C++ types
3. **Signal/Slot Testing**: Verification of Qt signal emissions and connections

### Test Scenarios Covered
- Element creation and property manipulation
- Single and multiple element selection
- Selection bounds calculation
- Model operations (add, remove, clear)
- Parent-child element relationships
- QML-C++ property bindings
- Signal emission verification

## Running Tests

```bash
# Run the test script
./run_tests.sh

# Or manually:
qmake6 tests.pro
make
./cubit-tests
```

## Test Design Principles

1. **No Mock Objects**: Tests use real implementations, not mocks
2. **Isolated Tests**: Each test method is independent with proper setup/cleanup
3. **Both C++ and QML**: Tests cover both backend C++ logic and QML integration
4. **Signal Verification**: Important signals are tested with QSignalSpy
5. **Property Binding**: QML property bindings to C++ objects are verified

## Adding New Tests

To add a new test suite:

1. Create a new `tst_NewFeature.cpp` file following the existing pattern
2. Add it to `tests.pro` SOURCES
3. Include it in `test_main.cpp`
4. Follow the QmlTestHelper pattern for QML testing

Example test class structure:
```cpp
class tst_NewFeature : public QmlTestHelper
{
    Q_OBJECT
    
private slots:
    void init() { /* setup */ }
    void cleanup() { /* cleanup */ }
    void test_basicFunctionality() { /* test */ }
    void test_qmlIntegration() { /* QML test */ }
};
```