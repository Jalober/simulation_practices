/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/point-to-point-helper.h>
#include <ns3/data-rate.h>
#include <ns3/error-model.h>
#include "Enlace.h"
#include "Observador.h"

#define T_STUDENT_VALUE 1.8331

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Practica04");


  double simulacion (Time trtx, uint32_t tamPaquete, Time rprop, DataRate vtx, uint8_t tamVentana, double errorRate) 
  
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
  // escenario.EnablePcapAll("practica04");
    
  Observador observador1(vtx, Time("5s"), tamVentana, tamPaquete);
  //Observador observador2(vtx, Time("5s"), tamVentana, tamPaquete);
  // Suscribimos la traza de paquetes correctamente asentidos de la primera aplicacion.
  dispositivos.Get (0)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &observador1));
  dispositivos.Get (0)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback(&Observador::PaqueteErroneo, &observador1)); 
  //dispositivos.Get (1)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteAsentido, &observador2));
  //dispositivos.Get (1)->TraceConnectWithoutContext ("PhyRxDrop", MakeCallback(&Observador::PaqueteErroneo, &observador2));

  // Primera aplicación 
  Enlace primeraAplicacion (dispositivos.Get (1), trtx, tamPaquete, tamVentana);
  // Segunda aplicación
  Enlace segundaAplicacion (dispositivos.Get (0), trtx, tamPaquete, tamVentana);
  
  // Añadimos cada aplicación a su nodo
  nodos.Get (0)->AddApplication(&primeraAplicacion);
  nodos.Get (1)->AddApplication(&segundaAplicacion);

  // Activamos las aplicaciones
  primeraAplicacion.SetStartTime (Seconds (1.0));
  primeraAplicacion.SetStopTime (Seconds (6.0));
  segundaAplicacion.SetStartTime (Seconds (1.0));
  segundaAplicacion.SetStopTime (Seconds (6.0));
  
  Simulator::Run ();
  Simulator::Destroy ();

  NS_LOG_DEBUG ("TamPaquete: " << tamPaquete + 6);
  NS_LOG_DEBUG ("Vtx: " << vtx);
  NS_LOG_DEBUG ("Rprop: " << rprop);
  NS_LOG_DEBUG ("RTT: " << Seconds(vtx.CalculateTxTime (tamPaquete + 6)) + 2 * rprop); //Enunciado modificado
  NS_LOG_DEBUG ("Temporizador" << trtx);
  
  uint32_t totalPaquetes = observador1.TotalPaquetes();
  uint32_t totalErroneos = observador1.TotalErroneos();

  NS_LOG_FUNCTION  ("Total paquetes correctos"  << totalPaquetes);
  NS_LOG_FUNCTION  ("Total paquetes erroneos"   << totalErroneos);
  
  //Calculo de estadisticos
  DataRate cef  = observador1.GETCef(); 
  double   rend = observador1.GETRend();  
  
  NS_LOG_INFO ("Cef: " << cef);
  NS_LOG_INFO ("Rend: " << rend);

  return rend;
}

int
main (int argc, char *argv[])
{
  Time::SetResolution (Time::US);

  uint32_t seed = 1; //Semilla a utilizar

  // Parámetros de la simulación
  Time     trtx       = Time ("6ms");
  uint32_t tamPaquete = uint32_t (121);
  Time     rprop      = Time ("200us");
  DataRate vtx        = DataRate ("1000kbps");
  uint8_t  tamVentana = 6;
  double   errorRate  = double (0);

  //Obtencion de parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue ("window", "tamanio de la ventana de transmision", tamVentana);
  cmd.AddValue ("delay", "retardo de propagacion del enlace", rprop);
  cmd.AddValue ("rate", "capacidad de transmision en el canal", vtx);
  cmd.AddValue ("pktSize", "tamanio de la SDU del nivel de enlace", tamPaquete);
  cmd.AddValue ("wait", "tiempo de espera para la retransmision", trtx);  
  cmd.Parse (argc, argv);

  Gnuplot plot;
  std::ostringstream rotulo;
  plot.SetTitle ("Rendimiento en funcion de la probabilidad de error de paquete");
  plot.SetLegend ("perror", "rend");+
  
  Gnuplot2dDataset dataset;
  dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
  dataset.SetErrorBars(Gnuplot2dDataset::Y);

  for (errorRate = 0; errorRate <= 0.5; errorRate += 0.05) {
    Average <double> rendimientos; 
    for (int i = 0; i < 10; i++) {
      //Modificamos la semilla de la simulacion
      SeedManager::SetRun (seed++);
      double rendimiento = simulacion (trtx, tamPaquete, rprop, vtx, tamVentana, errorRate);      
      rendimientos.Update (rendimiento);
    }
    double rendimientoMedio = rendimientos.Mean();
    double z = T_STUDENT_VALUE * std::sqrt (rendimientos.Var () / 10);
    rendMedio.Add (errorRate, rendimientoMedio, 2 * z);    
    NS_LOG_INFO ("Para errorRate = " << errorRate << " --> "
       << rendimientoMedio - z << " < rendimiento < " << rendimientoMedio + <);  
  } 

  
  return 0;
}
