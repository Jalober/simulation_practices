/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/callback.h>
#include <ns3/packet.h>
#include "Enlace.h"
#include "CabEnlace.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Enlace");

Enlace::Enlace(                  Ptr<NetDevice> disp,
                                 Time           espera,
                                 uint32_t       tamPqt, 
                                 uint8_t        tamVentana) : m_ventana(tamVentana, (uint32_t) tamVentana * 2)
{
  NS_LOG_FUNCTION (disp << espera << tamPqt << tamVentana);

  // Inicializamos las variables privadas
  m_disp          = disp;
  m_esperaACK     = espera;
  m_tamPqt        = tamPqt;
  m_rx            = 0;
  m_tamVentana    = tamVentana;
  m_primerEnvio   = true;
}


void
Enlace::PaqueteRecibido(  Ptr<NetDevice>        receptor,
                          Ptr<const Packet>     recibido,
                          uint16_t              protocolo,
                          const Address &       desde,
                          const Address &       hacia,
                          NetDevice::PacketType tipoPaquete)
{

  //Obtenemos el paquete y almacenamos los valores de la cabecera
  Ptr<Packet> copia = recibido->Copy ();
  CabEnlace header;
  copia->RemoveHeader (header);

  uint8_t tipo = header.GetTipo();
  uint8_t numSecuencia = header.GetSecuencia();

  if (tipo == ACK) {
  
    NS_LOG_INFO ("NODO " << m_node->GetId() << ": Recibido ACK con numSeq " << (unsigned int) numSecuencia);

    //Si numero de secuencia en ventana
    if (m_ventana.EnVentana((uint32_t) numSecuencia)) {
      //Para temporizador
      Simulator::Cancel(m_temporizador);
      NS_LOG_FUNCTION("Se cancela temporizador");
      //Actualiza ventanaTx
      m_ventana.Asentida((uint32_t) numSecuencia);
      //Envia 
      EnviaPaquete();
    }

  } else if (tipo == PAQUETE) {

    NS_LOG_INFO ("NODO " << m_node->GetId() << ": Recibido paquete con numSeq "  << (unsigned int) numSecuencia);
  
    // Si el número de secuencia es correcto
    if (numSecuencia == m_rx) {
      // Si es correcto, incrementamos el numero de secuencia
      m_rx = ++m_rx % (2 * m_tamVentana);      
    }
    // Transmito en cualquier caso un ACK
    EnviaACK();

  } else {
    NS_LOG_ERROR ("Tipo de paquete desconocido");
  }
}



void
Enlace::VenceTemporizador()
{
  NS_LOG_DEBUG("NODO " << m_node->GetId() << ": VENCE TEMPORIZADOR");
  //Reenviamos todos los paquetes de la ventana
  m_ventana.Vacia();
  EnviaPaquete ();
}


void
Enlace::EnviaPaquete()
{

  // Paquete a enviar 
  Ptr<Packet> paquete;
  // Num secuencia
  uint8_t  m_tx;  
  uint32_t num_transmisiones = 0;

  //Se transmiten los paquetes desde m_tx hasta el final de la ventana
  while (m_ventana.Credito()) {
    num_transmisiones++;
    
    // Envío el paquete  
    m_tx = (uint8_t) m_ventana.Pendiente();
    paquete = Create<Packet> (m_tamPqt);
    CabEnlace header;
    header.SetData (0, m_tx);
    paquete->AddHeader (header);    
    m_node->GetDevice(0)->Send(paquete, m_disp->GetAddress(), 0x0800);

    NS_LOG_INFO ("NODO " << m_node->GetId() << ": Transmitido paquete de " << paquete->GetSize () <<
                 " octetos con numSeq " << (unsigned int) m_tx <<
                 " en " << Simulator::Now());
    
    if (m_esperaACK != 0) {
      if (Simulator::IsExpired(m_temporizador)) {     
        NS_LOG_FUNCTION ("NODO 0: Programamos temporizador");
        m_temporizador = Simulator::Schedule (m_esperaACK, &Enlace::VenceTemporizador, this);
      }
    }
  }
  if (num_transmisiones > 0) {
     NS_LOG_DEBUG ("NODO " << m_node->GetId() << ": LLENA LA VENTANA");
  }
  if (num_transmisiones > 1 && !m_primerEnvio) {
     NS_LOG_DEBUG ( "NODO " << m_node->GetId() << ": SE RECUPERA DE ERROR"); 
  }
  //Se indica que ya se ha pasado al menos una vez por esta funcion
  m_primerEnvio = false;
}


void
Enlace::EnviaACK()
{

  Ptr<Packet> p = Create<Packet> (0);
  CabEnlace header;
  header.SetData (1, m_rx);
  p->AddHeader (header);
  m_node->GetDevice(0)->Send(p, m_disp->GetAddress(), 0x0800);

  NS_LOG_INFO ("NODO " << m_node->GetId() << ": Transmitido ACK de " << p->GetSize () <<
                " octetos con numSeq " << (unsigned int) m_rx <<
                " en " << Simulator::Now());

}


