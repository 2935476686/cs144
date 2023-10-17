#include <stdexcept>

#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  uint64_t len = data.size();
  if(len>capacity_ - buffer.size())
    len = capacity_ - buffer.size();
  writeSize += len;
  for(uint64_t i = 0;i<len;i++)
  {
    buffer.push_back(data[i]);
  }
}

void Writer::close()
{
  end_flag = true;
}

void Writer::set_error()
{
  error_flag = true;
}

bool Writer::is_closed() const
{
  return end_flag;
}

uint64_t Writer::available_capacity() const
{

  return capacity_ - buffer.size();
}

uint64_t Writer::bytes_pushed() const
{
  return writeSize;
}

string_view Reader::peek() const
{
  return {std::string_view(&buffer.front(), 1)};
}

bool Reader::is_finished() const
{
  return end_flag&&buffer.empty();
}

bool Reader::has_error() const
{
  return error_flag;
}

void Reader::pop( uint64_t len )
{
  uint64_t length = len;
  if(length > buffer.size())
    length = buffer.size();
  readSize += length;
  for(uint64_t i = 0;i<length;i++)
    buffer.pop_front();
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return buffer.size();
}

uint64_t Reader::bytes_popped() const
{
  return readSize;
}
