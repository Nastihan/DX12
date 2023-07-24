#include "GraphicsError.h"
#include <ranges>
#include <format>
#include <Windows.h>

std::string TranslateErrorCode(HRESULT hr) noexcept
{
	char* pMsgBuf = nullptr;
	// windows will allocate memory for err string and make our pointer point to it
	const DWORD nMsgLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
	);
	// 0 string length returned indicates a failure
	if (nMsgLen == 0)
	{
		return "Unidentified error code";
	}
	// copy error string from windows-allocated buffer to std::string
	std::string errorString = pMsgBuf;
	// free windows buffer
	LocalFree(pMsgBuf);
	return errorString;
}

namespace rn = std::ranges;
namespace vi = rn::views;

CheckerToken chk;

HrGrabber::HrGrabber(unsigned int hr, std::source_location loc) noexcept
	:
	hr(hr),
	loc(loc)
{}
void operator>>(HrGrabber g, CheckerToken)
{
	if (FAILED(g.hr)) {
		throw std::runtime_error{
			
			std::format(
				" {}:{}:{} \nHRESULT failed with error code : {}",
				g.loc.file_name(),
				g.loc.line(),
				g.loc.column(),
				TranslateErrorCode(g.hr)
			)
		};
	}
}
