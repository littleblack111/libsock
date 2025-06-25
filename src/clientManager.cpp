#include "libsock/clientManager.hpp"
#include "libsock/client.hpp"
#include <algorithm>
#include <functional>
#include <ranges>
#include <unistd.h>
#include <utility>

using namespace LibSock;

template <typename T>
void ClientManager<T>::shutdownClientManager(std::optional<std::function<bool(const std::vector<std::pair<std::jthread, SP<Client<T>>>> &)>> cb) {
	if (m_vClientManager.empty())
		return;

	if (cb && !(*cb)(m_vClientManager))
		return;

	for (auto &[thread, client] : m_vClientManager) {
		kick(WP<Client<T>>(client));
		if (thread.joinable())
			client->m_wait ? thread.join() : thread.detach();
	}

	m_vClientManager.clear();
}

template <typename T>
SP<ClientManager<T>> ClientManager<T>::make(WP<Abstract::IServer<T>> server) {
	auto c		  = SP<ClientManager>(new ClientManager());
	c->m_self	  = c;
	c->m_wpServer = server;
	return c;
}

template <typename T>
WP<ClientManager<T>> ClientManager<T>::get() {
	if (m_self.expired())
		m_self = this->shared_from_this();
	return m_self;
}

template <typename T>
ClientManager<T>::~ClientManager() {
	shutdownClientManager();
}

template <typename T>
std::pair<SP<Client<T>>, std::future<void>> ClientManager<T>::createClient(bool track, bool wait, std::function<void(SP<Client<T>> &)> cb) {
	auto promise = std::make_shared<std::promise<void>>();
	SP	 client	 = SP<Client<T>>(new Client(m_wpServer.lock(), this->shared_from_this(), track, wait));
	m_vClientManager.emplace_back(std::jthread([client, cb, promise]() mutable {
									  client->init();
									  promise->set_value();
									  cb(client);
								  }),
								  client);
	client->m_self = WP<Client<T>>(client);
	return {client, promise->get_future()};
}

template <typename T>
void ClientManager<T>::broadcast(const SData<T> &msg) {
	for (const auto &[thread, client] : m_vClientManager) {
		if (!client)
			continue;

		client->write(msg.msg);
	}

	m_vDatas.emplace_back(msg);
}

template <typename T>
SP<Client<T>> ClientManager<T>::getByIp(const std::string &ip) const {
	auto it = std::ranges::find_if(m_vClientManager, [&ip](const auto &s) { return s.second->getIp() == ip; });
	return it != m_vClientManager.end() ? it->second : nullptr;
}

template <typename T>
std::vector<SP<Client<T>>> ClientManager<T>::getClientManager() const {
	return m_vClientManager | std::views::transform([](const auto &s) { return s.second; }) | std::ranges::to<std::vector>();
}

template <typename T>
std::vector<SData<T>> ClientManager<T>::getDatas() const {
	return m_vDatas;
}

template <typename T>
void ClientManager<T>::kick(WP<Client<T>> clientWeak, const bool kill, std::optional<std::function<bool(const typename std::vector<std::pair<std::jthread, SP<Client<T>>>>::iterator &)>> cb) {
	auto client = clientWeak.lock();
	if (!client)
		return;

	for (auto it = m_vClientManager.begin(); it != m_vClientManager.end(); ++it) {
		if (it->second == client) {
			if (client)
				// this only exist when the client is registered
				// not sure why the second is null
				if (cb && !(*cb)(it))
					return;

			const auto native_handle = it->first.native_handle();
			if (it->first.joinable())
				it->second->m_wait ? it->first.join() : it->first.detach();
			it->second.reset();
			m_vClientManager.erase(it);
			if (kill && native_handle != 0)
				// cancel(terminal/kill) the thread if it doesn't exit on its own
				// pthread_cancel cuz it's the safest https://stackoverflow.com/a/3438576
				// need a condition because it might crash if the thread is already dead
				pthread_cancel(native_handle);
			return;
		}
	}
}
