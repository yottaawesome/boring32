# Boring32 repository instructions

## Build and test

Use Visual Studio 2026 with the **Desktop development with C++** workload, MSVC v145, a Windows SDK, and C++ AddressSanitizer support (enabled in the x64 configurations). Open `src\Boring32.sln` or run these Debug x64 commands from the repository root in the x64 Native Tools Command Prompt:

```bat
rem Build the solution, including the native test DLL
msbuild .\src\Boring32.sln /m /p:Configuration=Debug /p:Platform=x64
rem Build only the library
msbuild .\src\Boring32.sln /m /t:Boring32 /p:Configuration=Debug /p:Platform=x64
```

The static library is `src\x64\Debug\Boring32.lib`. The test DLL and `testfile.txt` fixture go to `src\Build\x64\Debug`; run VSTest from that directory so file tests can find the fixture:

```bat
cd /d .\src\Build\x64\Debug
rem Run the full suite
vstest.console.exe .\Boring32.UnitTests.dll /Platform:x64
rem List available tests
vstest.console.exe .\Boring32.UnitTests.dll /Platform:x64 /ListTests
rem Run a single test by namespace, class, and method
vstest.console.exe .\Boring32.UnitTests.dll /Platform:x64 /TestCaseFilter:"FullyQualifiedName=Strings::FixedString::TestSize"
```

Tests use the native Visual Studio C++ Unit Test Framework (`pch.h`, `TEST_CLASS`, `TEST_METHOD`) and exercise the public API. Some certificate tests depend on certificates installed on the developer's machine.

Use the library-only target until the test project migrates from `import boring32;` to `import Boring32;`. The pre-existing duplicate `Boring32:DataStructures.CappedStack` declaration in `CappedStack.ixx` and `CircularBuffer.ixx` also blocks compilation of the CappedStack tests.

## Architecture

- Boring32 is a C++23, module-only static library of Win32 wrappers, with an emphasis on synchronization, asynchronous I/O, and IPC. Consumers link the library and need the module interface files to `import Boring32;`.
- `src\Boring32\boring32.ixx` is the primary interface. It re-exports subsystem partitions such as `:Async`; those aggregators re-export public leaf partitions such as `:Async.Event`. Wrapper code usually lives in the leaf `.ixx` files, with internal partitions imported without re-exporting them.
- `Win32\Win32.ixx` is the SDK boundary: it includes `Win32\framework.h` in its global module fragment and exposes SDK functions, types, and typed constants under `Boring32::Win32`. Higher-level wrappers import `:Win32` and build on the shared `:Raii` and `:Error` partitions. Subsystem aggregators also carry their SDK link directives, such as `WinHttp\WinHttp.ixx` linking `winhttp.lib`.

## Conventions

- For SDK additions, extend the `:Win32` shim and use `Win32::` names in wrappers rather than adding Windows headers to leaf partitions. Use `import std;` for standard library facilities.
- Use `RAII::UniqueHandle` for exclusive ownership of handles closed by `CloseHandle`. For resources with another release function, follow the subsystem's deleter pattern or use `RAII::UniquePtr`/`IndirectUniquePtr` with the matching API. `GetHandle()` borrows; `Detach()` transfers ownership. For Win32 calls that report errors through `GetLastError`, capture it immediately and use `Error::Win32Error`. Where a wrapper has a `Try*` variant, preserve its `bool` or `std::expected` failure contract.
- Partition filenames and logical partition identifiers use PascalCase, with acronyms of up to three characters in uppercase (`:Async.APCThread`, `:Networking.API`) and longer acronyms in initial-cap case (`:Raii`, `:Sspi`). Keep declarations and imports in sync when changing a logical identifier. When adding, moving, or renaming a partition file, update both `Boring32.vcxproj` and `Boring32.vcxproj.filters`; add public partitions to the corresponding export chain. MSBuild keeps module and object intermediates per relative directory because names such as `Functions.ixx` recur across subsystems.
