#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  if(IP2MAC.find(next_hop.ipv4_numeric()) != IP2MAC.end())
  {
    EthernetFrame frame;
    frame.header.type = EthernetHeader::TYPE_IPv4;
    frame.header.src = ethernet_address_;
    frame.header.dst = IP2MAC[next_hop.ipv4_numeric()].first;
    frame.payload = serialize(dgram);
    Ethernet_Frame.push_back(frame);
  }
  else
  {
    if(ARP_time.find(next_hop.ipv4_numeric()) == ARP_time.end())
    {
      ARPMessage ARP_grame;
      ARP_grame.opcode = ARPMessage::OPCODE_REQUEST;
      ARP_grame.sender_ethernet_address = ethernet_address_;
      ARP_grame.sender_ip_address = ip_address_.ipv4_numeric();
      ARP_grame.target_ip_address = next_hop.ipv4_numeric();
      EthernetFrame frame;
      frame.header.type = EthernetHeader::TYPE_ARP;
      frame.header.src = ethernet_address_;
      frame.header.dst = ETHERNET_BROADCAST;
      frame.payload = serialize(ARP_grame);

      IP_wait_mac[next_hop.ipv4_numeric()].push_back(dgram);
      ARP_time.emplace(next_hop.ipv4_numeric(),0);
      Ethernet_Frame.push_back(frame);
    }
  }
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if(frame.header.dst != ethernet_address_ && frame.header.dst != ETHERNET_BROADCAST) return nullopt;

  if(frame.header.type == EthernetHeader::TYPE_IPv4)
  {
    InternetDatagram IP_gram;
    if(parse(IP_gram,frame.payload)) return IP_gram;
  }
  else if(frame.header.type == EthernetHeader::TYPE_ARP)
  {
    ARPMessage apr_gram;
    if(parse(apr_gram,frame.payload))
    {
      IP2MAC[apr_gram.sender_ip_address] = {apr_gram.sender_ethernet_address,0};
      if(apr_gram.opcode == ARPMessage::OPCODE_REQUEST)
      {
        if(apr_gram.target_ip_address == ip_address_.ipv4_numeric())
        {
          ARPMessage reply;
          reply.opcode = ARPMessage::OPCODE_REPLY;
          reply.sender_ip_address = ip_address_.ipv4_numeric();
          reply.sender_ethernet_address = ethernet_address_;
          reply.target_ethernet_address = apr_gram.sender_ethernet_address;
          reply.target_ip_address = apr_gram.sender_ip_address;

          EthernetFrame Eth_frame;
          Eth_frame.header.type = EthernetHeader::TYPE_ARP;
          Eth_frame.header.src = reply.sender_ethernet_address;
          Eth_frame.header.dst = reply.target_ethernet_address;
          Eth_frame.payload = serialize(reply);
          Ethernet_Frame.push_back(Eth_frame);
        }
      }
      else if(apr_gram.opcode == ARPMessage::OPCODE_REPLY)
      {
        auto& inte_dgram = IP_wait_mac[apr_gram.sender_ip_address];
        for(auto& i:inte_dgram)
        {
          send_datagram(i,Address::from_ipv4_numeric(apr_gram.sender_ip_address));
        }
        IP_wait_mac.erase(apr_gram.sender_ip_address);
      }
    }
  }
  return nullopt;
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  for(auto i = IP2MAC.begin();i!=IP2MAC.end();)
  {
    i->second.second += ms_since_last_tick;
    if(i->second.second >= ARP_Cache_TTL)
    {
      i = IP2MAC.erase(i);
    }
    else ++i;
  }

  for(auto it = ARP_time.begin();it!=ARP_time.end();)
  {
    it->second += ms_since_last_tick;
    if(it->second >= ARP_Request_TTL)
    {
      it = ARP_time.erase(it);
    }
    else ++it;
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if(Ethernet_Frame.empty()) return nullopt;
  auto frame = Ethernet_Frame.front();
  Ethernet_Frame.pop_front();
  return frame;
}
