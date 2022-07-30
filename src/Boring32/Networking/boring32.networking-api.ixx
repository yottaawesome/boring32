module;

#include <vector>

export module boring32.networking:api;

export namespace Boring32::Networking
{
	std::vector<std::byte> GetAdapters(const unsigned family, const unsigned flags);
}