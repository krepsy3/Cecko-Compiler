# Building Cecko

## Command-line

Create a `<build-folder>`, by convention as `<cecko-folder>/build`. Then configure using these commands:

```bash
cd <build-folder>
cmake -DCMAKE_BUILD_TYPE=Debug -DLLVM_DIR=<llvm-install-folder>/lib/cmake/llvm <cecko-folder>
```

The `-DLLVM_DIR` option is usually not required if LLVM was installed as a system package.

Building is then done by invoking `cmake --build .` or `make` in `<build-folder>`. 

The resulting executables are located in `<build-folder>/main` and invoked like:

```bash
cd <build-folder>
<build-folder>/main/cecko6 <cecko-folder>/test/test1.c
```

## Visual Studio 2019

Open the `<cecko-folder>` with Visual Studio. 

When opened for the first time, use **PROJECT/CMake Settings** to adjust the cmake configuration, the `LLVM_DIR` variable in particular (see Prerequisities). The default `x64-Debug` configuration is created automatically; to use WSL, add the `WSL-GCC-Debug` configuration, adjust it if necessary and switch to it.

## Visual Studio Code

In Windows, install the Visual C++ compiler first. Then start an **x64 Developer Command Prompt** (which defines the environment where the compiler is visible), then run `code` from the command prompt. Then follow the documentation for the **CMake Tools** extension.




