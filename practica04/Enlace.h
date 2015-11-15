/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

using namespace ns3;

#include "ns3/node.h"
#include "ns3/net-device.h"
#include "ns3/application.h"
#include "Ventana.h"

#define PAQUETE 0
#define ACK 1

class Enlace : public Application
{
public:

  // Constructor de la clase. Necesita como parámetros el puntero al dispositivo de red
  // con el que debe comunicarse, el temporizador de retransmisiones y el tamaño de
  // paquete. Inicializa las variables privadas.
  Enlace(Ptr<NetDevice>, Time, uint32_t tamPqt, uint8_t tamVentana);

  
  void PaqueteRecibido(Ptr<NetDevice> receptor, Ptr<const Packet> recibido,
                   uint16_t protocolo, const Address &desde, const Address &hacia,
                   NetDevice::PacketType tipoPaquete);

  // Función de vencimiento del temporizador
  void VenceTemporizador ();
  
  // Función que envía un paquete.
  void EnviaPaquete();

  // Función que envía un ACK.
  void EnviaACK();

private:
  // Método de inicialización de la aplicación.
  // Se llama sólo una vez al inicio.
  // En nuestro caso sirve para instalar el Callback que va a procesar
  // los asentimientos recibidos.
  void DoInitialize()
  {
    // Solicitamos que nos entreguen (mediante la llamada a ACKRecibido)
    // cualquier paquete que llegue al nodo.
    m_node->RegisterProtocolHandler (ns3::MakeCallback(&Enlace::PaqueteRecibido,
                                                       this),
                                     0x0000, 0, false);
    Application::DoInitialize();
  };

  // Método que se llama en el instante de comienzo de la aplicación.
  void StartApplication()
  {
    //Envio de los primeros paquetes    
    if (m_tamPqt > 0) { 
       EnviaPaquete();
    }
  }

  // Método que se llama en el instante de final de la aplicación.
  void StopApplication()
  {
    Simulator::Stop ();
  }

  // Dispositivo de red con el que hay que comunicarse.
  Ptr<NetDevice> m_disp;
  // Temporizador de retransmisión
  Time           m_esperaACK;
  // Tamaño del paquete
  uint32_t       m_tamPqt;  
  // Evento de retransmision
  EventId        m_temporizador;
  // Objeto para gestion de ventana;
  Ventana        m_ventana;  
  // tamaño de la ventana de TRANSMISION
  uint32_t       m_tamVentana;
  // NumSeq de paquete recibido
  uint8_t        m_rx;
  // Flag para indicar si es el primer envio
  bool           m_primerEnvio;
};
