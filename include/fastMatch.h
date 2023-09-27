/**
 * Copyright (c) 2023-present, Zejun Wang.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef FAST_MATCH_H
#define FAST_MATCH_H

#include <algorithm>
#include <cmath>
#include <cstring>
#include <numeric>
#include <iostream>
#include <functional>
#include <fstream>
#include <thread>

#ifndef USE_PREFIX_TRIE
#include "cedar.h"
#else
#include "cedarpp.h"
#endif

#define maxPrefixMatches 64
typedef cedar::da<int> trie;

using namespace std;

#if __cplusplus >= 201703L
inline int match(string_view text, string_view pattern) {
  if (text.empty() || pattern.empty())
    return -1;
#ifdef USE_STRING_FIND
  auto pos = text.find(pattern);
  if (pos == string::npos)
    return -1;
  return pos;
#elif USE_DEFAULT_SEARCHER
  auto it = search(text.begin(), text.end(), default_searcher(pattern.begin(), pattern.end()));
  if (it == text.end())
    return -1;
  return it - text.begin();
#elif USE_BOYER_MOORE_SEARCHER
  auto it = search(text.begin(), text.end(), boyer_moore_searcher(pattern.begin(), pattern.end()));
  if (it == text.end())
    return -1;
  return it - text.begin();
#elif USE_BOYER_MOORE_HORSPOOL_SEARCHER
  auto it = search(text.begin(), text.end(), boyer_moore_horspool_searcher(pattern.begin(), pattern.end()));
  if (it == text.end())
    return -1;
  return it - text.begin();
#else
  auto address = strstr(text.data(), pattern.data());
  if (address == NULL)
    return -1;
  return address - text.data();
#endif
}
#else
inline int match(const string& text, const string& pattern) {
  if (text.empty() || pattern.empty())
    return -1;
#ifdef USE_STRING_FIND
  auto pos = text.find(pattern);
  if (pos == string::npos)
    return -1;
  return pos;
#else
  auto address = strstr(text.data(), pattern.data());
  if (address == NULL)
    return -1;
  return address - text.data();
#endif
}
#endif

inline void RunMultiThread(function<void(size_t, size_t)> func, size_t n, int num_threads) {
  vector<thread> threads;
  threads.reserve(static_cast<size_t>(num_threads));
  size_t start = 0, end = 0, step = ceil(n / float(num_threads));
  for (int i = 0; i < num_threads; ++i) {
    end = start + step;
    if (end > n)
      end = n;
    threads.emplace_back(thread(func, start, end));
    start = end;
  }
  for (auto& t : threads)
    t.join();
}

inline void SingleMatch(const vector<string>& text, const string& pattern, int num_threads = 0) {
  if (text.empty() || pattern.empty())
    return;
  if (num_threads <= 0)
    num_threads = thread::hardware_concurrency();
  // single thread processing
  size_t n = text.size();
  if (num_threads == 1) {
    for (size_t i = 0; i < n; ++i)
      if (match(text[i], pattern) >= 0)
        cout << text[i] << '\n';
    return;
  }
  // multithread processing
  bool v[n];
#ifdef USE_OMP
#pragma omp parallel for num_threads(num_threads)
  for (size_t i = 0; i < n; ++i)
    v[i] = match(text[i], pattern) >= 0;
#else
  auto func = [&](size_t start, size_t end) {
    for (size_t i = start; i < end; ++i)
      v[i] = match(text[i], pattern) >= 0;
  };
  RunMultiThread(func, n, num_threads);
#endif
  for (size_t i = 0; i < n; ++i)
    if (v[i])
      cout << text[i] << '\n';
}

class FastMatch : public trie {
  public:
  FastMatch() {}
  FastMatch(const string& filename, size_t capacity = 0) {
    ifstream in(filename);
    if (!in.is_open()) {
      cerr << "Failed to load key file!\n";
      exit(EXIT_FAILURE);
    }
    string key;
    if (capacity)
      _key.reserve(capacity);
    while (getline(in, key))
      if (key.size())
        _key.emplace_back(key);
    _size = _key.size();
    build(_size, _key, 0);
  }
  FastMatch(const vector<string>& key) : _key(key) {
    _size = _key.size();
    build(_size, _key, 0);
  }
  
  size_t size() const { return _size; }
  
  int insert(const string& key) {
    int index = exactMatchSearch<int>(key.c_str(), key.size());
    if (index < 0) {
      update(key.c_str(), key.size(), _size);
      ++_size;
      _key.emplace_back(key);
      return _size - 1;
    }
    return index;
  }
  
  int remove(const string& key) {
    return erase(key.c_str(), key.size());
  }
  
  string getKey(int id) const {
    if (id >= 0 && id < _size)
      return _key[id];
    return "";
  }
  
  int getValue(const string& key) const {
    return exactMatchSearch<int>(key.c_str(), key.size());
  }
  
  int hit(const string& text) const {
    if (text.empty())
      return -1;
    trie::result_pair_type result_pair;
    const char* str = text.c_str();
    size_t num = 0, cur = 0, len = text.size();
    while (cur < len) {
      num = commonPrefixSearch(str + cur, len - cur, &result_pair, maxPrefixMatches);
      if (num)
        return result_pair.value;
      ++cur;
      while (cur < len && (str[cur] & 0xC0) == 0x80)
        ++cur;
    }
    return -1;
  }
  
  vector<pair<string, int>> parse(const string& text) const {
    vector<pair<string, int>> res;
    if (text.empty())
      return res;
    trie::result_pair_type result_pair[maxPrefixMatches];
    const char* str = text.c_str();
    size_t num = 0, cur = 0, len = text.size();
    while (cur < len) {
      num = commonPrefixSearch(str + cur, result_pair, maxPrefixMatches, len - cur);
      for (size_t i = 0; i < num; ++i)
        res.emplace_back(_key[result_pair[i].value], cur);
      ++cur;
      while (cur < len && (str[cur] & 0xC0) == 0x80)
        ++cur;
    }
    return res;
  }
  
  vector<pair<string, int>> parseBind(const string& text) const {
    vector<pair<string, int>> res;
    if (text.empty())
      return res;
    trie::result_pair_type result_pair[maxPrefixMatches];
    const char* str = text.c_str();
    size_t num = 0, cur = 0, index = 0, len = text.size();
    while (cur < len) {
      num = commonPrefixSearch(str + cur, result_pair, maxPrefixMatches, len - cur);
      for (size_t i = 0; i < num; ++i)
        res.emplace_back(_key[result_pair[i].value], index);
      ++index;
      ++cur;
      while (cur < len && (str[cur] & 0xC0) == 0x80)
        ++cur;
    }
    return res;
  }

  string parseSingle(const string& text, int num_patterns = -1) const {
    string res;
    if (text.empty())
      return res;
    trie::result_pair_type result_pair[maxPrefixMatches];
    const char* str = text.c_str();
    int count = 0;
    size_t num = 0, cur = 0, len = text.size();
    while (cur < len) {
      num = commonPrefixSearch(str + cur, result_pair, maxPrefixMatches, len - cur);
      for (int i = num - 1; i >= 0; --i) {
        ++count;
        res.push_back('\t');
        res.append(_key[result_pair[i].value]);
        if (num_patterns >= 0 && count >= num_patterns)
          return res;
      }
      ++cur;
      while (cur < len && (str[cur] & 0xC0) == 0x80)
        ++cur;
    }
    return res;
  }
  
  string parseSingleFast(const string& text, int num_patterns = -1) const {
    string res;
    if (text.empty())
      return res;
    trie::result_pair_type result_pair;
    const char* str = text.c_str();
    int count = 0;
    size_t num = 0, cur = 0, len = text.size();
    while (cur < len) {
      num = commonPrefixSearch(str + cur, len - cur, &result_pair, 1);
      if (num) {
        ++count;
        res.push_back('\t');
        res.append(_key[result_pair.value]);
        if (num_patterns >= 0 && count >= num_patterns)
          return res;
      }
      ++cur;
      while (cur < len && (str[cur] & 0xC0) == 0x80)
        ++cur;
    }
    return res;
  }
  
  void parse(const vector<string>& text, bool fast = false, int num_patterns = -1,
      int num_threads = 0) const {
    if (text.empty())
      return;
    if (num_threads <= 0)
      num_threads = thread::hardware_concurrency();
    // single thread processing
    size_t n = text.size();
    if (num_threads == 1) {
      for (size_t i = 0; i < n; ++i) {
        string str = fast ? move(parseSingleFast(text[i], num_patterns)) :
                            move(parseSingle(text[i], num_patterns));
        if (str.size())
          cout << text[i] << str << '\n';
      }
      return;
    }
    // multithread processing
    vector<string> v(n);
#ifdef USE_OMP
#pragma omp parallel for num_threads(num_threads)
    for (size_t i = 0; i < n; ++i)
      v[i] = fast ? move(parseSingleFast(text[i], num_patterns)) :
                    move(parseSingle(text[i], num_patterns));
#else
    auto func = [&](size_t start, size_t end) {
      for (size_t i = start; i < end; ++i)
        v[i] = fast ? move(parseSingleFast(text[i], num_patterns)) :
                      move(parseSingle(text[i], num_patterns));
    };
    RunMultiThread(func, n, num_threads);
#endif
    for (size_t i = 0; i < n; ++i)
      if (v[i].size())
        cout << text[i] << v[i] << '\n';
  }
  
  void parseHit(const vector<string>& text, int num_threads = 0) const {
    if (text.empty())
      return;
    if (num_threads <= 0)
      num_threads = thread::hardware_concurrency();
    // single thread processing
    size_t n = text.size();
    if (num_threads == 1) {
      int val = -1;
      for (size_t i = 0; i < n; ++i) {
        val = hit(text[i]);
        if (val >= 0)
          cout << text[i] << '\t' << _key[val] << '\n';
      }
      return;
    }
    // multithread processing
    int v[n];
#ifdef USE_OMP
#pragma omp parallel for num_threads(num_threads)
    for (size_t i = 0; i < n; ++i)
      v[i] = hit(text[i]);
#else
    auto func = [&](size_t start, size_t end) {
      for (size_t i = start; i < end; ++i)
        v[i] = hit(text[i]);
    };
    RunMultiThread(func, n, num_threads);
#endif
    for (size_t i = 0; i < n; ++i)
      if (v[i] >= 0)
        cout << text[i] << '\t' << _key[v[i]] << '\n';
  }
  
  vector<string> maxForwardMatch(const string& text) const {
    vector<string> res;
    if (text.empty())
      return res;
    trie::result_pair_type result_pair;
    const char* str = text.c_str();
    size_t num = 0, cur = 0, last = 0, len = text.size();
    res.reserve(len >> 2);
    while (cur < len) {
      num = commonPrefixSearch(str + cur, len - cur, &result_pair, maxPrefixMatches);
      if (num) {
        res.emplace_back(_key[result_pair.value]);
        cur += result_pair.length;
        continue;
      }
      last = cur;
      while (cur < len && isascii(str[cur]) && !isspace(str[cur]))
        ++cur;
      if (last == cur) {
        ++cur;
        while (cur < len && (str[cur] & 0xC0) == 0x80)
          ++cur;
      }
      res.emplace_back(text.substr(last, cur - last));
    }
    return res;
  }
  
  #if __cplusplus >= 201703L
  vector<string_view> maxForwardMatchView(string_view text) const {
    vector<string_view> res;
    if (text.empty())
      return res;
    trie::result_pair_type result_pair;
    const char* str = text.data();
    size_t num = 0, cur = 0, last = 0, len = text.size();
    res.reserve(len >> 2);
    while (cur < len) {
      num = commonPrefixSearch(str + cur, len - cur, &result_pair, maxPrefixMatches);
      if (num) {
        res.emplace_back(_key[result_pair.value]);
        cur += result_pair.length;
        continue;
      }
      last = cur;
      while (cur < len && isascii(str[cur]) && !isspace(str[cur]))
        ++cur;
      if (last == cur) {
        ++cur;
        while (cur < len && (str[cur] & 0xC0) == 0x80)
          ++cur;
      }
      res.emplace_back(text.substr(last, cur - last));
    }
    return res;
  }
  #endif
  
  string maxForwardMatchSingle(const string& text) const {
    string res;
    if (text.empty())
      return res;
    trie::result_pair_type result_pair;
    const char* str = text.c_str();
    size_t num = 0, cur = 0, last = 0, len = text.size();
    res.reserve(len * 4 / 3);
    while (cur < len) {
      num = commonPrefixSearch(str + cur, len - cur, &result_pair, maxPrefixMatches);
      if (num) {
        res.append(_key[result_pair.value]);
        res.push_back(' ');
        cur += result_pair.length;
        continue;
      }
      last = cur;
      while (cur < len && isascii(str[cur]) && !isspace(str[cur]))
        ++cur;
      if (last == cur) {
        ++cur;
        while (cur < len && (str[cur] & 0xC0) == 0x80)
          ++cur;
      }
      res.append(str + last, cur - last);
      res.push_back(' ');
    }
    res.back() = '\n';
    return res;
  }
  
  void maxForwardMatch(const vector<string>& text, int num_threads = 0) const {
    if (text.empty())
      return;
    if (num_threads <= 0)
      num_threads = thread::hardware_concurrency();
    
    // single thread processing
    size_t n = text.size();
    if (num_threads == 1) {
      for (size_t i = 0; i < n; ++i)
        cout << maxForwardMatchSingle(text[i]);
      return;
    }
    // multithread processing
    vector<string> v(n);
#ifdef USE_OMP
#pragma omp parallel for num_threads(num_threads)
    for (size_t i = 0; i < n; ++i)
      v[i] = move(maxForwardMatchSingle(text[i]));
#else
    auto func = [&](size_t start, size_t end) {
      for (size_t i = start; i < end; ++i)
        v[i] = move(maxForwardMatchSingle(text[i]));
    };
    RunMultiThread(func, n, num_threads);
#endif
    for (size_t i = 0; i < n; ++i)
      cout << v[i];
  }

  private:
  size_t _size = 0;
  vector<string> _key;
};

#endif

