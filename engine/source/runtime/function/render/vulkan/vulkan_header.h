#pragma once
#define VULKAN_HEADER_H
#ifndef VK_NO_PROTOTYPES
#define VK_NO_PROTOTYPES
#endif
#ifdef L_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include <vma/vk_mem_alloc.h>
#endif