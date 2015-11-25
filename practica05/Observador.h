/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/packet.h>
#include <ns3/data-rate.h>

using namespace ns3;


class Observador {

public:
  Observador (uint32_t nodoId);
  void PaqueteParaEnviar (Ptr<const Packet> paquete);
  void PaqueteEnBackoff (Ptr<const Packet> paquete);
  void PaqueteRecibidoParaEntregar (Ptr<const Packet> paquete);
  double GetMediaNumIntentos ();
 
private:
  uint32_t m_nodoId;
  uint32_t m_paquetesParaEnviar;
  uint32_t m_paquetesEnBackoff;

};