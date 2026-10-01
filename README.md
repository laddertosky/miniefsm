## Build
```Bash
cmake -S . -B build
cmake --build build -j8
```

## Run Tests
```Bash
ctest --test-dir build
```

