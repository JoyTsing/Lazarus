#include "dns/dns_client.h"

#include <thread>

#include "utils/minilog.h"

void start_dns_client() {
  std::jthread([]() { handle_dns(); }).detach();
}

void handle_dns() { minilog::log_info("dns client start..."); }