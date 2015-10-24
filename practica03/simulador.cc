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
  BitAlternanteTx transmisor(dispRx, "10ms", 2048);
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

  NS_LOG_UNCOND ("Voy a simular");
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_UNCOND ("Total paquetes: " << transmisor.TotalDatos());

  return 0;
}
