# Prerequisities

## Platform

Recodex uses Linux and GCC to compile and run your submissions. It may trigger errors which escaped detection in other compile/run environments. For such cases, it is advantageous to have a GCC-based build environment prepared beforehand. 

On the other hand, the Cecko sources are intended to be multi-platform and the skeleton is regularly tested with both GCC and Microsoft Visual C++. Therefore, development using MSVC is possible (and encouraged).

### Windows Subsystem for Linux

For developers equipped with Windows 10, [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/about) (WSL) is recommended as the best approach to multi-platform development. Compiling, running and debugging in the WSL-GCC environment is integrated in Visual Studio 2019 (as well as VS Code) and fully supported by Cecko projects.

_Note: Due to the small size of Cecko, there is no significant build-performance difference neither between WSL 1 and WSL 2 nor between Windows and Linux filesystems. However, if you plan to build LLVM on your own (instead of downloading binaries), placing the working folder in the Linux filesystem of WSL 2 may shorten the build time by an hour. Ubuntu (currently 20.04 Focal) is the recommended distribution for WSL (and the default supplied by Microsoft)._

## Git

Make yourself familiar with the principles of **git**.

In Linux, you will probably use command-line `git` which is present in all modern Linux distributions.

In Windows, [TortoiseGit](https://tortoisegit.org/download/) is the recommended option. 

_You may also try using the Git plugins in Visual Studio 2019 or VS Code; however, be aware that some advanced commands may not be available there._

To access [gitlab.mff.cuni.cz](https://gitlab.mff.cuni.cz), you will first need to login (using your SIS credentials) in its web interface and register your SSH keys there (if you have not done so yet).

## Build tools

### cmake

**CMake 3.8 or newer is required.** It is present in any (sufficiently new) distribution of **Linux (including WSL)**, installed by something like:

```bash
sudo apt install cmake
```

For **Windows**, cmake (and ninja) is installed with Visual Studio 2019 when **C++ CMake tools for Windows** is enabled in the Visual Studio Installer. **C++ CMake tools for Linux** is required when Visual Studio is used to build in WSL or remote Linux.

When using VS Code, cmake (as well as a C++ compiler) must be installed as an external tool.

### bison and flex

**bison 3.4 and flex 2.6.4 or newer are required.** They are present in any (sufficiently new) distribution of **Linux (including WSL)**, installed by something like:

```bash
sudo apt install bison flex
```

For **Windows**, they may be downloaded from [lexxmark/winflexbison](https://github.com/lexxmark/winflexbison/releases). Unpack the .zip to a suitable location and **add the folder to the `Path` variable** using the **Edit the system environment variables** control panel dialog. This package contains `win_bison.exe` and `win_flex.exe` - these names *are* recognized by cmake.

_Note: Although Chocolatey contains a `winflexbison3` package, it (as of July 2020) does **not** satisfy our version requirements._

_Note: You cannot use WSL installations of bison/flex for MSVC builds (and vice versa) because cmake would not invoke them properly. Therefore you will probably end with two bison/flex installations if you do both MSVC and GCC builds, similarly to having two installations of cmake in this case._

### C++17 compiler

MSVC 19.26 (part of Visual Studio 2019) and gcc 9.3.0 are verified to be sufficient for the compilation of Cecko.

## LLVM

Cecko uses the intermediate code (IR) and JIT x86 code generator (MCJIT) from the [LLVM](https://llvm.org/docs/) framework. Although the LLVM team recommends building LLVM from sources, it may be well beyond the capability and patience of most Cecko developers. Therefore, downloading a binary version is recommended. 

_Note: Cecko requires only headers and static libraries; however, all binary distributions of LLVM include also many executables and dynamic libraries. Some downloads/packages include only the executables which is insufficient for Cecko._

### Linux (including WSL) binaries

The simplest way to install the required LLVM modules is from a Linux distribution channel, like:

```bash
sudo apt install llvm-10
```

cmake will automatically find LLVM if installed in this way.

_Note: Although these libraries are compatible with Debug mode, they do not contain full debug info, for example, stepping into LLVM procedures during debugging of Cecko may be impossible._

_Note: LLVM is also available in the form of [Pre-Built Binaries](https://releases.llvm.org/download.html). Hovewer, their availability and contents are variable; also their correct installation is not trivial (see the Windows subsection below for some hints)._

### Windows binaries

_Beware: The Windows versions of Pre-Built Binaries published by LLVM (as of July 2020) are **not** complete enough for Cecko._

Pre-built LLVM binaries for Cecko are avalable at [gitlab.mff.cuni.cz](https://gitlab.mff.cuni.cz/teaching/nswi098/cecko/llvm-install). The version for Windows MSVC is located at the branch `MSVC-x64-Debug`. Clone the branch into `<llvm-install-folder>` using:

```bash
git clone -b MSVC-x64-Debug git@gitlab.mff.cuni.cz:teaching/nswi098/cecko/llvm-install.git <llvm-install-folder>
```

_The cloning may take some time, the download size is about 600 MB because of full debug info._

_Note: Because publishing incomplete binaries may interfere with the LLVM licence, this gitlab repository is available only for logged-on users._

When building Cecko, **cmake must be instructed to find LLVM** by setting the `LLVM_DIR` cmake variable to the value `<llvm-install-folder>/lib/cmake/llvm`. 

In Visual Studio 2019, the `LLVM_DIR` variable is set in the **CMake variables and cache** section of the **CMake Settings** (CMakeSettings.json); you may run **Generate Cache** to pre-create this variable before setting it to the correct value.

In VS Code, the `LLVM_DIR` variable is located in the `cmake.configureSettings` entry of `.vscode/settings.json`.

When cmake is invoked from command-line, it may look like:

```bash
cmake -DLLVM_DIR=<llvm-install-folder>/lib/cmake/llvm <...other-arguments...>
```

### Building LLVM (advanced)

If you decide to ignore binary distributions and build LLVM on your own (beware, it may require several hours of downloading and building), the simplest approach is cloning [llvm-project](https://github.com/llvm/llvm-project.git) into a `<llvm-project-folder>`:

```bash
git clone https://github.com/llvm/llvm-project.git <llvm-project-folder>
```

To build and install, create a `<llvm-build-folder>` and a `<llvm-install-folder>`, then configure, build and install using:

```bash
cd <llvm-build-folder>
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=<llvm-install-folder> <llvm-project-folder>/llvm
cmake --build .
cmake --build . --target install
```

When building Cecko with the home-built LLVM, define the `LLVM_DIR` variable as `<llvm-install-folder>/lib/cmake/llvm`.

_See also [Building LLVM with CMake](https://llvm.org/docs/CMake.html)._

