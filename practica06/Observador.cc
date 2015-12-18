/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include "Observador.h"

NS_LOG_COMPONENT_DEFINE ("Observador");

//Constructor de observador
Observador::Observador ()
{
  
}

void
Observador::EnvioPaquete (Ptr<const Packet> paquete) {
  NS_LOG_FUNCTION_NOARGS();
  //Obtenemos el uid de paquete
  m_tEnvio[paquete->GetUid ()] = Simulator::Now ();
}

void
Observador::PaqueteRecibido (Ptr<const Packet> paquete, const Address &dir) {
  NS_LOG_FUNCTION_NOARGS();
  std::map<uint64_t, Time>::iterator it;
  it = m_tEnvio.find (paquete->GetUid ());
  if (it != m_tEnvio.end()) {
    double retardo = Simulator::Now().GetDouble() - it->second.GetDouble();
    m_acum_retardo.Update(retardo);
    m_tEnvio.erase (it);
    NS_LOG_INFO ("Retardo de paquete con uid " << paquete->GetUid() << ": " << retardo);
  } else {
    NS_LOG_ERROR ("Paquete con uid " << paquete->GetUid() << " no encontrado en map!");
  } 
}

Time
Observador::GetRetardoMedio () {
  return Time(m_acum_retardo.Mean());
}

unsigned int
Observador::MapSize () {
  return m_tEnvio.size ();
}
