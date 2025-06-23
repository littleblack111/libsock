#include <future>
#include <type_traits>

namespace LibSock {
template <typename T, typename Func>
auto then(std::future<T> fut, Func func) -> std::future<std::invoke_result_t<Func, T>> {
	return std::async(std::launch::async, [fut = std::move(fut), func = std::move(func)]() mutable {
		return func(std::move(fut.get()));
	});
}

template <typename Func>
auto then(std::future<void> fut, Func func) -> std::future<std::invoke_result_t<Func>> {
	return std::async(std::launch::async, [fut = std::move(fut), func = std::move(func)]() mutable {
		fut.wait();
		return func();
	});
}
} // namespace LibSock
