#pragma once
#ifndef VULKAN_HEADER_H
#define VULKAN_HEADER_H
#define VK_NO_PROTOTYPES
#ifdef L_PLATFORM_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#include <volk.h>
#include <vk_mem_alloc.h>
#endif
#endif