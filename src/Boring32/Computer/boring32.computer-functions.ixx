module;

#include <string>
#include <Windows.h>

export module boring32.computer:functions;

export namespace Boring32::Computer
{
    std::wstring GetFormattedHostName(const COMPUTER_NAME_FORMAT format);
}
