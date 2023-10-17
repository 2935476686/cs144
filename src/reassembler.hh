#pragma once

#include "byte_stream.hh"

#include <string>
#include <algorithm>
class Reassembler
{
public:
  std::deque<char> buffer{};
  std::deque<bool> map{};
  uint64_t begin_index = 0;
  uint64_t assemble_bytes = 0;
  uint64_t last_assemble = 0;
  bool init = false;
  bool eof_flag = false;
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;
};
