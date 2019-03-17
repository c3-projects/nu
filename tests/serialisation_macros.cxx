#include "c3/nu/data.hpp"
#include "c3/nu/data/helpers.hpp"

using namespace c3;

class type_0 : public nu::static_serialisable<type_0> {
public:
  int bob;

public:
  type_0(int bob) : bob{bob} {}

public:
  C3_NU_DEFER_STATIC_SERIALISATION_VAR(type_0, bob)
};

class type_1 : public nu::static_serialisable<type_1> {
public:
  uint bob;

public:
  type_1(int bob) : bob{static_cast<uint>(bob)} {}
  operator int() const { return static_cast<uint>(bob); }

public:
  C3_NU_DEFER_STATIC_SERIALISATION_TYPE(type_1, uint)
};

class type_2 : public nu::serialisable<type_2> {
public:
  int bob;

public:
  type_2(int bob) : bob{bob} {}

public:
  C3_NU_DEFER_SERIALISATION_VAR(type_2, bob)
};

class type_3 : public nu::serialisable<type_3> {
public:
  float bob;

public:
  type_3(int bob) : bob{static_cast<float>(bob)} {}
  operator int() const { return static_cast<int>(bob); }

public:
  C3_NU_DEFER_SERIALISATION_TYPE(type_3, int)
};

class type_4 : public nu::static_serialisable<type_4> {
public:
  int bob;

public:
  type_4(int bob) : bob{bob} {}
  operator int() const { return bob; }

public:
  void _serialise_static(nu::data_ref b) const override {
    nu::serialise_static(bob, b);
  }
  C3_NU_DEFINE_STATIC_DESERIALISE(type_4, nu::serialised_size<float>(), b) {
    return { nu::deserialise<type_4>(b) };
  }
};

class type_5 : public nu::serialisable<type_4> {
public:
  int bob;

public:
  type_5(int bob) : bob{bob} {}
  operator int() const { return bob; }

public:
  nu::data _serialise() const override {
    return nu::serialise(bob);
  }
  C3_NU_DEFINE_DESERIALISE(type_4, b) {
    return { nu::deserialise<type_4>(b) };
  }
};


// If this compiles, it is successful
int main() { return 0; }

#include "c3/nu/data/clean_helpers.hpp"
