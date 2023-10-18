#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { zero_point + (n&0xffffffff)};
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint32_t offset = this->raw_value_ - wrap(checkpoint,zero_point).raw_value_;
  uint64_t abs_seq = checkpoint + offset;
  if (offset > (0x80000000) && abs_seq >= (0x100000000)) {
      abs_seq -= (0x100000000);
  }

    return abs_seq;
}
