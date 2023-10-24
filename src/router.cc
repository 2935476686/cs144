#include "router.hh"

#include <iostream>
#include <limits>

using namespace std;

// route_prefix: The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
// prefix_length: For this route to be applicable, how many high-order (most-significant) bits of
//    the route_prefix will need to match the corresponding bits of the datagram's destination address?
// next_hop: The IP address of the next hop. Will be empty if the network is directly attached to the router (in
//    which case, the next hop address should be the datagram's final destination).
// interface_num: The index of the interface to send the datagram out on.
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  route_table.emplace_back(route_data{route_prefix,prefix_length,next_hop,interface_num});     

}

void Router::route() 
{
  for(auto& i : interfaces_)
  {
    std::optional<InternetDatagram> ip_gram;
    while (true)
    {
      ip_gram = i.maybe_receive();
      if(ip_gram == nullopt) break;
      if(ip_gram.value().header.ttl > 0) ip_gram.value().header.ttl -= 1;
      if(ip_gram.value().header.ttl == 0) continue;
      ip_gram.value().header.compute_checksum();

      if(route_table.size() == 0) continue;

      bool has_route = false;
      route_data route = {};

      bool has_default_route = false;
      route_data default_route = {};

      for(auto& destention:route_table)
      {
        uint8_t len = 32 - destention.mask;
        if(len == 32)
        {
          has_default_route = true;
          default_route = destention;
          continue;
        }

        if(destention.route_prefix >> len == ip_gram->header.dst >> len)
        {
          if(destention.mask > route.mask)
          {
            route = destention;
            has_route = true;
          }
        }
      }

      if(has_route == false)
      {
        if(has_default_route == true)
        {
          route = default_route;
          has_route = true;
        }
        else continue;
      }
      interfaces_[route.interface_num].send_datagram(ip_gram.value(),route.next_hop.value_or(Address::from_ipv4_numeric(ip_gram.value().header.dst)));
    }
  }
}
