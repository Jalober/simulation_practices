/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/node.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/point-to-point-channel.h>
#include <ns3/drop-tail-queue.h>
#include "BitAlternante.h"



using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Practica03");


int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::MS);

  //Variables para parametros de linea de comandos
  DataRate velocidadTxDesde(1);
  DataRate velocidadTxHasta(100);
  Time     retardoPropDesde(1);
  Time     retardoPropHasta(100);
  Time     tRetransmisionDesde(1);  
  Time     tRetransmisionHasta(100);

  //Obtencion de parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue("velocidadTxDesde", "valor de la velocidad de transmision inicial", velocidadTxDesde);
  cmd.AddValue("velocidadTxHasta", "valor de la velocidad de transmision final", velocidadTxHasta);
  cmd.AddValue("retardoPropDesde", "retardo de propagacion inicial", retardoPropDesde);
  cmd.AddValue("retardoPropHasta", "retardo de propagacion final", retardoPropHasta);
  cmd.AddValue("tRetransmisionDesde", "temporizador de retransmisiones inicial", tRetransmisionDesde);
  cmd.AddValue("tRetransmisionHasta", "temporizador de retransmisiones final", tRetransmisionHasta);
  cmd.Parse(argc, argv);

  // Componentes del escenario:
  // Dos nodos
  Ptr<Node> nodoTx = CreateObject<Node> ();
  Ptr<Node> nodoRx = CreateObject<Node> ();
  // Dos dispositivos de red
  Ptr<PointToPointNetDevice> dispTx = CreateObject<PointToPointNetDevice> ();
  Ptr<PointToPointNetDevice> dispRx = CreateObject<PointToPointNetDevice> ();
  // Un canal punto a punto
  Ptr<PointToPointChannel> canal = CreateObject<PointToPointChannel> ();;
  // Una aplicación transmisora
  BitAlternanteTx transmisor(dispRx, Time("10ms"), 994);
  // Y una receptora
  BitAlternanteRx receptor(dispTx);

  // Montamos el escenario:
  // Añadimos una cola a cada dispositivo
  dispTx->SetQueue (CreateObject<DropTailQueue> ());
  dispRx->SetQueue (CreateObject<DropTailQueue> ());
  // Añadimos cada dispositivo a su nodo
  nodoTx->AddDevice (dispTx);
  nodoRx->AddDevice (dispRx);
  // Añadimos cada aplicación a su nodo
  nodoTx->AddApplication(&transmisor);
  nodoRx->AddApplication(&receptor);
  // Asociamos los dos dispositivos al canal
  dispTx->Attach (canal);
  dispRx->Attach (canal);

  // Modificamos los parámetos configurables
  canal->SetAttribute ("Delay", StringValue ("2ms"));
  dispTx->SetAttribute ("DataRate", StringValue ("5Mbps"));

  // Activamos el transmisor
  transmisor.SetStartTime (Seconds (1.0));
  transmisor.SetStopTime (Seconds (10.0));

  //NS_LOG_UNCOND ("Voy a simular");
  //Creacion de la primera grafica
  for (i = 0; i < 5; i++) {
  //Bucle para la creación de cada curva
  //Configuración del parametro (retardo de propagacion)
  canal->SetAttribute ("Delay", StringValue ("2ms"));
      for (j = 0; j < 10; j++) {
          //Configuracion de aplicacion (temporizador de retransmision)
          BitAlternanteTx transmisor(dispRx, Time(""), 994);
      
          Simulator::Run ();
          Simulator::Destroy ();
      }
  }

  NS_LOG_UNCOND ("Total paquetes: " << transmisor.TotalDatos());

  return 0;
}
