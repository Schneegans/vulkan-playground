////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_STL_HELPERS_HPP
#define ILLUSION_STL_HELPERS_HPP

// ---------------------------------------------------------------------------------------- includes
#include <boost/algorithm/string/replace.hpp>

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

namespace Illusion {

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
std::unique_ptr<T> makeUnique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
T fromString(std::string const& v) {
  std::istringstream iss(v);
  T                  result;
  iss >> result;
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::string toString(T const& v) {
  std::ostringstream oss;
  oss << v;
  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::string toEscapedString(T const& v) {
  return toString(v);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline std::string toEscapedString(std::string const& v) {
  std::string escaped;
  escaped.reserve(v.size());
  for (size_t pos = 0; pos != v.size(); ++pos) {
    switch (v[pos]) {
    case '&':
      escaped.append("&amp;");
      break;
    case '\"':
      escaped.append("&quot;");
      break;
    case '\'':
      escaped.append("&apos;");
      break;
    case '<':
      escaped.append("&lt;");
      break;
    case '>':
      escaped.append("&gt;");
      break;
    case '\\':
      escaped.append("\\\\");
      break;
    case '\r':
      escaped.append("\\r");
      break;
    case '\n':
      escaped.append("\\n");
      break;
    default:
      escaped.append(&v[pos], 1);
      break;
    }
  }

  return "\"" + escaped + "\"";
}

////////////////////////////////////////////////////////////////////////////////////////////////////

inline std::string toEscapedString(char const* v) { return toEscapedString(std::string(v)); }

////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<std::string> splitString(std::string const& s, char delim);

////////////////////////////////////////////////////////////////////////////////////////////////////

bool stringContains(std::string const& s, char c);

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
struct Identity {
  typedef T type;
};

template <typename T>
static std::shared_ptr<T>
makeVulkanPtr(T const& vkObject, typename Identity<std::function<void(T* obj)>>::type deleter) {
  return std::shared_ptr<T>(new T{vkObject}, deleter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}

namespace std {

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::ostream& operator<<(std::ostream& os, std::vector<T> const& v) {

  typename std::vector<T>::const_iterator i(v.begin());
  while (i != v.end()) {
    os << *i;

    if (++i != v.end()) { os << " "; }
  }
  return os;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::istream& operator>>(std::istream& is, std::vector<T>& v) {
  v.clear();

  T new_one;
  while (is >> new_one) {
    v.push_back(new_one);
  }

  is.clear();

  return is;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
}
#endif // ILLUSION_STL_HELPERS_HPP
