#include "c3/nu/concurrency/cancellable.hpp"

#include <thread>

using namespace std::chrono_literals;

int main() {
  c3::nu::cancellable_provider<uint64_t> provider_0;
  c3::nu::cancellable_provider<uint64_t> provider_1;
  c3::nu::cancellable_provider<uint64_t> provider_2;

  c3::nu::gateway_bool got_first_datum;
  c3::nu::gateway_bool got_second_datum;

  std::thread{[&] {
    std::this_thread::yield();
    std::this_thread::sleep_for(200ms);
    provider_0.maybe_update([] { return 999; });
    provider_0.maybe_update([] { return 420; });
    got_first_datum.wait_for_open(1s);

    provider_1.maybe_provide([] { return 69; });
    got_second_datum.wait_for_open(1s);

    if (!provider_2.is_cancelled())
      throw std::runtime_error("Failed to cancel!");
  }}.detach();

  auto consumer_0 = provider_0.get_cancellable();
  auto consumer_1 = provider_1.get_cancellable();
  auto consumer_2 = provider_2.get_cancellable();

  if (consumer_0.try_get())
    throw std::runtime_error("Early provision");

  if (auto x = consumer_0.get_or_cancel(1s)) {
    if (*x != 420)
      throw std::runtime_error("Corrupted update");
  }
  else throw std::runtime_error("Failed to update");

  got_first_datum.open();

  if (auto x = consumer_1.get_or_cancel(1s)) {
    if (*x != 69)
      throw std::runtime_error("Corrupted provision");
  }
  else throw std::runtime_error("Failed to provide");

  if (auto x = consumer_1.get_or_cancel(1s)) {
    if (*x != 69)
      throw std::runtime_error("Corrupted second provision");
  }
  else throw std::runtime_error("Failed to get second provision");

  if (consumer_2.get_or_cancel(20ms)) {
    throw std::runtime_error("Fake provision");
  }

  got_second_datum.open();

  return 0;
}
