/**
 * Copyright (c) 2023-present, Zejun Wang.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef ARGS_H
#define ARGS_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

class Args {
  public:
  std::string input;
  std::string pattern;
  int num_threads = -1;
  int num_patterns = -1;
  bool fast = false;
  bool hit = false;
  bool seg = false;
  size_t N = 0;
  size_t M = 0;

  Args(const std::vector<std::string>& args) {
    for (int i = 1; i < args.size(); i += 2) {
      if (args[i] == "-h" || args[i] == "--help") {
        printHelp();
        exit(EXIT_FAILURE);
      }
      if (args[i][0] != '-' || args[i][1] != '-') {
        std::cerr << "Provided argument without a dash!" << std::endl;
        printHelp();
        exit(EXIT_FAILURE);
      }
      try {
        if (args[i] == "--input") {
          input = std::string(args.at(i + 1));
        } else if (args[i] == "--pattern") {
          pattern = std::string(args.at(i + 1));
        } else if (args[i] == "--num_threads") {
          num_threads = std::stoi(args.at(i + 1));
        } else if (args[i] == "--num_patterns") {
          num_patterns = std::stoi(args.at(i + 1));
        } else if (args[i] == "--fast") {
          fast = true;
          i--;
        } else if (args[i] == "--hit") {
          hit = true;
          i--;
        } else if (args[i] == "--seg") {
          seg = true;
          i--;
        } else if (args[i] == "--N") {
          N = static_cast<size_t>(stoul(args.at(i + 1)));
        } else if (args[i] == "--M") {
          M = static_cast<size_t>(stoul(args.at(i + 1)));
        } else {
          std::cerr << "Unknown argument: " << args[i] << std::endl;
          printHelp();
          exit(EXIT_FAILURE);
        }
      } catch (std::out_of_range) {
        std::cerr << args[i] << " is missing an argument" << std::endl;
        printHelp();
        exit(EXIT_FAILURE);
      }
    }
    if (input.empty() || pattern.empty()) {
      std::cerr << "Empty input or pattern path." << std::endl;
      printHelp();
      exit(EXIT_FAILURE);
    }
  }
  
  void printHelp() {
    std::cerr << "\nLarge-scale Exact String Matching Tool! Usage:\n";
    std::cerr << "  --input         text string file path\n"
              << "  --pattern       pattern string or pattern string file path\n"
              << "  --num_threads   number of threads\n"
              << "  --num_patterns  number of matching patterns returned\n"
              << "  --fast          enable fast matching mode\n"
              << "  --hit           enable hit matching mode\n"
              << "  --seg           enable maximum forward matching word segmentation\n"
              << "  --N             total number of text strings\n"
              << "  --M             total number of pattern strings\n"
              << "  --help -h       show help information\n\n";
  }
  
};

#endif

