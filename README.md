# Boring32

[![Language](https://img.shields.io/badge/Language%20-C++20-blue.svg)](https://github.com/yottaawesome/boring32/)
[![Platform](https://img.shields.io/badge/Platform%20-Win32-blue.svg)](https://github.com/yottaawesome/boring32/)

## Introduction

`Boring32` is a small static module-only library framework developed as a hobby that provides a number of convenience classes around the Win32 API, with a particular emphasis on asynchronous primitives and functions. `Boring32` provides wrapper classes for Win32 IPC and synchronisation primitives, such as events, mutexes, semaphores, timers, processes, threads, and thread pools, but also provides some additional classes for other things, such as compression, security, WinSock, and more.

## Building

Building requires Visual Studio 2022 with the _Desktop development with C++_ workload installed. Simply clone this repository and open `src/Boring32.sln` and you should be able to build the project immediately and run the test project `Boring32.Tests`. The built static library is `src\Build\x64\Debug`. Boring32 is built with pure C++20 modules, so any dependent projects need to be able to find the module files. At the moment, these modules are not packaged into a release, but I might do this eventually.

## Documentation

The test project `Boring32.Tests` is a good reference of how to use `Boring32`, at least until I can devote a bit more time to documenting the various classes. Note that this project is still in development, so as time goes by, I'll be adding, modifying, and removing stuff. There's also a unit test project that can be used as a reference, and you can run these tests either from VS's test runner, or through the `vstest.console.exe` binary. This binary can be found in the VS build tools and can be invoked through the `x64 Native Tools Command Prompt for VS 2019` like so: `vstest.console.exe Boring32.UnitTests.dll`. This returns 0 or 1, depending on whether the tests succeeded or failed (you can use `echo Exit Code is %errorlevel%` to check the return code from the command prompt).
