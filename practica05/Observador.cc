/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/ppp-header.h>
#include "Observador.h"

NS_LOG_COMPONENT_DEFINE ("Observador");

//Constructor de observador
Observador::Observador (uint32_t nodoId)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_nodoId = nodoId;
  m_paquetesParaEnviar = 0;
  m_paquetesEnBackoff = 0;
}

Observador::PaqueteParaEnviar (Ptr<const Packet> paquete) {
  m_paquetesParaEnviar++;
}

Observador::PaqueteEnBackoff (Ptr<const Packet> paquete) {
  m_paquetesEnBackoff++;
}




