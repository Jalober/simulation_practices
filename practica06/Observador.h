/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/packet.h>
#include <ns3/average.h>

using namespace ns3;

class Observador {

public:
  Observador ();
  PaqueteEnviado (Ptr<const Packet> paquete);
  PaqueteRecibido (Ptr<const Packet> paquete);
 
private:
  double m_tinicial;
  double m_tfinal;
  Average<double> m_acum_retardo;
};
