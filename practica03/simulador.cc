/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/node.h>
#include <ns3/point-to-point-net-device.h>
#include <ns3/point-to-point-channel.h>
#include <ns3/drop-tail-queue.h>
#include <ns3/gnuplot.h>
#include "BitAlternante.h"

#define TAM_PAQUETE 994

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
  //BitAlternanteTx transmisor(dispRx, Time("10ms"), 994);
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

  /*// Modificamos los parámetos configurables
  canal->SetAttribute ("Delay", StringValue ("2ms"));
  dispTx->SetAttribute ("DataRate", StringValue ("5Mbps"));
  */
  /*
  transmisor.SetStartTime (Seconds (1.0));
  transmisor.SetStopTime (Seconds (10.0));
  */
  
  
  
  //NS_LOG_UNCOND ("Voy a simular");
  
  /* 
   * PRIMERA GRAFICA
   */
  NS_LOG_INFO ("############################");
  NS_LOG_INFO ("# PRIMERA GRAFICA ##########");
  NS_LOG_INFO ("############################");
  Gnuplot plot1;
  //TODO: plot1.SetTitle("");
  
  //Calculo de la velocidad de transmision media
  DataRate velocidadTxMedia((velocidadTxDesde.GetBitRate() + velocidadTxHasta.GetBitRate()) / 2);
  NS_LOG_INFO ("velocidadTxMedia: " << velocidadTxMedia.GetBitRate());
  NS_LOG_INFO ("############################");
  //Configuracion de la constante (velocidad transmision)  
  dispTx->SetAttribute ("DataRate", DataRateValue(velocidadTxMedia));
  //Bucle para la creación de cada curva
  for (int i = 0; i < 5 ; i++) {  
    //Calculo del parametro (retardo de propagacion)
    Time retardoPropActual(retardoPropDesde.GetDouble() + i * (retardoPropHasta.GetDouble() - retardoPropDesde.GetDouble()) / 4);
    NS_LOG_INFO ("retardoPropActual: " << retardoPropActual.GetDouble());
    NS_LOG_INFO ("***************************");
    //Configuración del parametro (retardo de propagacion)
    canal->SetAttribute ("Delay", TimeValue(retardoPropActual));
    //Creacion de una nueva curva
    Gnuplot2dDataset dataset;
    for (int j = 0; j < 10; j++) {
      //Calculo del temporizador de retransmision
      Time tRetransmisionActual(tRetransmisionDesde.GetDouble() + j * (tRetransmisionHasta.GetDouble() - tRetransmisionDesde.GetDouble() / 9));
      //Configuracion de aplicacion (temporizador de retransmision)          
      BitAlternanteTx transmisor(dispRx, tRetransmisionActual, TAM_PAQUETE);
      transmisor.SetStartTime (Seconds (1.0));
      transmisor.SetStopTime (Seconds (10.0));
      Simulator::Run ();  
      Simulator::Destroy ();
      dataset.Add(tRetransmisionActual.GetDouble(), transmisor.TotalDatos());

      NS_LOG_INFO ("   tRetransmisionActual: " << tRetransmisionActual.GetDouble());
      NS_LOG_INFO ("   Total paquetes: " << transmisor.TotalDatos());
      NS_LOG_INFO ("-----------------------");
    }
    //Añadimos el dataset a la grafica
    plot1.AddDataset(dataset);
  }

  
  /*
   * SEGUNDA GRAFICA
   */
  NS_LOG_INFO ("############################");
  NS_LOG_INFO ("# SEGUNDA GRAFICA ##########");
  NS_LOG_INFO ("############################");

  Gnuplot plot2;
  //TODO: plot2.SetTitle("");
  
  //Calculo del retardo de propagacion
  Time retardoPropMedio((retardoPropDesde.GetDouble() + retardoPropHasta.GetDouble()) / 2);
  NS_LOG_INFO ("retardoPropMedio: " << retardoPropMedio.GetDouble());
  NS_LOG_INFO ("############################");

  //Configuracion de la constante (retardo de propagacion)
  canal->SetAttribute ("Delay", TimeValue(retardoPropMedio));
  //Bucle para la creación de cada curva
  for (int i = 0; i < 5; i++) {
    //Calculo del parametro (velocidad de transmision)
    DataRate velocidadTxActual(velocidadTxDesde.GetBitRate() + i * (velocidadTxHasta.GetBitRate() - velocidadTxDesde.GetBitRate()) / 4);
    NS_LOG_INFO ("velocidadTxActual: " << velocidadTxActual.GetBitRate());
    NS_LOG_INFO ("***************************");
    //Configuracion del parametro (velocidad transmision)
    dispTx->SetAttribute ("DataRate", DataRateValue(velocidadTxActual));
    //Creacion de la curva
    Gnuplot2dDataset dataset;
    for (int j = 0; j < 10; j++) {
      //Calculo del temporizador de retransmision
      Time tRetransmisionActual(tRetransmisionDesde.GetDouble() + j * (tRetransmisionHasta.GetDouble() - tRetransmisionDesde.GetDouble() / 9));
      //Configuracion de aplicacion (temporizador de retransmision)          
      BitAlternanteTx transmisor(dispRx, tRetransmisionActual, TAM_PAQUETE);
      transmisor.SetStartTime (Seconds (1.0));
      transmisor.SetStopTime (Seconds (10.0));
      Simulator::Run ();  
      Simulator::Destroy ();
      dataset.Add(tRetransmisionActual.GetDouble(), transmisor.TotalDatos());
      
      NS_LOG_INFO ("   tRetransmisionActual: " << tRetransmisionActual.GetDouble());
      NS_LOG_INFO ("   Total paquetes: " << transmisor.TotalDatos());
      NS_LOG_INFO ("-----------------------");
    }
    //Añadimos el dataset a la grafica
    plot2.AddDataset(dataset);
  }

  NS_LOG_UNCOND ("Total paquetes: " << transmisor.TotalDatos());

  return 0;
}
