#include "GraphicsError.h"
#include <ranges>
#include <format>
#include <winerror.h>

namespace rn = std::ranges;
namespace vi = rn::views;



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
				"{}:{}:{}: HRESULT failed with error code {}",
				g.loc.file_name(),
				g.loc.line(),
				g.loc.column(),
				g.hr
			)
		};
	}
}
