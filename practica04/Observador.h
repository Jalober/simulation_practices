/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/packet.h>
#include <ns3/data-rate.h>

using namespace ns3;


class Observador
{
public:
  Observador  ();
  void     PaqueteAsentido (Ptr<const Packet> paquete);
  void     PaqueteErroneo  (Ptr<const Packet> paquete);
  uint32_t TotalPaquetes   ();
  uint32_t TotalErroneos   ();
  DataRate GETCef ();
  double GETRend ();

private:
  uint64_t m_paquetes;
  uint64_t m_erroneos;
};
