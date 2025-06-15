#include "libsock/server/client.hpp"
#include <algorithm>
#include <format>
#include <ranges>
#include <unistd.h>
#include <utility>

using namespace LibSock::Server;

void Clients::shutdownClients() {
	if (m_vClients.empty())
		return;

	for (auto &[thread, client] : m_vClients) {
		if (client) {
			client->write("Server shutting down");
			if (!client->getName().empty())
				broadcast(std::format("{} has left the chat", client->getName()), WP<Client>(client));
		}
	}

	for (auto &[thread, client] : m_vClients) {
		if (thread.joinable())
			thread.detach();
		client.reset();
	}

	m_vClients.clear();
}

Clients::Clients() {
	// if (std::atexit([&]() {
	// 		shutdownClients();
	// 	}))
	// 	void(); // failed to register exit handler...
	pClients = SP<Clients>(this);
};

Clients::~Clients() {
	for (auto &[thread, client] : m_vClients) {
		if (thread.joinable())
			thread.detach();
		kick(WP<Client>(client));
	}
}

void Clients::newClient(std::optional<std::function<void()>> cb) {
	if (cb)
		(*cb)();
	SP	  client		  = std::make_shared<Client>(pServer, SP<Clients>(this));
	auto &instance		  = m_vClients.emplace_back(std::jthread([client]() { client->run(); }), client);
	instance.second->self = std::weak_ptr<Client>(instance.second);
}

void Clients::run(std::optional<std::function<void()>> cb) {
	while (true)
		newClient(cb);
}

void Clients::broadcast(const std::string &msg, std::optional<std::weak_ptr<Client>> self) {
	broadcast(SData{msg, self, false});
}

void Clients::broadcast(const SData &msg) {
	for (const auto &[thread, client] : m_vClients) {
		if (!client)
			continue;

		client->write(msg.msg);
	}

	m_vDatas.push_back(msg);
}

bool Clients::nameExists(const std::string &name) {
	auto it = std::ranges::find_if(m_vClients, [&name](const auto &s) { return s.second->getName() == name; });

	if (it != m_vClients.end()) {
		if (!it->second->isValid()) {
			kick(std::weak_ptr<Client>(it->second), true, "Connection lost");
			return false;
		}
		return true;
	}
	return false;
}

SP<Client> Clients::getByName(const std::string &name) const {
	auto it = std::ranges::find_if(m_vClients, [&name](const auto &s) { return s.second->getName() == name; });
	return it != m_vClients.end() ? it->second : nullptr;
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

void Clients::kick(WP<Client> clientWeak, const bool kill, const std::string &reason) {
	auto client = clientWeak.lock();
	if (!client)
		return;

	for (auto it = m_vClients.begin(); it != m_vClients.end(); ++it) {
		if (it->second == client) {
			if (client && !client->getName().empty())
				// this only exist when the client is registered
				// not sure why the second is null, but m_name definately is since it's setted during register
				// but we don't need to notify people if the client didn't even "join"/register anyways
				broadcast(std::format("{} has left the chat", client->getName()), clientWeak);

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
