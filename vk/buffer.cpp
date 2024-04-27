//
// Created by Ludw on 4/25/2024.
//

#include "../app.h"

uint32_t App::find_mem_type(uint32_t type_filter, VkMemoryPropertyFlags mem_flags) {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(phy_dev, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_props.memoryTypes[i].propertyFlags & mem_flags) == mem_flags) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type.");
}

VCW_Buffer App::create_buf(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_props) {
    VCW_Buffer buf;
    buf.size = size;

    VkBufferCreateInfo buf_info{};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.size = buf.size;
    buf_info.usage = usage;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(dev, &buf_info, nullptr, &buf.buf) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer.");

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(dev, buf.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_mem_type(mem_reqs.memoryTypeBits, mem_props);

    if (vkAllocateMemory(dev, &alloc_info, nullptr, &buf.mem) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate buffer memory.");

    vkBindBufferMemory(dev, buf.buf, buf.mem, 0);

    return buf;
}

void App::map_buf(VCW_Buffer *p_buf) {
    vkMapMemory(dev, p_buf->mem, 0, p_buf->size, 0, &p_buf->p_mapped_mem);
}

void App::unmap_buf(VCW_Buffer *p_buf) {
    vkUnmapMemory(dev, p_buf->mem);
    p_buf->p_mapped_mem = NULL;
}

void App::cp_data_to_buf(VCW_Buffer *p_buf, void *p_data) {
    map_buf(p_buf);
    memcpy(p_buf->p_mapped_mem, p_data, p_buf->size);
    unmap_buf(p_buf);
}

// allocates new command buffers
void App::cp_buf(VCW_Buffer src_buf, VCW_Buffer dst_buf) {
    if (!(src_buf.size == dst_buf.size))
        throw std::runtime_error("source buffer size is not destination buffer size.");

    VkCommandBuffer cmd_buf = begin_single_time_cmd();

    VkBufferCopy cp_region{};
    cp_region.size = src_buf.size;
    vkCmdCopyBuffer(cmd_buf, src_buf.buf, dst_buf.buf, 1, &cp_region);

    end_single_time_cmd(cmd_buf);
}

void App::clean_up_buf(VCW_Buffer buf) {
    vkDestroyBuffer(dev, buf.buf, nullptr);
    vkFreeMemory(dev, buf.mem, nullptr);
}
