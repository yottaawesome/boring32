# Boring32

[![Language](https://img.shields.io/badge/Language%20-C++20-blue.svg)](https://github.com/yottaawesome/boring32/)
[![Platform](https://img.shields.io/badge/Platform%20-Win32-blue.svg)](https://github.com/yottaawesome/boring32/)

## Introduction

`Boring32` is a small static library framework developed as a hobby that provides a number of convenience classes around the Win32 API, with a particular emphasis on asynchronous functions. `Boring32` provides wrapper classes for IPC Win32 kernel objects, such as events, mutexes, semaphores, timers, processes, and threads, as well as some additional classes for other things such as compression.

## Status

This project is being actively developed over time. The project is currently being migrated to C++20 modules.

## Building

Simply clone this repository and open `src/Boring32.sln` in Visual Studio 2019 Community Edition or higher. You should be able to build the project immediately and run the test project `Boring32.Tests`. The built static library is `src\Build\x64\Debug`, and you'll need the header files located in `src\Boring32\include` directory to be able to use the library once you've statically linked it.

## Documentation

The test project `Boring32.Tests` is a good reference of how to use the `Boring32` provided until I can devote a bit more time to comprehensively documenting the various classes and wrappers. Note that this project is still in development, so as time goes by, I'll be adding, modifying, and removing stuff. There's a unit test project added, but has no tests at the moment, they'll be added over time. You can run these tests either from VS's test runner, or through the `vstest.console.exe` binary. This binary can be found in the VS build tools and can be invoked through the `x64 Native Tools Command Prompt for VS 2019` like so: `vstest.console.exe Boring32.UnitTests.dll`. This returns 0 or 1, depending on whether the tests succeeded or failed (you can use `echo Exit Code is %errorlevel%` to check the return code from the command prompt).
