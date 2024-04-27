//
// Created by Ludw on 4/25/2024.
//

#include "../app.h"

VkVertexInputBindingDescription Vertex::get_binding_desc() {
    VkVertexInputBindingDescription binding_desc{};
    binding_desc.binding = 0;
    binding_desc.stride = sizeof(Vertex);
    binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_desc;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::get_attrib_descs() {
    std::array<VkVertexInputAttributeDescription, 2> attrib_descs{};

    attrib_descs[0].binding = 0;
    attrib_descs[0].location = 0;
    attrib_descs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attrib_descs[0].offset = offsetof(Vertex, pos);

    attrib_descs[1].binding = 0;
    attrib_descs[1].location = 1;
    attrib_descs[1].format = VK_FORMAT_R32G32_SFLOAT;
    attrib_descs[1].offset = offsetof(Vertex, uv);

    return attrib_descs;
}
