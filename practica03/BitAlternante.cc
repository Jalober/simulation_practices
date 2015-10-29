/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include "BitAlternante.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BitAlternante");

BitAlternanteTx::BitAlternanteTx(Ptr<NetDevice> disp,
                                 Time           espera,
                                 uint32_t       tamPqt)
{
  NS_LOG_FUNCTION (disp << espera << tamPqt);

  // Inicializamos las variables privadas
  m_disp      = disp;
  m_esperaACK = espera;
  m_tamPqt    = tamPqt;
  m_tx        = 0;
  m_totalPqt  = 0;   
}



void
BitAlternanteTx::ACKRecibido(Ptr<NetDevice>        receptor,
                             Ptr<const Packet>     recibido,
                             uint16_t              protocolo,
                             const Address &       desde,
                             const Address &       hacia,
                             NetDevice::PacketType tipoPaquete)
{
  NS_LOG_FUNCTION (receptor << recibido->GetSize () <<
                   std::hex << protocolo <<
                   desde << hacia << tipoPaquete);
  uint8_t contenido;

  // Copiamos el primer octeto del campo de datos del paquete recibido
  // (que es el que contiene el número de secuencia)
  recibido->CopyData(&contenido, 1);
  NS_LOG_DEBUG ("    Recibido ACK en nodo " << m_node->GetId() << " con "
                << (unsigned int) contenido);

  // Comprobamos si el número de secuencia del ACK se corresponde con
  // el de secuencia del siguiente paquete a transmitir
  if (contenido == 1 - m_tx) 
    {
      // Si es correcto desactivo el temporizador
      Simulator::Cancel(m_temporizador);
      // Cambiamos el número de secuencia
      m_tx = 1 - m_tx;
      // Incrementamos el total de paquetes
      m_totalPqt++;
      // Se transmite un nuevo paquete
      m_paquete = Create<Packet> (&m_tx, m_tamPqt + 1);
      EnviaPaquete();   
    }
}


void
BitAlternanteTx::VenceTemporizador()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_DEBUG("¡¡¡TIMEOUT!!! Reenviando");
  // Reenviamos el último paquete transmitido
  m_paquete = Create<Packet> (&m_tx, m_tamPqt + 1);
  EnviaPaquete ();
}


void
BitAlternanteTx::EnviaPaquete()
{
  NS_LOG_FUNCTION_NOARGS ();

  // Envío el paquete
  m_node->GetDevice(0)->Send(m_paquete, m_disp->GetAddress(), 0x0800);

  NS_LOG_DEBUG ("Transmitido paquete de " << m_paquete->GetSize () <<
               " octetos en nodo " << m_node->GetId() <<
               " con " << (unsigned int) m_tx <<
               " en " << Simulator::Now());

  // Programo el temporizador
  if (m_esperaACK != 0)
    m_temporizador = Simulator::Schedule (m_esperaACK, &BitAlternanteTx::VenceTemporizador, this);
}


uint32_t
BitAlternanteTx::TotalDatos()
{
  // Devuelvo el total de paquetes enviados
  return m_totalPqt;
}





BitAlternanteRx::BitAlternanteRx(Ptr<NetDevice> disp)
{
  NS_LOG_FUNCTION (disp);

  m_disp = disp;
  m_rx   = 0;
}


void
BitAlternanteRx::PaqueteRecibido(Ptr<NetDevice>        receptor,
                                 Ptr<const Packet>     recibido,
                                 uint16_t              protocolo,
                                 const Address &       desde,
                                 const Address &       hacia,
                                 NetDevice::PacketType tipoPaquete)
{
  NS_LOG_FUNCTION (receptor << recibido->GetSize () <<
                   std::hex << protocolo <<
                   desde << hacia << tipoPaquete);
  uint8_t contenido;

  // Obtengo el valor del número de secuecia
  recibido->CopyData(&contenido, 1);
  NS_LOG_DEBUG ("    Recibido paquete en nodo " << m_node->GetId() << " con "
                << (unsigned int) contenido);
  // Si el número de secuencia es correcto
  if (contenido == m_rx)
    // Si es correcto, cambio el bit
    m_rx = 1 - m_rx;
  // Transmito en cualquier caso un ACK
  EnviaACK();
}


void
BitAlternanteRx::EnviaACK()
{
  NS_LOG_FUNCTION_NOARGS ();
  Ptr<Packet> p = Create<Packet> (&m_rx, 1);

  NS_LOG_DEBUG ("  Transmitido ACK de " << p->GetSize () <<
                " octetos en nodo " << m_node->GetId() <<
                " con " << (unsigned int) m_rx <<
                " en " << Simulator::Now());
  m_node->GetDevice(0)->Send(p, m_disp->GetAddress(), 0x0800);
}
