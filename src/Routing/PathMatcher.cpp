#include "Routing/PathMatcher.h"

namespace FalconHTTP::Routing {

// Core API
bool PathMatcher::match(
	std::string_view pattern,
	std::string_view path,
	HashMap<std::string, std::string>& params
) {
	while (!pattern.empty() && !path.empty()) {
		std::size_t patternSlash = pattern.find('/', 1);
		std::size_t pathSlash    = path.find('/', 1);

		std::string_view patternSegment = pattern.substr(0, patternSlash == std::string_view::npos ? pattern.size() : patternSlash);
		std::string_view pathSegment    = path.substr(0, pathSlash == std::string_view::npos ? path.size() : pathSlash);

		if (patternSegment.size() > 1 && patternSegment[1] == ':') {
			std::string paramName(patternSegment.substr(2));
			std::string paramValue(pathSegment.substr(1));

			if (!params.update(paramName, paramValue)) {
				params.insert(paramName, paramValue);
			}
		} else if (patternSegment != pathSegment) {
			return false;
		}

		pattern = (patternSlash == std::string_view::npos) ? pattern.substr(pattern.size()) : pattern.substr(patternSlash);
		path    = (pathSlash    == std::string_view::npos) ? path.substr(path.size())       : path.substr(pathSlash);
	}

	return pattern.empty() && path.empty();
}

} // namespace FalconHTTP::Routing