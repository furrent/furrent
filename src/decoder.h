//
// Created by nicof on 28/07/22.
//

#ifndef FURRENT_DECODER_H
#define FURRENT_DECODER_H

#include <string>

namespace bencode {
class Decoder {
 public:
  Decoder();
  ~Decoder();
  void decode(std::string &decoded);
};
}


#endif  // FURRENT_DECODER_H
