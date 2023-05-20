module;

#include <source_location>;

export module boring32.strings;
import <string>;
import <vector>;
import <algorithm>;
import <stdexcept>;
import <win32.hpp>;
import boring32.error;

export namespace Boring32::Strings
{
	std::string ConvertString(const std::wstring_view wstr)
	{
		if (wstr.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-widechartomultibyte
		// Returns the size in bytes, this differs from MultiByteToWideChar, which returns the size in characters
		const int sizeInBytes = WideCharToMultiByte(
			CP_UTF8,										// CodePage
			WC_NO_BEST_FIT_CHARS,							// dwFlags 
			&wstr[0],										// lpWideCharStr
			static_cast<int>(wstr.size()),					// cchWideChar 
			nullptr,										// lpMultiByteStr
			0,												// cbMultiByte
			nullptr,										// lpDefaultChar
			nullptr											// lpUsedDefaultChar
		);
		if (sizeInBytes == 0)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("WideCharToMultiByte() [1] failed", lastError);
		}

		std::string strTo(sizeInBytes / sizeof(char), '\0');
		const int status = WideCharToMultiByte(
			CP_UTF8,										// CodePage
			WC_NO_BEST_FIT_CHARS,							// dwFlags 
			&wstr[0],										// lpWideCharStr
			static_cast<int>(wstr.size()),					// cchWideChar 
			&strTo[0],										// lpMultiByteStr
			static_cast<int>(strTo.size() * sizeof(char)),	// cbMultiByte
			nullptr,										// lpDefaultChar
			nullptr											// lpUsedDefaultChar
		);
		if (status == 0)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("WideCharToMultiByte() [2] failed", lastError);
		}

		return strTo;
	}

	std::wstring ConvertString(const std::string_view str)
	{
		if (str.empty())
			return {};

		// https://docs.microsoft.com/en-us/windows/win32/api/stringapiset/nf-stringapiset-multibytetowidechar
		// Returns the size in characters, this differs from WideCharToMultiByte, which returns the size in bytes
		const int sizeInCharacters = MultiByteToWideChar(
			CP_UTF8,									// CodePage
			0,											// dwFlags
			&str[0],									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),// cbMultiByte
			nullptr,									// lpWideCharStr
			0											// cchWideChar
		);
		if (sizeInCharacters == 0)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("MultiByteToWideChar() [1] failed", lastError);
		}

		std::wstring wstrTo(sizeInCharacters, '\0');
		const int status = MultiByteToWideChar(
			CP_UTF8,									// CodePage
			0,											// dwFlags
			&str[0],									// lpMultiByteStr
			static_cast<int>(str.size() * sizeof(char)),	// cbMultiByte
			&wstrTo[0],									// lpWideCharStr
			static_cast<int>(wstrTo.size())				// cchWideChar
		);
		if (status == 0)
		{
			const auto lastError = GetLastError();
			throw Error::Win32Error("MultiByteToWideChar() [2] failed", lastError);
		}

		return wstrTo;
	}

	std::vector<std::wstring> TokeniseString(
		const std::wstring& stringToTokenise,
		const std::wstring& delimiter
	)
	{
		size_t position = 0;
		// If we don't find it at all, add the whole string
		if (stringToTokenise.find(delimiter, position) == std::string::npos)
		{
			return { stringToTokenise };
		}

		std::vector<std::wstring> results;
		std::wstring intermediateString = stringToTokenise;
		while ((position = intermediateString.find(delimiter)) != std::string::npos)
		{
			// split and add to the results
			std::wstring split = intermediateString.substr(0, position);
			results.push_back(split);

			// move up our position
			position += delimiter.length();
			intermediateString = intermediateString.substr(position);

			// On the last iteration, enter the remainder
			if (intermediateString.find(delimiter) == std::string::npos)
				results.push_back(intermediateString);
		}

		return results;
	}

	// Adapted from https://stackoverflow.com/a/29752943/7448661
	std::wstring Replace(
		std::wstring source,
		const std::wstring& from,
		const std::wstring& to
	)
	{
		std::wstring newString;
		newString.reserve(source.length());

		std::wstring::size_type lastPos = 0;
		std::wstring::size_type findPos;

		while (std::wstring::npos != (findPos = source.find(from, lastPos)))
		{
			newString.append(source, lastPos, findPos - lastPos);
			newString += to;
			lastPos = findPos + from.length();
		}

		newString += source.substr(lastPos);

		return newString;
	}

	std::wstring Erase(std::wstring source, const wchar_t what)
	{
		source.erase(std::remove(source.begin(), source.end(), what), source.end());
		return source;
	}

	// Adapted from https://www.tutorialspoint.com/case-insensitive-string-comparison-in-cplusplus
	bool DoCaseInsensitiveMatch(std::string str1, std::string str2)
	{
		//convert s1 and s2 into lower case strings
		std::transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
		std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
		return str1 == str2;
	}

	bool DoCaseInsensitiveMatch(std::wstring str1, std::wstring str2)
	{
		//convert s1 and s2 into lower case strings
		std::transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
		std::transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
		return str1 == str2;
	}

	struct ArgInfo
	{
		std::vector<wchar_t> Argv;
		unsigned Argc = 0;;
	};

	ArgInfo StringsToArgVector(const std::vector<std::wstring>& args)
	{
		ArgInfo info{};
		for (const std::wstring& arg : args)
		{
			if (arg.empty())
				continue;
			info.Argv.insert(info.Argv.end(), arg.begin(), arg.end());
			info.Argv.push_back('\0');
			info.Argc++;
		}
		return info;
	}
}