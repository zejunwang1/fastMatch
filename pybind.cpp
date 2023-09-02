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

using MATCH = vector<pair<string, int>>;
using SEG = vector<string>;

PYBIND11_MAKE_OPAQUE(MATCH);
PYBIND11_MAKE_OPAQUE(SEG);

PYBIND11_MODULE(fast_match, m) {
  m.doc() = "Efficient exact string matching tool";
  py::bind_vector<MATCH>(m, "MATCH");
  py::bind_vector<SEG>(m, "SEG");
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
    .def("parse", &FastMatch::parseBind, py::arg("text"))
    .def("max_forward_match", (SEG (FastMatch::*)(const string&) const)
        (&FastMatch::maxForwardMatch), py::arg("text"));
}

