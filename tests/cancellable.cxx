#include "c3/nu/concurrency/cancellable.hpp"

#include <thread>

using namespace std::chrono_literals;

int main() {
  c3::nu::cancellable_provider<uint64_t> provider_0;
  c3::nu::cancellable_provider<uint64_t> provider_1;
  c3::nu::cancellable_provider<uint64_t> provider_2;
  c3::nu::cancellable_provider<uint64_t> provider_3;

  c3::nu::gateway_bool got_first_datum;
  c3::nu::gateway_bool got_second_datum;

  std::thread{[&] {
    std::this_thread::yield();
    std::this_thread::sleep_for(200ms);
    provider_0.maybe_update([] { return 999; });
    provider_0.maybe_update([] { return 420; });
    got_first_datum.wait_for_open();

    provider_1.maybe_provide([] { return 69; });
    got_second_datum.wait_for_open();

    if (!provider_2.is_cancelled())
      throw std::runtime_error("Failed to cancel!");

    auto provider_3_mapped = provider_3.map<std::string>([](auto s) { return s.length(); });
    provider_3_mapped.maybe_provide([]() { return "foobar"; });
  }}.detach();

  auto consumer_0 = provider_0.get_cancellable();
  auto consumer_1 = provider_1.get_cancellable();
  auto consumer_2 = provider_2.get_cancellable();
  auto consumer_3 = provider_3.get_cancellable();

  if (consumer_0.try_get())
    throw std::runtime_error("Early provision");

  if (consumer_0.wait(1s)) {
    if (consumer_0.try_get() != 420)
      throw std::runtime_error("Corrupted update");
  }
  else throw std::runtime_error("Failed to update");

  got_first_datum.open();

  if (consumer_1.wait(1s)) {
    if (consumer_1.try_get() != 69)
      throw std::runtime_error("Corrupted provision");
  }
  else throw std::runtime_error("Failed to provide");

  if (consumer_1.wait(1s)) {
    if (consumer_1.try_get() != 69)
      throw std::runtime_error("Corrupted second provision");
  }
  else throw std::runtime_error("Failed to get second provision");

  auto consumer_1_plus_1 = consumer_1.map<decltype(consumer_1)::provided_t>([](auto x) { return x + 1; });

  if (consumer_1_plus_1.wait(20ms)) {
    if (consumer_1_plus_1.try_get() != 70)
      throw std::runtime_error("Corrupted mapped provision");
  }
  else throw std::runtime_error("Failed to get mapped consumer provision");

  if (consumer_1_plus_1.wait(20ms)) {
    if (consumer_1_plus_1.try_take() != 70)
      throw std::runtime_error("Corrupted mapped provision");
  }
  else throw std::runtime_error("Failed to take mapped consumer provision");

  if (consumer_2.wait(20ms)) {
    throw std::runtime_error("Fake provision");
  }

  // Cancel consumer_2
  consumer_2.finalise();

  got_second_datum.open();

  if (consumer_3.try_take(1s) != std::string("foobar").length())
    throw std::runtime_error("Failed mapped provision");

  return 0;
}
