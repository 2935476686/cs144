#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  if(message.SYN)
  {
    isn = Wrap32(message.seqno);
    message.seqno = message.seqno + 1;
    is_syn = true;
  }

  if(message.FIN)
  {
    is_last_substring = true;
    fin = message.seqno + message.payload.size();
  }
  else
    is_last_substring = false;

  if(is_syn)
    reassembler.insert(message.seqno.unwrap(isn,inbound_stream.bytes_pushed())-1
    ,message.payload,is_last_substring,inbound_stream);
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  TCPReceiverMessage tcpreceiveMessage;
  if(is_syn)
  {
    Wrap32 ack = Wrap32::wrap(inbound_stream.bytes_pushed() + 1,isn); 
    tcpreceiveMessage.ackno = ack == fin ? ack+1:ack;
  }
    
  tcpreceiveMessage.window_size = inbound_stream.available_capacity() > UINT16_MAX?
  UINT16_MAX:inbound_stream.available_capacity();
  return tcpreceiveMessage;
}
