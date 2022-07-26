//
// Created by nicof on 26/07/22.
//

#include "tokenizer.h"

#include <regex>

using namespace bencode;

void Tokenizer::tokenize(const std::string &encoded,
                         std::vector<std::string> &tokens) {
  std::regex regexp("([idel])|(\\d+):|(-?\\d+)");
  std::sregex_token_iterator iter(encoded.begin(), encoded.end(), regexp,
                                  {-1, 0});
  std::sregex_token_iterator end;
  while (iter != end) {
    if (iter->length() > 0) {
      tokens.push_back(*iter);
    }
    ++iter;
  }
}