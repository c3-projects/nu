#include "c3/nu/concurrency/cancellable.hpp"

#include <thread>

using namespace std::chrono_literals;

int main() {
  c3::nu::cancellable_provider<uint64_t> provider_0;
  c3::nu::cancellable_provider<uint64_t> provider_1;

  std::thread{[&] {
    std::this_thread::yield();
    std::this_thread::sleep_for(20ms);
    provider_0.maybe_provide([] { return 69420; });
    std::this_thread::sleep_for(200ms);
    if (!provider_1.is_cancelled())
      throw std::runtime_error("Failed to cancel!");
  }}.detach();

  auto consumer_0 = provider_0.get_cancellable();
  auto consumer_1 = provider_1.get_cancellable();

  if (consumer_0.try_get())
    throw std::runtime_error("Early provision");

  if (consumer_1.get_or_cancel(20ms)) {
    throw std::runtime_error("Fake provision");
  }

  if (auto x = consumer_0.get_or_cancel(1s)) {
    if (*x != 69420)
      throw std::runtime_error("Corrupted provision");
  }
  else throw std::runtime_error("Failed to provide!");

  return 0;
}
