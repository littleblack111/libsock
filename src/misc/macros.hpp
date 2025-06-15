#pragma once

#define UNLIKELY(expr) (expr) [[unlikely]]
#define LIKELY(expr)   (expr) [[likely]]
