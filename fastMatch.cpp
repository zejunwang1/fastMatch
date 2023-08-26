/**
 * Copyright (c) 2023-present, Zejun Wang.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <memory>
#include <args.h>
#include <fastMatch.h>

int main(int argc, char** argv) {
  vector<string> args(argv, argv + argc);
  Args a(args);
  // load text strings
  vector<string> text;
  if (a.N)
    text.reserve(a.N);
  ifstream textIn(a.input);
  if (!textIn.good()) {
    cerr << "Failed to load text strings!" << endl;
    exit(EXIT_FAILURE);
  }
  string str;
  while (getline(textIn, str))
    text.emplace_back(str);
  // single pattern string
  ifstream ifs(a.pattern);
  if (!ifs.good()) {
    SingleMatch(text, a.pattern, a.num_threads);
    return 0;
  }
  // multi-pattern matching
  shared_ptr<FastMatch> fastMatch = make_shared<FastMatch>(a.pattern, a.M);
  if (a.seg) {
    fastMatch->maxForwardMatch(text, a.num_threads);
  } else if (a.hit) {
    fastMatch->parseHit(text, a.num_threads);
  } else {
    fastMatch->parse(text, a.fast, a.num_patterns, a.num_threads);
  }
  return 0;
}

