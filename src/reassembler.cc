#include "reassembler.hh"
#include <iostream>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  if(!init)
  {
    map.resize(output.available_capacity(),false);
    buffer.resize(output.available_capacity(),0);
    init = true;
  }

  last_assemble = output.bytes_pushed();
  uint64_t end_index = begin_index + output.available_capacity();
  if(first_index>end_index) return;
  if(is_last_substring) eof_flag = true;

  for(uint64_t i = max(begin_index,first_index);i<data.size()+first_index&&i<end_index;i++)
  {
    if(map[i-last_assemble]) continue;
    map[i-last_assemble] = true;
    buffer[i-last_assemble] = data[i-first_index];
    assemble_bytes += 1;
  }
  string str;

  while(map.front())
  {
    str += buffer.front();
    buffer.pop_front();
    map.pop_front();
    buffer.push_back('\0');
    map.push_back(false);
    assemble_bytes -= 1;
    begin_index += 1;
  }
  output.push(str);

 if(eof_flag&& assemble_bytes == 0) output.close();
}

uint64_t Reassembler::bytes_pending() const
{
  return assemble_bytes;
}
