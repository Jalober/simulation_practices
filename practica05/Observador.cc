/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/ppp-header.h>
#include "Observador.h"


NS_LOG_COMPONENT_DEFINE ("Observador");

//Constructor de observador
Observador::Observador ()
{
  m_numeroIntentos = 0;
  m_numeroPaquetesCorrectos = 0;
  m_numeroPaquetesPerdidos = 0;
}
void
Observador::PaqueteEnviado (Ptr<const Packet> paquete) {
  m_numeroIntentos++;
  m_numeroPaquetesCorrectos++;
  m_acum_numeroIntentos.Update(m_numeroIntentos);
  m_numeroIntentos = 0;
}

void
Observador::PaqueteEnBackoff (Ptr<const Packet> paquete) {
  m_numeroIntentos++;
}

void
Observador::PaquetePerdido (Ptr<const Packet> paquete) {
  m_numeroIntentos = 0;
  m_numeroPaquetesPerdidos++;  
}

void
Observador::PaqueteParaEnviar (Ptr<const Packet> paquete) {
  m_tinicial = Simulator::Now().GetDouble();
}

void
Observador::PaqueteRecibidoParaEntregar (Ptr<const Packet> paquete) {
  m_tfinal = Simulator::Now().GetDouble ();
  m_acum_tEco.Update(m_tfinal - m_tinicial);
}

double
Observador::GetMediaNumIntentos () {
  return m_acum_numeroIntentos.Mean ();  
}

double
Observador::GetMediaTiempoEco () {
  return m_acum_tEco.Mean ();
}

double
Observador::GetPorcentajePaquetesPerdidos () {
  return (double) 100 * m_numeroPaquetesPerdidos / (m_numeroPaquetesCorrectos + m_numeroPaquetesPerdidos);
}

