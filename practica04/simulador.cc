/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/data-rate.h>
#include <ns3/error-model.h>
#include "Enlace.h"
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
  uint8_t  tamVentana = 2;

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
  
  //Habilitamos la creacion de pcaps
  escenario.EnablePcapAll("practica04");
  
  Observador primerObservador;
  // Suscribimos la traza de paquetes correctamente asentidos de la primera aplicacion.
  dispositivos.Get (0)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &primerObservador));
  
  Observador segundoObservador;
  // Suscribimos la traza de paquetes correctamente asentidos de la segunda aplicacion
  dispositivos.Get (1)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &segundoObservador));
  
  // Primera aplicación 
  Enlace primeraAplicacion (dispositivos.Get (1), trtx, tamPaquete, tamVentana);
  // Segunda aplicación
  Enlace segundaAplicacion (dispositivos.Get (0), trtx, tamPaquete, tamVentana);
  
  // Añadimos cada aplicación a su nodo
  nodos.Get (0)->AddApplication(&primeraAplicacion);
  nodos.Get (1)->AddApplication(&segundaAplicacion);

  // Activamos las aplicaciones
  primeraAplicacion.SetStartTime (Seconds (1.0));
  primeraAplicacion.SetStopTime (Seconds (9.95));
  segundaAplicacion.SetStartTime (Seconds (1.0));
  segundaAplicacion.SetStopTime (Seconds (9.95));
  
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_DEBUG ("TamPaquete: " << tamPaquete + 6);
  NS_LOG_DEBUG ("Vtx: " << vtx);
  NS_LOG_DEBUG ("Rprop: " << rprop);
  NS_LOG_DEBUG ("RTT: " << Seconds(vtx.CalculateTxTime (tamPaquete + 6)) + 2 * rprop); //Enunciado modificado
  NS_LOG_DEBUG ("Temporizador: " << trtx);
  NS_LOG_INFO  ("Total paquetes primer nodo: " << primerObservador.TotalPaquetes ());
  NS_LOG_INFO  ("Total paquetes segundo nodo: " << segundoObservador.TotalPaquetes());
  return 0;
}
