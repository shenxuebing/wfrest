﻿#include <type_traits>
#include "wfrest/SysInfo.h"

namespace wfrest
{

namespace CurrentThread
{
thread_local int t_cached_tid = 0;
thread_local char t_tid_str[32];
thread_local int t_tid_str_len = 6;
#ifdef OS_WINDOWS
#else
static_assert(std::is_same<int, pid_t>::value, "pid_t should be int");
#endif // OS_WINDOWS

}  // namespace CurrentThread
}  // namespace wfrest
