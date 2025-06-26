export module libsock.misc;

import std;

export {
#define UNLIKELY(expr) (expr) [[unlikely]]
#define LIKELY(expr)   (expr) [[likely]]
}

export namespace LibSock {
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

template <typename Func>
struct Then {
	Func func;
};

template <typename Func>
Then<Func> then(Func f) {
	return Then<Func>{std::move(f)};
}

template <typename T, typename Func>
auto operator|(std::future<T> &fut, Then<Func> thenObj) -> std::future<std::invoke_result_t<Func, T>> {
	return then(std::move(fut), std::move(thenObj.func));
}

template <typename Func>
auto operator|(std::future<void> &fut, Then<Func> thenObj) -> std::future<std::invoke_result_t<Func>> {
	return then(std::move(fut), std::move(thenObj.func));
}
} // namespace LibSock
