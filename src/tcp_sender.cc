#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>

using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{
  cur_rto_ms = initial_RTO_ms;
  rec_msg.window_size = 1;
  rec_msg.ackno = isn_;
}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  // Your code here.
  return outstanding_bytes;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return retrans_nums;
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  // Your code here.
  if(message.size() == 0) return nullopt;

  TCPSenderMessage msg(message.front());
  message.pop_front();

  turnTimer = true;

  return msg;
}

void TCPSender::push( Reader& outbound_stream )
{
  while(outstanding_bytes < rec_msg.window_size)
  {
    TCPSenderMessage send_msg;
    if(!set_syn)
    {
      send_msg.seqno = isn_;
      send_msg.SYN = true;
      set_syn = true;
    }
    else send_msg.seqno = Wrap32::wrap(abs_seq,isn_);
    size_t len = min(min(TCPConfig::MAX_PAYLOAD_SIZE,static_cast<size_t>(rec_msg.window_size-outstanding_bytes)),outbound_stream.bytes_buffered());

    read(outbound_stream,len,send_msg.payload);

    if(outbound_stream.is_finished() == true && 
    send_msg.sequence_length() + outstanding_bytes < rec_msg.window_size)
    {
      if(!set_fin)
      {
        send_msg.FIN = true;
        set_fin = true;
      }
    }

    if(send_msg.sequence_length() == 0) break;
    else
    {
      outstanding_seg.push_back(send_msg);
      message.push_back(send_msg);
      outstanding_bytes += send_msg.sequence_length();
      abs_seq += send_msg.sequence_length();
    }
  }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  TCPSenderMessage msg;
  msg.seqno = Wrap32::wrap(abs_seq,isn_);
  return msg;
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  rec_msg = msg;
  window_size = msg.window_size;
  if(rec_msg.window_size == 0) rec_msg.window_size = 1;

  if(msg.ackno.has_value()) 
  {
    if(msg.ackno.value().unwrap(isn_,abs_seq) > abs_seq) return;
    while(outstanding_bytes != 0&&
    outstanding_seg.front().seqno.unwrap(isn_,abs_seq)
     + outstanding_seg.front().sequence_length() <= msg.ackno.value().unwrap(isn_,abs_seq))
    {
      outstanding_bytes -= outstanding_seg.front().sequence_length();
      outstanding_seg.pop_front();
      if(outstanding_bytes == 0)
        turnTimer = false;
      else
        turnTimer = true;
      
      retrans_nums = 0;
      cur_rto_ms = initial_RTO_ms_;
    }
  }
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  if(turnTimer) cur_rto_ms -= ms_since_last_tick;

  if(cur_rto_ms <= 0)
  {
    message.push_front(outstanding_seg.front());

    retrans_nums += 1;

    if(window_size > 0)
      cur_rto_ms = pow(2,retrans_nums) * initial_RTO_ms_;
    else
      cur_rto_ms = initial_RTO_ms_;
  }
  
}
