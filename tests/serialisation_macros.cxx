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
  float bob;

public:
  type_1(int bob) : bob{static_cast<float>(bob)} {}
  operator int() const { return static_cast<int>(bob); }

public:
  C3_NU_DEFER_STATIC_SERIALISATION_TYPE(type_1, int)
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

// If this compiles, it is successful
int main() { return 0; }

#include "c3/nu/data/clean_helpers.hpp"
