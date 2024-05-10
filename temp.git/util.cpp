//
// Created by Ludw on 4/25/2024.
//

#include "util.h"

std::vector<char> read_file(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open())
        throw std::runtime_error("failed to open file.");

    size_t file_size = (size_t) file.tellg();
    std::vector<char> buf(file_size);

    file.seekg(0);
    file.read(buf.data(), static_cast<std::streamsize>(file_size));

    file.close();

    return buf;
}

std::ostream &operator<<(std::ostream &os, const glm::vec3 &v) {
    os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
    return os;
}