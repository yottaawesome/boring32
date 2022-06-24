#pragma warning(disable: 4005)
#pragma warning(disable: 5106)
import <string>;
import <Windows.h>;
import <comdef.h>;

// https://developercommunity.visualstudio.com/t/warning-C4005:-Outptr:-macro-redefinit/1546919
//import boring32.headerunit;
//
void Blah()
{
	IUnknown* sl;
	int bla5 = 4;
//	//std::source_location::current();
	ULONG blah = 5;
	Sleep(500);
}