/**
 * Copyright (c) 2023-present, Zejun Wang.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <fastMatch.h>

namespace py = pybind11;

using width_type = uint_fast8_t;
using MATCH = vector<pair<string, int>>;
using SEG = vector<string>;

PYBIND11_MAKE_OPAQUE(MATCH);
PYBIND11_MAKE_OPAQUE(SEG);

width_type get_num_bytes_of_utf8_char(const char* str, int len) {
  int cur = 1;
  width_type num_bytes = 1;
  while (cur < len && (str[cur++] & 0xC0) == 0x80)
    num_bytes++;
  return num_bytes;
}

int match_bind(const string& text, const string& pattern) {
  int pos = match(text, pattern);
  if (pos < 0)
    return pos;
  int cur = 0, loc = 0;
  const char* str = text.c_str();
  while (cur < pos) {
    cur += get_num_bytes_of_utf8_char(str + cur, pos - cur);
    loc += 1;
  }
  return loc;
}

MATCH parse_bind(FastMatch& fastMatch, const string& text) {
  MATCH res = fastMatch.parse(text);
  int cur = 0, index = 0, loc = 0, pos = res.back().second;
  const char* str = text.c_str();
  while (cur < pos) {
    if (cur == res[index].second) {
      res[index].second = loc;
      ++index;
    }
    cur += get_num_bytes_of_utf8_char(str + cur, pos - cur);
    loc += 1;
  }
  res[index].second = loc;
  return res;
}

PYBIND11_MODULE(fast_match, m) {
  m.doc() = "Efficient exact string matching tool";
  py::bind_vector<MATCH>(m, "MATCH");
  py::bind_vector<SEG>(m, "SEG");
  m.def("match", &match_bind, py::arg("text"), py::arg("pattern"));
  py::class_<FastMatch>(m, "FastMatch")
    .def(py::init())
    .def(py::init<const string&, size_t>(), py::arg("path"), py::arg("capacity") = 0)
    .def(py::init<const vector<string>&>(), py::arg("key"))
    .def("size", &FastMatch::size)
    .def("num_keys", &FastMatch::num_keys)
    .def("insert", &FastMatch::insert, py::arg("key"))
    .def("remove", &FastMatch::remove, py::arg("key"))
    .def("get_key", &FastMatch::getKey, py::arg("id"))
    .def("get_value", &FastMatch::getValue, py::arg("key"))
    .def("hit", &FastMatch::hit, py::arg("text"))
    .def("parse", &parse_bind, py::arg("text"))
    .def("max_forward_match", (SEG (FastMatch::*)(const string&) const)
        (&FastMatch::maxForwardMatch), py::arg("text"));
}

