/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/packet.h>
#include <ns3/average.h>

using namespace ns3;


class Observador {

public:
  Observador ();
  void PaqueteParaEnviar (Ptr<const Packet> paquete);
  void PaqueteEnBackoff (Ptr<const Packet> paquete);
  void PaqueteRecibidoParaEntregar (Ptr<const Packet> paquete);
  double GetMediaNumIntentos ();
 
private:
  Average<uint32_t> m_acum_numeroIntentos;
  uint32_t  m_numeroIntentos;
};
