//
// Created by nicof on 26/07/22.
//

#ifndef FURRENT_TOKENIZER_H
#define FURRENT_TOKENIZER_H

#include <string>
#include <vector>

namespace bencode{

  class Tokenizer {
     public:
      static void tokenize(const std::string& encoded, std::vector<std::string>& tokens);
     private:
      Tokenizer();
      ~Tokenizer();
  };

}


#endif  // FURRENT_TOKENIZER_H
