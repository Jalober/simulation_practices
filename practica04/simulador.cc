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
  Time     trtx       = Time("6ms");
  uint32_t tamPaquete = uint32_t(121);
  Time     rprop      = Time("200us");
  DataRate vtx        = DataRate("1000kbps");
  uint8_t  tamVentana = 6;
  double   errorRate  = double(0.001);

  //Obtencion de parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue("window", "tamanio de la ventana de transmision", tamVentana);
  cmd.AddValue("delay", "retardo de propagacion del enlace", rprop);
  cmd.AddValue("rate", "capacidad de transmision en el canal", vtx);
  cmd.AddValue("pktSize", "tamanio de la SDU del nivel de enlace", tamPaquete);
  cmd.AddValue("wait", "tiempo de espera para la retransmision", trtx);  
  cmd.AddValue("errorRate", "media de errores por paquete", errorRate);
  cmd.Parse(argc, argv);
  
  // Definimos el modelo de errores
  Ptr<ErrorModel> errorModel = CreateObject<RateErrorModel> ();
  errorModel->SetAttribute("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
  errorModel->SetAttribute("ErrorRate", DoubleValue (errorRate));
  
  // Configuramos el escenario:
  PointToPointHelper escenario;
  escenario.SetChannelAttribute ("Delay", TimeValue (rprop));
  escenario.SetDeviceAttribute ("DataRate", DataRateValue (vtx));
  escenario.SetDeviceAttribute ("ReceiveErrorModel", PointerValue(errorModel));
  escenario.SetQueue ("ns3::DropTailQueue");
 
  // Creamos los nodos
  NodeContainer nodos;
  nodos.Create(2);

  // Creamos el escenario
  NetDeviceContainer dispositivos = escenario.Install (nodos);
 
  // Habilitamos la creacion de pcaps
  escenario.EnablePcapAll("practica04");
    
  Observador observador(vtx, Time("5s"), tamVentana);
  // Suscribimos la traza de paquetes correctamente asentidos de la primera aplicacion.
  dispositivos.Get (0)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &observador));
  dispositivos.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback(&Observador::PaqueteErroneo, &observador)); 
  dispositivos.Get (1)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &observador));
  dispositivos.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback(&Observador::PaqueteErroneo, &observador));

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
  NS_LOG_DEBUG ("Temporizador" << trtx);  
  NS_LOG_FUNCTION  ("Total paquetes correctos"  << observador.TotalPaquetes ());
  NS_LOG_FUNCTION  ("Total paquetes erroneos"   << observador.TotalErroneos ());
  
  NS_LOG_INFO ("Cef: " << observador.GETCef());
  NS_LOG_INFO ("Rend: " << observador.GETRend());

  return 0;
}
