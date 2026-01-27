# Boring32

[![Language](https://img.shields.io/badge/Language%20-C++23-blue.svg)](https://github.com/yottaawesome/boring32/)
[![Platform](https://img.shields.io/badge/Platform%20-Win32x64-blue.svg)](https://github.com/yottaawesome/boring32/)

## Introduction

`Boring32` is a small C++23 module-only static library framework developed as a hobby that provides a number of C++ convenience classes and functions around the Win32 (x64) API, with a particular emphasis on asynchronous primitives and functions. `Boring32` provides wrapper classes for Win32 IPC and synchronisation primitives, such as events, mutexes, semaphores, timers, processes, threads, and thread pools, but also provides some additional classes for other things, such as compression, security, WinSock, and more. `Boring32` embraces modern C++ and is written with the latest C++ features and conventions in mind. `Boring32` is not meant to be a comprehensive abstraction over the Win32 API, just something about exploring how to work with the Win32 API in bleeding-edge C++.

## Using

`Boring32` is built as a hobby and isn't intended to be an actual framework for production use. However, it's a useful practical reference for how to use Win32 functions and primitives (including some obscure ones), or how you might go about constructing your own wrappers around Win32 primitives. Some of the classes, e.g. `Event`, can be copied over into your own codebases with relatively minor changes. You can also use `Boring32` as a launch point to explore some of the more obscure Win32 APIs, such as Windows Imaging Component and Windows Filtering Platform.

## Building

Building requires Visual Studio 2026 with the _Desktop development with C++_ workload installed. Simply clone this repository and open `src\Boring32.sln` and you should be able to build the project immediately and run the test project `Boring32.Tests`. The built static library is `src\Build\x64\Debug`. Boring32 is built with pure C++20 modules, so any dependent projects need to be able to find the module files. At the moment, these modules are not packaged into a release, but I might do this eventually.

## Documentation

There's a unit test project that can be used as a reference, and you can run these tests either from VS's test runner, or through the `vstest.console.exe` binary. This binary can be found in the VS build tools and can be invoked through the `x64 Native Tools Command Prompt for VS 2022` like so: `vstest.console.exe Boring32.UnitTests.dll`. This returns 0 or 1, depending on whether the tests succeeded or failed (you can use `echo Exit Code is %errorlevel%` to check the return code from the command prompt).

## Other notes

### C++ modules

`Boring32` is now (2023-11-24) fully module-based and has eliminated any direct reliance on the Windows headers, and the primary module interface file is `boring32.ixx`. Some unit tests will likely fail on your machine as they are referencing objects (e.g. certificates) in my local environment I use for testing Win32 API calls. You can safely ignore these failures or comment them out. 

### C++ module naming convention

The preferred naming convention of the C++ modules in the codebase has undergone several changes over time as I've tested and reviewed different naming schemes. The final scheme I chose to adopt is to name module filenames in the format `<module-name>-<namespace>.<partitionname>.ixx`. Correspondingly, the modules are declared also following this convention, e.g. `boring32:crypto.certificatechain` This convention organises files by namespace and keeps filenames unique for MSVC which prevents obj file clashes when compiling. However, if you structure your code such that a unique vcxproj corresponds to one and one module only, then you can omit the `<module-name>` for all the partitions while keeping it for the primary module interface file. This is the current convention adopted by the `Boring32` project.

## Additional resources

* [Programming Windows 5th edition](https://github.com/yottaawesome/programming-windows-5th-edition)
* [Windows via C/C++ 5th edition](https://github.com/yottaawesome/windows-via-c-cpp)
* [ZetCode's Windows API tutorial](https://zetcode.com/gui/winapi/)
