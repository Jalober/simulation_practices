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
                                 uint8_t        tamVentana)
{
  NS_LOG_FUNCTION (disp << espera << tamPqt);

  // Inicializamos las variables privadas
  m_disp          = disp;
  m_esperaACK     = espera;
  m_tamPqt        = tamPqt;
  m_inicioVentana = 0;
  m_tamVentana    = tamVentana;
  m_tx            = m_inicioVentana;
  m_totalPqt      = 0;
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

  /* Comprobamos si el siguiente paquete a enviar (valor de ACK) es el
     de inicio de ventana. En ese caso enviamos reenviamos todos los paquetes
     de la ventana (ha habido un error) */
  if (contenido == m_inicioVentana) {
    m_tx = m_inicioVentana;
    EnviaPaquete();
  }
  /* Comprobamos que el siguiente paquete a enviar se encuentra entre 
  los valores esperados */ 
  else if (CompruebaACK(contenido)) {
    //El valor es valido, por lo que avanzamos la ventana
    m_inicioVentana = contenido;
    //Enviamos los paquetes que faltan por enviar en la ventana
    EnviaPaquete();
  }
  
  /* Si el valor esta fuera de la ventana, el paquete es ignorado */


  // Comprobamos si el número de secuencia del ACK se corresponde con
  // el de secuencia del siguiente paquete a transmitir
  /*if (contenido == m_tx + 1 || 2 * tamVentana - contenido == 0) {
     // Si es correcto desactivo el temporizador
    Simulator::Cancel(m_temporizador);
    // Incrementamos el numero de secuencia
    if (++m_tx == 2 * m_tamVentana + 1) {
      m_tx = 0;
    }
    // Incrementamos el total de paquetes (ELIMINAR)
    m_totalPqt++;
    // Se transmite un nuevo paquete
    m_paquete = Create<Packet> (&m_tx, m_tamPqt + 1);
    EnviaPaquete();   
  }*/
}

int
BitAlternanteTx::CompruebaACK(uint8_t contenido) {
  
  int resultado = 0;  
  //Evitamos el desborde
  if (contenido < m_inicioVentana) {
    contenido = contenido + m_tamVentana;
  }  

  if (contenido > m_inicioVentana &&
      contenido <= m_inicioVentana + m_tamVentana) {
    resultado = 1;
  }
  return resultado;
}

void BitAlternanteTx::IncrementaNumSeq() {
  NS_LOG_FUNCTION("m_tx inicial = " << m_tx);
  if (++m_tx == 2 * m_tamVentana) {
    m_tx = 0;
  }
  NS_LOG_FUNCTION("m_tx final = " << m_tx);
}


void
BitAlternanteTx::VenceTemporizador()
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_LOG_DEBUG("¡¡¡TIMEOUT!!! Reenviando");
  // Reenviamos el último paquete transmitido
  EnviaPaquete ();
}


/**
 * EnviaPaquete: transmite paquetes deste m_tx hasta el final de la ventana.
 */
void
BitAlternanteTx::EnviaPaquete()
{
  NS_LOG_FUNCTION_NOARGS ();

  // Paquete a enviar 
  Ptr<Packet> m_paquete;
  
  //Se transmiten los paquetes desde m_tx hasta el final de la ventana
  for ( ; m_tx <= m_inicioVentana + m_tamVentana; IncrementaNumSeq()) {  
    // Envío el paquete  
    m_paquete = Create<Packet> (&m_tx, m_tamPqt + 1);
    m_node->GetDevice(0)->Send(m_paquete, m_disp->GetAddress(), 0x0800);

    NS_LOG_DEBUG ("Transmitido paquete de " << m_paquete->GetSize () <<
                 " octetos en nodo " << m_node->GetId() <<
                 " con " << (unsigned int) m_tx <<
                 " en " << Simulator::Now());

  }
  NS_LOG_FUNCTION ("Salimos del bucle");
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
    IncrementaNumSeq();
  }
  // Transmito en cualquier caso un ACK
  EnviaACK();
}

void BitAlternanteRx::IncrementaNumSeq() {
  if (++m_rx == 2 * m_tamVentana + 1)
    m_rx = 0;
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


