/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include "Observador.h"
#include "CabEnlace.h"

#define PAQUETE 0
#define ACK 1

NS_LOG_COMPONENT_DEFINE ("Observador");


Observador::Observador ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_paquetes = 0;
}


void
Observador::PaqueteAsentido (Ptr<const Packet> paquete)
{
  NS_LOG_FUNCTION (paquete);
  CabEnlace header;
  paquete->RemoveHeader (header);
  if (header.GetTipo() == ACK) {
  	m_paquetes ++;
  }
}


uint32_t
Observador::TotalPaquetes ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_paquetes;
}
