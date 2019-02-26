#include "c3/nu/concurrency/cancellable.hpp"

#include <thread>

using namespace std::chrono_literals;

int main() {
  c3::nu::cancellable_provider<uint64_t> provider_0;
  c3::nu::cancellable_provider<uint64_t> provider_1;
  c3::nu::cancellable_provider<uint64_t> provider_2;
  c3::nu::cancellable_provider<uint64_t> provider_3;
  c3::nu::cancellable_provider<uint64_t> provider_4;
  c3::nu::cancellable_provider<std::unique_ptr<uint64_t>> provider_5;
  c3::nu::cancellable_provider<std::unique_ptr<uint64_t>> provider_6;

  c3::nu::gateway_bool got_first_datum;
  c3::nu::gateway_bool got_second_datum;
  c3::nu::gateway_bool provider_3_update_ensurer;

  std::thread{[&] {
    std::this_thread::yield();
    std::this_thread::sleep_for(200ms);
    provider_0.maybe_update([] { return 999; });
    provider_0.maybe_update([] { return 420; });
    got_first_datum.wait_for_open();

    provider_1.provide(69);
    got_second_datum.wait_for_open();

    if (!provider_2.is_cancelled())
      throw std::runtime_error("Failed to cancel!");

    provider_3.update(3);
    provider_3.maybe_modify([](auto& i) { i.value() += 3; });

    provider_4.provide(180);

    provider_5.provide(std::make_unique<uint64_t>(10));
    provider_6.provide(std::make_unique<uint64_t>(36));
  }}.detach();

  auto consumer_0 = provider_0.get_cancellable();
  auto consumer_1 = provider_1.get_cancellable();
  auto consumer_2 = provider_2.get_cancellable();
  auto consumer_3 = provider_3.get_cancellable();
  auto consumer_4 = provider_4.get_cancellable();
  auto consumer_5 = provider_5.get_cancellable();
  auto consumer_6 = provider_6.get_cancellable();
  auto consumer_7 = c3::nu::cancellable<uint64_t>::external([]() { return 64; });

  if (consumer_0.try_take())
    throw std::runtime_error("Early provision");

  if (consumer_0.wait(1s)) {
    if (consumer_0.try_take() != 420)
      throw std::runtime_error("Corrupted update");
  }
  else throw std::runtime_error("Failed to update");

  got_first_datum.open();

  auto consumer_1_plus_1 = consumer_1.map<decltype(consumer_1)::provided_t>([](auto x) { return x + 1; });

  if (consumer_1_plus_1.wait(20ms)) {
    if (consumer_1_plus_1.try_take() != 70)
      throw std::runtime_error("Corrupted mapped provision");
  }
  else throw std::runtime_error("Failed to get mapped consumer provision");

  if (consumer_2.wait(20ms)) {
    throw std::runtime_error("Fake provision");
  }

  // Cancel consumer_2
  consumer_2.finalise();

  got_second_datum.open();

  if (consumer_3.try_take(1s) != 6)
    throw std::runtime_error("Failed mapped provision");

  c3::nu::gateway_bool consumed_4;

  consumer_4.take_on_complete([&](auto i) {
    if (i != 180)
      throw std::runtime_error("Failed take_on_complete provision");
    consumed_4.open();
  });

  consumed_4.wait_for_open();

  consumer_5.take_on_complete([&](auto i) {
    if (*i != 10)
      throw std::runtime_error("Failed move unique_ptr provision");
  });

  auto consumer_6_plus_1 = consumer_6.map<std::unique_ptr<uint32_t>>([](auto x) { return std::make_unique<uint32_t>(*x + 1); });
  consumer_6_plus_1.take_on_complete([&](std::unique_ptr<uint32_t> i) {
    if (*i != 37)
      throw std::runtime_error("Failed move unique_ptr provision");
  });

  consumer_7.finalise();
  consumer_7.take_on_complete([&](auto i) {
    if (i != 64)
      throw std::runtime_error("Failed external provision");
  });

  return 0;
}
