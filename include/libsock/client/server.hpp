#include "../interfaces/IServer.hpp"

namespace LibSock::Client {
class Server : public Abstract::IServer {
  private:
	void init(std::optional<sockaddr_in> addr = std::nullopt) override;
};
} // namespace LibSock::Client
