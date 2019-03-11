#include "c3/nu/data/structs/xml.hpp"

#include <iostream>
#include <fstream>

using namespace c3::nu;

int main() {
  markup_struct html("html");
  {
    auto& body = html.add_elem("body");
    body.add(
      markup_struct{"h1", markup_struct::value_tag, "Hello, World!"},
      markup_struct::value_tag, "Foobar!"
    );
  }

  std::cout << xml_encode(html) << std::endl;
}

