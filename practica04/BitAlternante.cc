/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include "BitAlternante.h"
#include "CabEnlace.h"

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

  //Obtenemos el paquete y almacenamos los valores de la cabecera
  Ptr<Packet> copia = recibido->Copy ();
  CabEnlace header;
  copia->RemoveHeader (header);

  uint8_t tipo = header.GetTipo();
  uint8_t numSecuencia = header.GetSecuencia();
  
  NS_LOG_DEBUG ("    Recibido ACK en nodo " << m_node->GetId() << " de tipo  "
                << (unsigned int) tipo << " y numSeq "<< (unsigned int) numSecuencia);

  //Si numero de secuencia en ventana
  if (m_ventana.EnVentana((uint32_t) numSecuencia)) {
    //Para temporizador
    Simulator::Cancel(m_temporizador);
    //Actualiza ventanaTx
    m_ventana.Asentida((uint32_t) numSecuencia);
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
  Ptr<Packet> paquete;
  // Num secuencia
  uint8_t m_tx;  

  //Se transmiten los paquetes desde m_tx hasta el final de la ventana
  while (m_ventana.Credito()) {
    // Envío el paquete  
    m_tx = (uint8_t) m_ventana.Pendiente();
    paquete = Create<Packet> (m_tamPqt);
    CabEnlace header;
    header.SetData (0, m_tx);
    paquete->AddHeader (header);    
    m_node->GetDevice(0)->Send(paquete, m_disp->GetAddress(), 0x0800);

    NS_LOG_DEBUG ("Transmitido paquete de " << paquete->GetSize () <<
                 " octetos en nodo " << m_node->GetId() <<
                 " con numSeq " << (unsigned int) m_tx <<
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

  // Obtengo el valor del número de secuecia
  Ptr<Packet> copia = recibido->Copy ();
  CabEnlace header;
  copia->RemoveHeader (header);
  uint8_t tipo = header.GetTipo ();
  uint8_t numSecuencia = header.GetSecuencia();
  
  NS_LOG_DEBUG ("    Recibido paquete en nodo " << m_node->GetId() << " de tipo "
              << (unsigned int) tipo << " con numSeq"  << (unsigned int) numSecuencia);
  
  // Si el número de secuencia es correcto
  if (numSecuencia == m_rx) {
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
  
  Ptr<Packet> p = Create<Packet> (0);
  CabEnlace header;
  header.SetData (1, m_rx);
  p->AddHeader (header);

  NS_LOG_DEBUG ("  Transmitido ACK de " << p->GetSize () <<
                " octetos en nodo " << m_node->GetId() <<
                " con numSeq" << (unsigned int) m_rx <<
                " en " << Simulator::Now());
  m_node->GetDevice(0)->Send(p, m_disp->GetAddress(), 0x0800);
}


