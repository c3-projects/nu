#pragma once

#include <string>
#include <regex>
#include <codecvt>
#include "c3/nu/data/hex.hpp"

namespace c3::nu {
  std::string cstr_encode(const std::string_view buf) {
    std::string ret;
    for (auto i : buf) {
      switch (i) {
        case ('\a'): ret.append(R"(\a)"); break;
        case ('\b'): ret.append(R"(\b)"); break;
        case ('\e'): ret.append(R"(\e)"); break;
        case ('\f'): ret.append(R"(\f)"); break;
        case ('\n'): ret.append(R"(\n)"); break;
        case ('\r'): ret.append(R"(\r)"); break;
        case ('\v'): ret.append(R"(\v)"); break;
        case ('\\'): ret.append(R"(\\)"); break;
        case ('\''): ret.append(R"(\')"); break;
        case ('\?'): ret.append(R"(\?)"); break;
        case ('\"'): ret.append(R"(\")"); break;

        case (' '): ret.push_back(' '); break;

        default: {
          if (std::isalnum(i))
            ret.push_back(i);
          else {
            ret.push_back('\\');
            char a[4] = { 0 };
            sprintf(a, "%03o", i);
            ret.append(a);
          }
        }
      }
    }
    return ret;
  }

  std::string cstr_decode(const std::string_view buf) {
    std::string ret;

    for (auto iter = buf.begin(); iter != buf.end(); ++iter) {
      switch (*iter) {
        case ('\\'): {
          switch (*++iter) {
            case ('a'): ret.push_back('\a'); break;
            case ('b'): ret.push_back('\b'); break;
            case ('e'): ret.push_back('\e'); break;
            case ('f'): ret.push_back('\f'); break;
            case ('n'): ret.push_back('\n'); break;
            case ('r'): ret.push_back('\r'); break;
            case ('v'): ret.push_back('\v'); break;
            case ('\\'): ret.push_back('\\'); break;
            case ('\''): ret.push_back('\''); break;
            case ('?'): ret.push_back('\?'); break;
            case ('"'): ret.push_back('\"'); break;
            case ('x'): {
              auto begin = ++iter;
              auto end = begin;
              do {
                if (++end == buf.end())
                  throw std::runtime_error("Invalid hex sequence in cstring");
              }
              while (std::isxdigit(*end));

              auto end_ptr = &const_cast<char&>(*end);
              if (auto i = std::strtol(&*begin, &end_ptr, 16); i >= 0)
                ret.push_back(i);
              else
                throw std::runtime_error("strtol failed");

              iter = end - 1;
            } break;
            case ('u') : {
              auto begin = ++iter;
              if (buf.end() - begin < 4)
                throw std::runtime_error("Invalid hex unicode sequence in cstring");
              auto end = begin + 4;

              auto end_ptr = &const_cast<char&>(*end);
              static std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> cvt;
              if (auto i = std::strtol(&*begin, &end_ptr, 16); i >= 0)
                ret.append(cvt.to_bytes(i));
              else
                throw std::runtime_error("strtol failed");

              iter = end;
            }
            case ('U') : {
              auto begin = ++iter;
              if (buf.end() - begin < 8)
                throw std::runtime_error("Invalid hex unicode sequence in cstring");
              auto end = begin + 8;

              auto end_ptr = &const_cast<char&>(*end);
              static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
              if (auto i = std::strtol(&*begin, &end_ptr, 16); i >= 0)
                ret.append(cvt.to_bytes(i));
              else
                throw std::runtime_error("strtol failed");

              iter = end;
            }
            default:
              if (*iter >= '0' && *iter <= '7') {
                auto begin = iter;
                auto end = begin;
                for (auto i = 0; i < 3 && end != buf.end(); ++i, ++end)
                  if (*end < '0' || *end > '7')
                    break;

                auto end_ptr = &const_cast<char&>(*end);

                ret.push_back(std::strtol(&*begin, &end_ptr, 8));

                iter = end - 1;
              }
              else
                throw std::runtime_error("Invalid escape sequence");
          }
        } break;

        default:
          ret.push_back(*iter);
      }
    }
    return ret;
  }
}

