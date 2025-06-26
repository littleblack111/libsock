export module libsock.types;

import std;
export import libsock.fileDescriptor;

export template <typename T>
using SP = std::shared_ptr<T>;

export template <typename T>
using WP = std::weak_ptr<T>;

export template <typename T>
using UP = std::unique_ptr<T>;
