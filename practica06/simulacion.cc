/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include <ns3/gnuplot.h>
#include <sstream>
#include "Observador.h"

#define NUM_SIMULACIONES 2
#define T_STUDENT_VALUE 4.303


typedef struct datos {
  Time retardoMedio;
  double porcentajePaquetesCorrectos;
} DATOS; 

// Default Network Topolo
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Practica06");

DATOS simulacion (uint32_t nCsma, Time ton, Time toff, uint32_t sizePkt, DataRate dataRate, uint32_t tamCola) {

  // Nodos que pertenecen al enlace punto a punto
  NodeContainer p2pNodes;
  p2pNodes.Create (2);

  // Nodos que pertenecen a la red de área local
  // Como primer nodo añadimos el encaminador que proporciona acceso
  //      al enlace punto a punto.
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

  // Instalamos el dispositivo en los nodos punto a punto
  PointToPointHelper pointToPoint;
  NetDeviceContainer p2pDevices;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("2Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  pointToPoint.SetQueue("ns3::DropTailQueue", "MaxPackets", UintegerValue(tamCola));
  p2pDevices = pointToPoint.Install (p2pNodes);

  // Instalamos el dispositivo de red en los nodos de la LAN
  CsmaHelper csma;
  NetDeviceContainer csmaDevices;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));
  csmaDevices = csma.Install (csmaNodes);

  // Instalamos la pila TCP/IP en todos los nodos
  InternetStackHelper stack;
  stack.Install (p2pNodes.Get (0));
  stack.Install (csmaNodes);

  // Asignamos direcciones a cada una de las interfaces
  // Utilizamos dos rangos de direcciones diferentes:
  //    - un rango para los dos nodos del enlace
  //      punto a punto
  //    - un rango para los nodos de la red de área local.
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterfaces;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pInterfaces = address.Assign (p2pDevices);
  Ipv4InterfaceContainer csmaInterfaces;
  address.SetBase ("10.1.2.0", "255.255.255.0");
  csmaInterfaces = address.Assign (csmaDevices);

  // Calculamos las rutas del escenario. Con este comando, los
  //     nodos de la red de área local definen que para acceder
  //     al nodo del otro extremo del enlace punto a punto deben
  //     utilizar el primer nodo como ruta por defecto.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  
  // Establecemos un sumidero para los paquetes en el puerto 9 del nodo
  //     aislado del enlace punto a punto
  uint16_t port = 9;
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port)));
  ApplicationContainer app = sink.Install (p2pNodes.Get (0));

  // Instalamos un cliente OnOff en uno de los equipos de la red de área local
  OnOffHelper clientes ("ns3::UdpSocketFactory",
                        Address (InetSocketAddress (p2pInterfaces.GetAddress (0), port)));
  clientes.SetAttribute("PacketSize", UintegerValue(sizePkt));
      
  Ptr<ExponentialRandomVariable> onTimeExp = CreateObject<ExponentialRandomVariable> ();
  onTimeExp->SetAttribute ("Mean", DoubleValue (ton.GetSeconds()));
  clientes.SetAttribute("OnTime", PointerValue (onTimeExp));
  Ptr<ExponentialRandomVariable> offTimeExp = CreateObject<ExponentialRandomVariable> ();
  offTimeExp->SetAttribute ("Mean", DoubleValue (toff.GetSeconds()));
  clientes.SetAttribute("OffTime", PointerValue (offTimeExp));
  
  clientes.SetAttribute ("DataRate", DataRateValue(dataRate));
  clientes.SetAttribute ("PacketSize", UintegerValue (sizePkt));    
  ApplicationContainer clientApps = clientes.Install (csmaNodes);
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));
 
  // Activamos las trazas pcap en las dos interfaces del nodo de enlace
  pointToPoint.EnablePcap ("practica06", p2pDevices.Get (1));
  csma.EnablePcap ("practica06", csmaDevices.Get (0), true);
  
  Observador observador;
  for (unsigned int i = 0; i <= nCsma; i++) {
    csmaNodes.Get(i)->GetApplication(0)->TraceConnectWithoutContext ("Tx", MakeCallback(&Observador::EnvioPaquete, &observador));
  }  
  //p2pDevices.Get(0)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteRecibido, &observador));  
  p2pNodes.Get(0)->GetApplication(0)->TraceConnectWithoutContext ("Rx", MakeCallback(&Observador::PaqueteRecibido, &observador));
  // Lanzamos la simulación
  Simulator::Run ();
  Simulator::Destroy ();
 
  unsigned int tamMapa = observador.MapSize();
  if (tamMapa != 0) {
    NS_LOG_INFO ("Mapa de paquetes enviado no vacío (" << tamMapa << ")!");
  }
  
  Time retardoMedio = observador.GetRetardoMedio();
  double porcentajePaquetesCorrectos = observador.GetPorcentajePaquetesCorrectos();
  
  NS_LOG_INFO ("retardo medio: " << retardoMedio.GetMicroSeconds() << " us"); 
  NS_LOG_INFO ("porcentaje paquetes correctos: " << porcentajePaquetesCorrectos << " %");
  
  DATOS datos = { retardoMedio, porcentajePaquetesCorrectos };
  return datos;
}

