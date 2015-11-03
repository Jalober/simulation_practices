/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include "Observador.h"

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
  m_paquetes ++;
}


uint32_t
Observador::TotalPaquetes ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_paquetes;
}
