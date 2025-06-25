#pragma once

#include "interfaces/IServer.hpp"
#include "types.hpp"
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace LibSock {

template <typename T>
class Client;

template <typename T>
struct SData {
	std::string					 msg;
	std::optional<WP<Client<T>>> sender = std::nullopt;
};

template <typename T>
class ClientManager : public std::enable_shared_from_this<ClientManager<T>> {
  public:
	static SP<ClientManager> make(WP<Abstract::IServer<T>> server);
	~ClientManager();

	std::pair<SP<Client<T>>, std::future<void>> createClient(bool track = true, bool wait = false, std::function<void(SP<Client<T>> &)> cb = [](SP<Client<T>>) {});

	void broadcast(const SData<T> &msg); // second param only specified when we
										 // want to exclude the sender
	void kick(WP<Client<T>> client, const bool kill = false, std::optional<std::function<bool(const typename std::vector<std::pair<std::jthread, SP<Client<T>>>>::iterator &)>> cb = std::nullopt);
	void addClient(const Client<T> &client);

	void shutdownClientManager(std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client<T>>>> &)>> cb = std::nullopt);

	SP<Client<T>>			   getByIp(const std::string &ip) const;
	std::vector<SP<Client<T>>> getClientManager() const;
	std::vector<SData<T>>	   getDatas() const;

  private:
	std::vector<std::pair<std::jthread, SP<Client<T>>>> m_vClientManager;
	std::vector<SData<T>>								m_vDatas;

	WP<ClientManager> get();

	WP<ClientManager>		 m_self;
	WP<Abstract::IServer<T>> m_wpServer;

	friend class Client<T>;
};
} // namespace LibSock
