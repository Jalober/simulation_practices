/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include "BitAlternante.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("BitAlternante");

BitAlternanteTx::BitAlternanteTx(Ptr<NetDevice> disp,
                                 Time           espera,
                                 uint32_t       tamPqt, 
                                 uint8_t        tamVentana) : m_ventana(tamVentana, (uint32_t) tamVentana * 2)
{
  NS_LOG_FUNCTION (disp << espera << tamPqt);

  // Inicializamos las variables privadas
  m_disp          = disp;
  m_esperaACK     = espera;
  m_tamPqt        = tamPqt;  
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

  //Si numero de secuencia en ventana
  if (m_ventana.EnVentana((uint32_t) contenido)) {
    //Para temporizador
    Simulator::Cancel(m_temporizador);
    //Actualiza ventanaTx
    m_ventana.Asentida((uint32_t)contenido);
    //Envia 
    EnviaPaquete();
  }
}



void
BitAlternanteTx::VenceTemporizador()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_DEBUG("¡¡¡TIMEOUT!!! Reenviando");
  // Reenviamos el último paquete transmitido
  m_ventana.Vacia();
  EnviaPaquete ();
}


void
BitAlternanteTx::EnviaPaquete()
{
  NS_LOG_FUNCTION_NOARGS ();

  // Paquete a enviar 
  Ptr<Packet> m_paquete;
  // Num secuencia
  uint8_t m_tx;  

  //Se transmiten los paquetes desde m_tx hasta el final de la ventana
  while (m_ventana.Credito()) {
    // Envío el paquete  
    m_tx = (uint8_t) m_ventana.Pendiente();
    m_paquete = Create<Packet> (&m_tx, m_tamPqt + 1);
    m_node->GetDevice(0)->Send(m_paquete, m_disp->GetAddress(), 0x0800);

    NS_LOG_DEBUG ("Transmitido paquete de " << m_paquete->GetSize () <<
                 " octetos en nodo " << m_node->GetId() <<
                 " con " << (unsigned int) m_tx <<
                 " en " << Simulator::Now());
    if (m_esperaACK != 0) {
      if (Simulator::IsExpired(m_temporizador))      
        m_temporizador = Simulator::Schedule (m_esperaACK, &BitAlternanteTx::VenceTemporizador, this);
    }
  }
  NS_LOG_FUNCTION ("Salimos del bucle");  
}





BitAlternanteRx::BitAlternanteRx(Ptr<NetDevice> disp, uint8_t tamVentana)
{
  NS_LOG_FUNCTION (disp);

  m_disp = disp;
  m_rx   = 0;
  m_tamVentana = tamVentana;
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
  if (contenido == m_rx) {
    // Si es correcto, incrementamos el numero de secuencia
    m_rx = ++m_rx % (2 * m_tamVentana);      
  }
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


