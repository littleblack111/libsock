#pragma once

#include <memory>

template <typename T>
using SP = std::shared_ptr<T>;

template <typename T>
using WP = std::weak_ptr<T>;

template <typename T>
using UP = std::unique_ptr<T>;

namespace sock {
class CFileDescriptor;
}
