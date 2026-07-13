#pragma once

#include <string_view>
#include <sys/types.h>

namespace ipc::config
{
inline constexpr std::string_view queue_name = "/mq_processor_to_sink";

inline constexpr mode_t QUEUE_PERMS = 0660;
inline constexpr std::size_t MAX_MSG_NUM = 10;
inline constexpr std::size_t MAX_MSG_SIZE = 512;
} // namespace ipc::config