//
// Created by Ludw on 4/25/2024.
//

#include "inc.h"

#ifndef VCW_UTIL_H
#define VCW_UTIL_H

std::vector<char> read_file(const std::string &filename);

std::ostream& operator<<(std::ostream& os, const glm::vec3& v);

#endif //VCW_UTIL_H
