#include "c3/nu/data/structs/xml.hpp"

#include <iostream>
#include <fstream>

using namespace c3::nu;

int main() {
  markup_struct html("html");
  {
    auto& body = html.add_elem("body");
    body.add(
      markup_struct{"h1", markup_struct::value, "Hello, World!"},
      markup_struct::value, "Foobar!",
      markup_struct{"div",
        markup_struct::attr, "style", "color:red",
        markup_struct::value, "wibble"
      }
    );
  }

  auto buf = xml_encode(html);

  auto html_ = xml_decode(buf);
}

