/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/packet.h>
#include <ns3/average.h>
#include <ns3/nstime.h>
#include <ns3/address.h>

using namespace ns3;

class Observador {

public:
  Observador ();
  void EnvioPaquete (Ptr<const Packet> paquete);
  void PaqueteRecibido (Ptr<const Packet> paquete, const Address &dir);
  Time GetRetardoMedio ();
  unsigned int MapSize ();
private:  
  //Map con el momento de envio de los paquetes
  std::map<uint64_t, Time> m_tEnvio;
  //Acumulador con los retardos
  Average<double> m_acum_retardo;
};
