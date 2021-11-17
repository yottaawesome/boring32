#pragma once
#include "RegistryFuncs.hpp"

// https://docs.microsoft.com/en-us/archive/msdn-magazine/2017/may/c-use-modern-c-to-access-the-windows-registry
namespace Boring32::Registry
{
    void WatchKey(const HKEY key, const Async::Event& eventToSignal);
}
