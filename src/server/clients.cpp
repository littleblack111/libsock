#include "libsock/server/clients.hpp"
#include "libsock/server/client.hpp"
#include <algorithm>
#include <any>
#include <ranges>
#include <stdexcept>
#include <unistd.h>
#include <utility>

using namespace LibSock::Server;

namespace LibSock {
namespace Server {

void Clients::shutdownClients(std::optional<std::function<std::any(const std::vector<std::pair<std::jthread, SP<Client>>> &)>> cb) {
	if (m_vClients.empty())
		return;

	if (cb)
		(*cb)(m_vClients);

	for (auto &[thread, client] : m_vClients) {
		if (thread.joinable())
			thread.detach();
		client.reset();
	}

	m_vClients.clear();
}

Clients::Clients() {
	if (std::atexit([]() {
			if (pClients)
				pClients->shutdownClients();
		}))
		void(); // failed to register exit handler...
}

SP<Clients> Clients::create() {
	if (pClients)
		throw std::runtime_error("server:clients ClientManager already exist");

	pClients = SP<Clients>(new Clients());
	return pClients;
}

SP<Clients> Clients::get() {
	if (!pClients)
		pClients = shared_from_this();
	return pClients;
}

Clients::~Clients() {
	for (auto &[thread, client] : m_vClients) {
		if (thread.joinable())
			thread.detach();
		kick(WP<Client>(client));
	}
}

WP<Client> Clients::newClient(std::function<std::any(const WP<Client> &)> loopFunc, SP<Server> server) {
	SP	  client		  = SP<Client>(new Client(server, shared_from_this()));
	auto &instance		  = m_vClients.emplace_back(std::jthread([&client, &loopFunc]() { loopFunc(client); }), client);
	instance.second->self = std::weak_ptr<Client>(instance.second);
	return instance.second;
}

void Clients::broadcast(const std::string &msg, std::optional<std::weak_ptr<Client>> self) {
	broadcast(SData{msg, self});
}

void Clients::broadcast(const SData &msg) {
	for (const auto &[thread, client] : m_vClients) {
		if (!client)
			continue;

		client->write(msg.msg);
	}

	m_vDatas.push_back(msg);
}

SP<Client> Clients::getByIp(const std::string &ip) const {
	auto it = std::ranges::find_if(m_vClients, [&ip](const auto &s) { return s.second->getIp() == ip; });
	return it != m_vClients.end() ? it->second : nullptr;
}

std::vector<SP<Client>> Clients::getClients() const {
	return m_vClients | std::views::transform([](const auto &s) { return s.second; }) | std::ranges::to<std::vector>();
}

std::vector<SData> Clients::getDatas() const {
	return m_vDatas;
}

void Clients::kick(WP<Client> clientWeak, const bool kill, const std::string &reason, std::optional<std::function<std::any(const std::vector<std::pair<std::jthread, SP<Client>>>::iterator &)>> cb) {
	auto client = clientWeak.lock();
	if (!client)
		return;

	for (auto it = m_vClients.begin(); it != m_vClients.end(); ++it) {
		if (it->second == client) {
			if (client)
				// this only exist when the client is registered
				// not sure why the second is null
				if (cb)
					(*cb)(it);

			if (!reason.empty())
				client->write(reason);

			const auto native_handle = it->first.native_handle();
			if (it->first.joinable())
				it->first.detach(); // .detach the thread since it's removing itself
			it->second.reset();
			m_vClients.erase(it);
			if (kill && native_handle != 0)
				// cancel(terminal/kill) the thread if it doesn't exit on its own
				// pthread_cancel cuz it's the safest https://stackoverflow.com/a/3438576
				// need a condition because it might crash if the thread is already dead
				pthread_cancel(native_handle);
			return;
		}
	}
}

} // namespace Server
} // namespace LibSock
