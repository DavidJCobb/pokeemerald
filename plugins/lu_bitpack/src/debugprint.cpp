#include "debugprint.h"
#include <iostream>

extern void lu_debugprint(std::string_view text) {
   std::cerr << text;
}