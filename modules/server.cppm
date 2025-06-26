export module libsock.server;

import libsock.interfaces;
import std;

export namespace LibSock::Server {
class Server : public Abstract::IServer {
  private:
	void init(std::optional<sockaddr_in> addr = std::nullopt) override;
};
} // namespace LibSock::Server

export namespace LibSock::Client {
class Server : public Abstract::IServer {
  private:
	void init(std::optional<sockaddr_in> addr = std::nullopt) override;
};
} // namespace LibSock::Client
