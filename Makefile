#
# Copyright (c) 2023-present, Zejun Wang.
# All rights reserved.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.
#

CXX = c++
CXXFLAGS = -pthread -std=c++11 -O3 -funroll-loops -march=native
INCLUDE_DIR = include

.PHONY: all
all: fastMatch singleExample
fastMatch: fastMatch.cpp
		$(CXX) $(CXXFLAGS) fastMatch.cpp -I $(INCLUDE_DIR) -o fastMatch
singleExample: singleExample.cpp
		$(CXX) $(CXXFLAGS) singleExample.cpp -I $(INCLUDE_DIR) -o singleExample

clean:
		rm -rf fastMatch singleExample