int 
main (int argc, char *argv[])
{
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
  Time::SetResolution (Time::US);

  uint32_t dni[] = {3,1,4,8,2,2,0,3};
  NS_LOG_INFO ("DNI: " << dni[7]<<dni[6]<<dni[5]<<dni[4]<<dni[3]<<dni[2]<<dni[1]<<dni[0]);

  //Valores por defecto de los parámetros
  uint32_t nCsma = 150 + 5*dni[0];
  Time     ton  ("150ms");
  Time     toff ("650ms");
  uint32_t sizePkt = 40 - dni[1]/2;
  DataRate dataRate ("64Kbps");

  //Obtención de parámetros por línea de comandos
  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("ton", "valor inicial del tiempo medio de permanencia en el estado On", ton);
  cmd.AddValue ("toff", "tiempo medio de permanencia en el estado Off", toff);
  cmd.AddValue ("sizePkt", "tamaño de paquete", sizePkt);
  cmd.AddValue ("dataRate", "tasa de bit en el estado activo", dataRate);
  cmd.Parse (argc,argv);

  nCsma = nCsma == 0 ? 1 : nCsma;

  Time intervalo = Time("50ms");

  /* Preparacion de las graficas */
  Gnuplot plot[2];
  plot[0].SetTitle ("Porcentaje de paquetes correctamente transmitidos");
  plot[0].SetLegend ("tMedioEstadoOn (ms)", "PqtsCorrectos (%)");
  plot[1].SetTitle ("Retardo medio");
  plot[1].SetLegend ("tMedioEstadoOn (ms)", "retardo medio (us)");

  Gnuplot2dDataset datasetPlot1[5];
  Gnuplot2dDataset datasetPlot2[5];
  for (uint32_t i = 0; i < 5; i++) {
    datasetPlot1[i].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    datasetPlot1[i].SetErrorBars(Gnuplot2dDataset::Y);
    datasetPlot2[i].SetStyle (Gnuplot2dDataset::LINES_POINTS);
    datasetPlot2[i].SetErrorBars(Gnuplot2dDataset::Y);
  }  

  for (uint32_t tamCola = 1; tamCola <= 5; tamCola++) { //bucle para cada curva
    NS_LOG_DEBUG("##################################");
    NS_LOG_DEBUG("TAM_COLA = " << tamCola);
    NS_LOG_DEBUG("##################################");
    std::ostringstream rotulo;
    rotulo << "tamCola: " << tamCola;
    datasetPlot1[tamCola-1].SetTitle(rotulo.str());
    datasetPlot2[tamCola-1].SetTitle(rotulo.str());
    double tonActual = ton.GetDouble();
    for (int i = 0; i < 5; i++) { //Bucle para cada punto de la curva     
      Average<double> mediaRetardo;
      Average<double> mediaPorcentajePaquetesCorrectos; 
      for (int j = 0; j < NUM_SIMULACIONES; j++) { //Bucle para cada pack de simulaciones
        DATOS datos = simulacion(nCsma, Time(tonActual), toff, sizePkt, dataRate, tamCola);        
        mediaPorcentajePaquetesCorrectos.Update(datos.porcentajePaquetesCorrectos);
        mediaRetardo.Update(datos.retardoMedio.GetDouble());      
      }      
      double z1 = T_STUDENT_VALUE * std::sqrt (mediaPorcentajePaquetesCorrectos.Var() / NUM_SIMULACIONES);
      double z2 = T_STUDENT_VALUE * std::sqrt (mediaRetardo.Var () / NUM_SIMULACIONES);
      datasetPlot1[tamCola-1].Add(Time(tonActual).GetMilliSeconds(), mediaPorcentajePaquetesCorrectos.Mean(), z1);
      datasetPlot2[tamCola-1].Add(Time(tonActual).GetMilliSeconds(), Time(mediaRetardo.Mean()).GetMicroSeconds(), z2);      
      NS_LOG_DEBUG( "Ton = " << Time(tonActual).GetMilliSeconds() << 
                    " ms\tPktCorrectos = " << mediaPorcentajePaquetesCorrectos.Mean() <<
                    " %\tretardo = " << Time(mediaRetardo.Mean()).GetMicroSeconds() << " us");
      tonActual = tonActual + intervalo.GetDouble();
    }
  }

  for(int i = 0; i < 5; i++) {
    plot[0].AddDataset (datasetPlot1[i]);
  }
  std::ofstream plotFile1 ("practica06-1.plt");
  plot[0].GenerateOutput (plotFile1);
  plotFile1 << "pause -1" << std::endl;
  plotFile1.close ();

  for(int i = 0; i < 5; i++) {
    plot[1].AddDataset (datasetPlot2[i]);
  }  
  std::ofstream plotFile2 ("practica06-2.plt");
  plot[1].GenerateOutput (plotFile2);
  plotFile2 << "pause -1" << std::endl;
  plotFile2.close ();
  
  return 0;
}
