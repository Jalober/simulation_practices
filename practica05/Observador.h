/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/packet.h>
#include <ns3/average.h>

using namespace ns3;

class Observador {

public:
  Observador ();
  void PaqueteEnviado (Ptr<const Packet> paquete);
  void PaqueteEnBackoff (Ptr<const Packet> paquete);
  void PaquetePerdido (Ptr<const Packet> paquete);
  void PaqueteParaEnviar (Ptr<const Packet> paquete);
  void PaqueteRecibidoParaEntregar (Ptr<const Packet> paquete);
  double GetMediaNumIntentos ();
  double GetMediaTiempoEco ();
 
private:
  Average<uint32_t> m_acum_numeroIntentos;
  Average<double>   m_acum_tEco;
  uint32_t          m_numeroIntentos;
  double            m_tinicial;
  double            m_tfinal; 
};
