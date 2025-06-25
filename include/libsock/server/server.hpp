#include "../interfaces/IServer.hpp"

namespace LibSock::Server {
class Server : public Abstract::IServer<Server> {
  private:
	void init(std::optional<sockaddr_in> addr = std::nullopt) override;
};
} // namespace LibSock::Server
