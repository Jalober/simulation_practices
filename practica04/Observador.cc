/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/ppp-header.h>
#include "Observador.h"
#include "CabEnlace.h"

#define PAQUETE 0
#define ACK 1

NS_LOG_COMPONENT_DEFINE ("Observador");


Observador::Observador (DataRate vtx, Time tOcupacion, uint32_t tamVentana, uint32_t tamPqt)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_paquetes     = 0;
  m_erroneos     = 0;
  m_bits_utiles  = 0;
  m_vtx          = vtx;
  m_tOcupacion   = tOcupacion; 
  m_tamVentana   = tamVentana;
  m_numSeq       = 0;
  m_tamPqt       = tamPqt;
}

void
Observador::PaqueteAsentido (Ptr<const Packet> paquete)
{
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
    uint32_t aux_num_seq = m_numSeq;
    if (numSecuencia == ++aux_num_seq % (2 * m_tamVentana)) {
        m_bits_utiles += 8 * m_tamPqt;
        m_numSeq++;
        NS_LOG_INFO ("m_bits_utiles: " << m_bits_utiles);
    }
  }
}

void
Observador::PaqueteErroneo (Ptr<const Packet> paquete)
{
  Ptr<Packet> copia = paquete->Copy();
 
  //uint32_t tamPaquete;
  
  m_erroneos++; 

  PppHeader pppHeader;
  CabEnlace header;
  copia->RemoveHeader (pppHeader);
  copia->RemoveHeader (header);
  
  uint8_t tipo = header.GetTipo();
  uint8_t numSecuencia = header.GetSecuencia(); 
  if (tipo == ACK) {
    uint32_t aux_num_seq = m_numSeq;
    if (numSecuencia == ++aux_num_seq % (2 * m_tamVentana)) {
        m_bits_utiles += 8 * m_tamPqt;
        m_numSeq++;
        NS_LOG_INFO ("m_bits_utiles: " << m_bits_utiles);
    }
  }

  //uint8_t tipo = header.GetTipo();

  /*
  if (tipo == PAQUETE) {
     tamPaquete = copia->GetSize();
     if (m_bits_utiles >= (8 * m_tamVentana * tamPaquete)) {
        m_bits_utiles -= (8 * m_tamVentana * tamPaquete);
        NS_LOG_INFO ("Restados " << 8 * m_tamVentana * tamPaquete << "bits. Ahora hay " << m_bits_utiles << "bits");
     } else {
         m_bits_utiles = 0;
     }
  }  
  */
  
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

DataRate Observador::GETCef () 
{
  DataRate result = DataRate((m_bits_utiles) / m_tOcupacion.GetSeconds());
  NS_LOG_DEBUG ("Cef: " << result);
  return result;
}

double Observador::GETRend () {
  double result = (((double)m_bits_utiles) / (double) m_vtx.GetBitRate()) / (double)m_tOcupacion.GetSeconds();
  NS_LOG_DEBUG ("Rend: " << result * 100);
  return result;
}
