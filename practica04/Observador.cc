/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/ppp-header.h>
#include "Observador.h"
#include "CabEnlace.h"

#define PAQUETE 0
#define ACK 1

NS_LOG_COMPONENT_DEFINE ("Observador");


Observador::Observador ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_paquetes = 0;
  m_erroneos = 0;
}


void
Observador::PaqueteAsentido (Ptr<const Packet> paquete)
{
  NS_LOG_FUNCTION (paquete);
  Ptr<Packet> copia = paquete->Copy();
 
  PppHeader pppHeader; 
  CabEnlace header;
  copia->RemoveHeader (pppHeader);
  copia->RemoveHeader (header);
  
  uint8_t tipo = header.GetTipo();
  uint8_t numSecuencia = header.GetSecuencia();
  
  NS_LOG_FUNCTION ("Tipo de paquete detectado" << (unsigned int) tipo);
  NS_LOG_FUNCTION ("Tamaño paquete" << (unsigned int) copia->GetSize());
  NS_LOG_FUNCTION ("NumSeq" << (unsigned int) numSecuencia);
  if (tipo == ACK) {
    m_paquetes++;
  }
}

void
Observador::PaqueteErroneo (Ptr<const Packet> paquete)
{
  NS_LOG_FUNCTION (paquete);
  m_erroneos++;
}

uint32_t
Observador::TotalPaquetes ()
{
  NS_LOG_FUNCTION ("Total paquetes: " << (unsigned int) m_paquetes);
  return m_paquetes;
}

uint32_t
Observador::TotalErroneos ()
{
  NS_LOG_FUNCTION ("Total Erroneos: " << (unsigned int) m_erroneos);
  return m_erroneos;
}

//TODO
DataRate Observador::GETCef () 
{
  DataRate result(0);
  return result;
}

//TODO
double Observador::GETRend () {
  double result = 0;
  return result;
}
