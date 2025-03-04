#include "pragma_parse_exception.h"

pragma_parse_exception::pragma_parse_exception(location_t loc, const std::string& what) : std::runtime_error(what), location(loc) {}