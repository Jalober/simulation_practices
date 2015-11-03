/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/data-rate.h>
#include <ns3/error-model.h>
#include "BitAlternante.h"
#include "Observador.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Practica04");


int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::US);

  // Parámetros de la simulación
  Time     trtx       = Time("5ms");
  uint32_t tamPaquete = 994;
  Time     rprop      = Time("2ms");
  DataRate vtx        = DataRate("1000kbps");

  // Configuramos el escenario:
  PointToPointHelper escenario;
  escenario.SetChannelAttribute ("Delay", TimeValue (rprop));
  escenario.SetDeviceAttribute ("DataRate", DataRateValue (vtx));
  escenario.SetQueue ("ns3::DropTailQueue");
  // Creamos los nodos
  NodeContainer      nodos;
  nodos.Create(2);
  // Creamos el escenario
  NetDeviceContainer dispositivos = escenario.Install (nodos);

  Observador observador;
  // Suscribimos la traza de paquetes correctamente asentidos.
  dispositivos.Get (0)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &observador));

  // Una aplicación transmisora
  BitAlternanteTx transmisor (dispositivos.Get (1), trtx, tamPaquete);
  // Y una receptora
  BitAlternanteRx receptor(dispositivos.Get (0));
  // Añadimos cada aplicación a su nodo
  nodos.Get (0)->AddApplication(&transmisor);
  nodos.Get (1)->AddApplication(&receptor);

  // Activamos el transmisor
  transmisor.SetStartTime (Seconds (1.0));
  transmisor.SetStopTime (Seconds (9.95));
  
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_DEBUG ("TamPaquete: " << tamPaquete + 6);
  NS_LOG_DEBUG ("Vtx: " << vtx);
  NS_LOG_DEBUG ("Rprop: " << rprop);
  NS_LOG_DEBUG ("RTT: " << vtx.CalculateTxTime (tamPaquete + 6) + 2 * rprop);
  NS_LOG_DEBUG ("Temporizador: " << trtx);
  NS_LOG_INFO  ("Total paquetes: " << observador.TotalPaquetes ());
  
  return 0;
}
