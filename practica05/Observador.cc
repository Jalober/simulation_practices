/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/ppp-header.h>
#include "Observador.h"

NS_LOG_COMPONENT_DEFINE ("Observador");

//Constructor de observador
Observador::Observador ()
{
  m_numeroIntentos = 0;
}
void
Observador::PaqueteParaEnviar (Ptr<const Packet> paquete) {
  m_numeroIntentos++;
  m_acum_numeroIntentos.Update(m_numeroIntentos);
  m_numeroIntentos = 0;
}

void
Observador::PaqueteEnBackoff (Ptr<const Packet> paquete) {
  m_numeroIntentos++;
}

void
Observador::PaqueteRecibidoParaEntregar (Ptr<const Packet> paquete) {

}


double
Observador::GetMediaNumIntentos () {
  return m_acum_numeroIntentos.Mean();  
}


