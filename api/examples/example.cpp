#include <memory>

#include "lazarus/lazarus.h"

// developer can use the library by including the header file
using namespace lazarus;
int main(int argc, const char** argv) {
  auto client = std::make_shared<LazarusClient>();
  client->get_hosts();
  return 0;
}