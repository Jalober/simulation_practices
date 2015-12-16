/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include "Observador.h"

NS_LOG_COMPONENT_DEFINE ("Observador");

//Constructor de observador
Observador::Observador ()
{
  m_tinicial = 0;
  m_tfinal   = 0;
}

void
Observador::PaqueteEnviado (Ptr<const Packet> paquete) {
  NS_LOG_FUNCTION_NOARGS(); 
  m_tinicio = Simulator::Now().GetDouble();
}

void
Observador::PaqueteRecibido (Ptr<const Packet> paquete) {
  m_tfinal = Simulator::Now().GetDouble ();
  if (m_tfinal > m_tinicial) {
    m_acum_retardo.Update(m_tfinal - m_tinicial);
  } else {
    if (m_tinicial !=0) {
       NS_LOG_ERROR ("Tiempo final menor que inicial!!");
    }
  }
  NS_LOG_FUNCTION ("Retardo" << m_tfinal - m_tinicial);
}
