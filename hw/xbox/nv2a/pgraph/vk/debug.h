/*
 * Geforce NV2A PGRAPH Vulkan Renderer
 *
 * Copyright (c) 2024 Matt Borgerson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HW_XBOX_NV2A_PGRAPH_VK_DEBUG_H
#define HW_XBOX_NV2A_PGRAPH_VK_DEBUG_H

#ifdef CONFIG_UWP
#include "qemu/qemu-host.h"
#endif

#define DEBUG_VK 0

extern int nv2a_vk_dgroup_indent;

#define NV2A_VK_XDPRINTF(x, fmt, ...)                                  \
    do {                                                               \
        if (x) {                                                       \
            fprintf(stderr, "%*s" fmt "\n", nv2a_vk_dgroup_indent, "", \
                    ##__VA_ARGS__);                                    \
        }                                                              \
    } while (0)

#define NV2A_VK_DPRINTF(fmt, ...) NV2A_VK_XDPRINTF(DEBUG_VK, fmt, ##__VA_ARGS__)

#define NV2A_VK_DGROUP_BEGIN(fmt, ...)                  \
    do {                                                \
        NV2A_VK_XDPRINTF(DEBUG_VK, fmt, ##__VA_ARGS__); \
        nv2a_vk_dgroup_indent++;                        \
    } while (0)

#define NV2A_VK_DGROUP_END(...)             \
    do {                                    \
        nv2a_vk_dgroup_indent--;            \
        assert(nv2a_vk_dgroup_indent >= 0); \
    } while (0)

#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult vk_result = (x);                                       \
        if (vk_result != VK_SUCCESS) {                                  \
            fprintf(stderr, "vk_result = %d\n", vk_result);             \
            /* stderr is not observable in an embedded UWP process. */  \
            /* Preserve the exact failing call in the host log. */      \
            do {                                                        \
                /* Keep desktop builds independent of the host API. */  \
                /* clang-format off */                                  \
                QEMU_HOST_VK_CHECK_LOG(#x, vk_result, __FILE__,          \
                                       __LINE__);                        \
                /* clang-format on */                                   \
            } while (0);                                                \
        }                                                               \
        assert(vk_result == VK_SUCCESS && "vk check failed");           \
    } while (0)

#ifdef CONFIG_UWP
static inline void qemu_host_vk_check_log(const char *expression,
                                          VkResult result,
                                          const char *file, int line)
{
    char message[512];
    snprintf(message, sizeof(message),
             "Vulkan/DZN call failed: %s returned %d at %s:%d",
             expression, result, file, line);
    qemu_host_emit_log(QEMU_HOST_LOG_ERROR, message);
}
#define QEMU_HOST_VK_CHECK_LOG(expr, result, file, line) \
    qemu_host_vk_check_log(expr, result, file, line)
#else
#define QEMU_HOST_VK_CHECK_LOG(expr, result, file, line) ((void)0)
#endif

void pgraph_vk_debug_frame_terminator(void);

#endif
