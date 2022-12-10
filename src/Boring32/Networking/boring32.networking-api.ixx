export module boring32.networking:api;
import <vector>;

export namespace Boring32::Networking
{
	std::vector<std::byte> GetAdapters(const unsigned family, const unsigned flags);
}