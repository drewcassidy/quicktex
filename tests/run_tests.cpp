/*  Quicktex Texture Compression Library
    Copyright (C) 2021 Andrew Cassidy <drewcassidy@me.com>
    Partially derived from rgbcx.h written by Richard Geldreich <richgel99@gmail.com>
    and licenced under the public domain

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// This file allows for easy debugging in CLion or other IDEs that dont natively support cross-debugging between Python and C++

#include <pybind11/embed.h>

#include <array>
#include <string>

namespace py = pybind11;
using namespace pybind11::literals;

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

int main() {
    py::scoped_interpreter guard{};

    py::module_ site = py::module_::import("site");

    site.attr("addsitedir")(CUSTOM_SYS_PATH);

    py::module_ nose = py::module_::import("nose");
    py::module_ tests = py::module_::import("tests");
    py::list argv(1);
    nose.attr("runmodule")("name"_a = "tests.test_bc1", "exit"_a = false);
}