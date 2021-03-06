/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include <ns3/gnuplot.h>
#include <sstream>


#include "Observador.h"

#define STARTTIME 2.0
#define STOPTIME  10000.0
#define T_STUDENT_VALUE 2.2622

typedef struct datos {
    double mediaNumIntentosTotales;
    double mediaTiempoEcoTotal;
    double mediaPorcentajeErrorClientes;
} DATOS; 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("practica05");


DATOS simulacion (uint32_t nCsma, Time retardoProp, DataRate capacidad, 
        uint32_t tamPaquete, Time intervalo, uint32_t maxReintentos, double tSimulacion) {
    
    NodeContainer csmaNodes;
    csmaNodes.Create (nCsma);

    CsmaHelper csma;
    csma.SetChannelAttribute ("DataRate", DataRateValue (capacidad));
    csma.SetChannelAttribute ("Delay", TimeValue (retardoProp));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install (csmaNodes);
    // Instalamos la pila TCP/IP en todos los nodos
    InternetStackHelper stack;
    stack.Install (csmaNodes);
    // Y les asignamos direcciones
    Ipv4AddressHelper address;
    address.SetBase ("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces = address.Assign (csmaDevices);

    /////////// Instalación de las aplicaciones
    // Servidor
    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApp = echoServer.Install (csmaNodes.Get (nCsma - 1));
    serverApp.Start (Seconds (STARTTIME));
    serverApp.Stop (Seconds (STARTTIME + tSimulacion));
    // Clientes
    UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma - 1), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
    echoClient.SetAttribute ("Interval", TimeValue (intervalo));
    echoClient.SetAttribute ("PacketSize", UintegerValue (tamPaquete));
    NodeContainer clientes;

    for (uint32_t i = 0; i < nCsma - 1; i++)
    {
        clientes.Add (csmaNodes.Get (i));
    }
    NS_LOG_FUNCTION ("Fin bucle de creacion de observadores");

    ApplicationContainer clientApps = echoClient.Install (clientes);
    NS_LOG_FUNCTION ("Instalacion de clientes de echo realizada");
    clientApps.Start (Seconds (STARTTIME));
    clientApps.Stop (Seconds (STARTTIME + tSimulacion));

    // Cálculo de rutas
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    csma.EnablePcap ("practica05", csmaDevices.Get (nCsma - 1), true);

    Observador observadores[nCsma]; 
    for (uint32_t i = 0; i < nCsma; i++) {
        csmaDevices.Get(i)->TraceConnectWithoutContext ("PhyTxEnd",     MakeCallback(&Observador::PaqueteEnviado,              &observadores[i]));
        csmaDevices.Get(i)->TraceConnectWithoutContext ("PhyTxDrop",    MakeCallback(&Observador::PaquetePerdido,              &observadores[i]));
        csmaDevices.Get(i)->TraceConnectWithoutContext ("MacTxBackoff", MakeCallback(&Observador::PaqueteEnBackoff,            &observadores[i]));
        csmaDevices.Get(i)->TraceConnectWithoutContext ("MacTx",        MakeCallback(&Observador::PaqueteParaEnviar,           &observadores[i]));
        csmaDevices.Get(i)->TraceConnectWithoutContext ("MacRx",        MakeCallback(&Observador::PaqueteRecibidoParaEntregar, &observadores[i])); 
    
        Ptr<CsmaNetDevice> csma_device = csmaDevices.Get(i)->GetObject<CsmaNetDevice>();
        csma_device->SetBackoffParams (Time ("1us"), 10, 1000, 10, maxReintentos);
    }
 
    NS_LOG_FUNCTION ("Va a comenzar la simulacion");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_FUNCTION ("Finalizada simulacion");


    Average<uint32_t> numIntentosTotales;
    Average<double>   tiempoEcoTotal;
    Average<double>   porcentajeErrorClientes;
    Average<double>   porcentajeErrorEscenario;

    for (uint32_t i = 0; i < nCsma; i++) {
        
        double mediaNumIntentos = observadores[i].GetMediaNumIntentos();
        double mediaTiempoEco = observadores[i].GetMediaTiempoEco();
        double porcentajeError = observadores[i].GetPorcentajePaquetesPerdidos();        

        if (std::isnan(mediaNumIntentos)) {
            mediaNumIntentos = 0;
        }
        if (std::isnan(mediaTiempoEco)) {
            mediaTiempoEco = 0;
        }

        NS_LOG_INFO ("Media de intentos en nodo " << i << ": " << mediaNumIntentos);
        porcentajeErrorEscenario.Update(porcentajeError);
        if (i < nCsma - 1) {
           NS_LOG_INFO ("Tiempo medio de eco en nodo " << i << ": " << Time(mediaTiempoEco));
           if (i > 1) {
               porcentajeErrorClientes.Update(porcentajeError);
               tiempoEcoTotal.Update(mediaTiempoEco);
               numIntentosTotales.Update(mediaNumIntentos);
           }
        }
        NS_LOG_INFO ("Porcentaje de paquetes perdidos en nodo " << i << ": " << porcentajeError << " %");
        NS_LOG_INFO ("");
    }

    double mediaNumIntentosTotales = numIntentosTotales.Mean();
    double mediaTiempoEcoTotal = tiempoEcoTotal.Mean();
    double mediaPorcentajeErrorClientes = porcentajeErrorClientes.Mean();
    double mediaPorcentajeErrorEscenario = porcentajeErrorEscenario.Mean(); 

    if (std::isnan(mediaNumIntentosTotales)) {
        mediaNumIntentosTotales = 0;
    }
    if (std::isnan(mediaTiempoEcoTotal)) {
        mediaTiempoEcoTotal = 0;
    }
    if (std::isnan(mediaPorcentajeErrorClientes)) {
        mediaPorcentajeErrorClientes = 0;
    }
    if (std::isnan(mediaPorcentajeErrorEscenario)) {
        mediaPorcentajeErrorEscenario = 0;
    }
 
    NS_LOG_INFO ("--------------------------------------------------------");
    NS_LOG_INFO ("Media de intentos de transmision en el escenario: " << mediaNumIntentosTotales);
    NS_LOG_INFO ("Tiempo de eco medio en el escenario: " << Time(mediaTiempoEcoTotal));
    NS_LOG_INFO ("Porcentaje de paquetes perdidos por los clientes: " << mediaPorcentajeErrorClientes << " %");
    NS_LOG_INFO ("Porcentaje de paquetes perdidos en el escenario: " << mediaPorcentajeErrorEscenario << " %");
    NS_LOG_INFO ("--------------------------------------------------------");
    
    DATOS datos = { mediaNumIntentosTotales, mediaTiempoEcoTotal, mediaPorcentajeErrorClientes };
    return datos;
}

int
main (int argc, char *argv[])
{
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
    Time::SetResolution (Time::US);

    uint32_t seed = 1;

    uint32_t dni[8] = {3,1,4,8,2,2,0,3};
    NS_LOG_INFO ("DNI: " << dni[8]<<dni[7]<<dni[6]<<dni[5]<<dni[4]<<dni[3]<<dni[2]<<dni[1]);

    //Parametros de simulacion
    uint32_t nCsma             = 10-dni[0]/2;
    Time     retardoProp       = Time ("6560ns");
    DataRate capacidad         = DataRate ("100Mbps");
    uint32_t tamPaquete        = 500+100*dni[1];
    Time     intervalo         = Time ("1s");
    uint32_t reintentosInicial = 1;
    uint32_t reintentosFinal   = 15;
    double   tSimulacion       = 1000;
   
    CommandLine cmd;
    cmd.AddValue ("nCsma", "Número de nodos de la red local", nCsma);
    cmd.AddValue ("retardoProp", "retardo de propagación en el bus", retardoProp);
    cmd.AddValue ("capacidad", "capacidad del bus", capacidad);
    cmd.AddValue ("tamPaquete", "tamaño de las SDU de aplicación", tamPaquete);
    cmd.AddValue ("intervalo", "tiempo entre dos paquetes consecutivos enviados por el mismo cliente", intervalo);
    cmd.AddValue ("reintentosInicial", "valor inicial de reintentos", reintentosInicial);
    cmd.AddValue ("reintentosFinal", "valor final de reintentos", reintentosFinal);
    cmd.AddValue ("tSimulacion", "tiempo de cada simulacion", tSimulacion);
    cmd.Parse (argc,argv);
  
    NS_LOG_FUNCTION("nCsma" << nCsma);
    NS_LOG_FUNCTION("retardoProp" << retardoProp);
    NS_LOG_FUNCTION("capacidad" << capacidad);
    NS_LOG_FUNCTION("tamPaquete" << tamPaquete);
    NS_LOG_FUNCTION("intervalo" << intervalo);
    NS_LOG_FUNCTION("reintentosInicial" << reintentosInicial);
    NS_LOG_FUNCTION("reintentosFinal" << reintentosFinal);

    Gnuplot plot[3];
    plot[0].SetTitle ("Evolución del número medio de intentos");
    plot[0].SetLegend ("maxReint", "numIntentos");
    plot[1].SetTitle ("Evolución del tiempo medio de eco");
    plot[1].SetLegend ("maxReint", "tEco (us)");
    plot[2].SetTitle ("Evolución del porcentaje de error");
    plot[2].SetLegend ("maxReint", "perror (%)");

    Gnuplot2dDataset dataset[3];
    for (int i = 0; i < 3 ; i++) {
        dataset[i].SetStyle (Gnuplot2dDataset::LINES_POINTS);
        dataset[i].SetErrorBars(Gnuplot2dDataset::Y);
        dataset[i].SetTitle("");
    }
    
    for (uint32_t maxReintentos = reintentosInicial; maxReintentos <= reintentosFinal; maxReintentos++) {
        Average<double> numIntentosTotales;
        Average<double> tiempoEcoTotal;
        Average<double> porcentajeErrorClientes;
        for (int i = 0; i < 10; i++) {
            DATOS datos;
            //Modificamos la semilla de la simulacion
            SeedManager::SetRun (seed++);
            datos = simulacion(nCsma, retardoProp, capacidad, tamPaquete, intervalo, maxReintentos, tSimulacion);
            numIntentosTotales.Update (datos.mediaNumIntentosTotales);
            tiempoEcoTotal.Update (datos.mediaTiempoEcoTotal);
            porcentajeErrorClientes.Update (datos.mediaPorcentajeErrorClientes);
        }
        double z[3];
        z[0] = T_STUDENT_VALUE * std::sqrt (numIntentosTotales.Var() / 10);
        dataset[0].Add(maxReintentos, numIntentosTotales.Mean(), 2 * z[0]);
        z[1] = T_STUDENT_VALUE * std::sqrt (tiempoEcoTotal.Var() / 10);
        dataset[1].Add(maxReintentos, tiempoEcoTotal.Mean(), 2 * z[1]);
        z[2] = T_STUDENT_VALUE * std::sqrt (porcentajeErrorClientes.Var() / 10);
        dataset[2].Add(maxReintentos, porcentajeErrorClientes.Mean(), 2 * z[2]);
        NS_LOG_DEBUG ("MAX REINTENTOS: " << maxReintentos); 
        NS_LOG_DEBUG (numIntentosTotales.Mean() - z[0] << " < numIntentosTotales < " << numIntentosTotales.Mean() + z[0]);
        NS_LOG_DEBUG (tiempoEcoTotal.Mean() - z[1] << " < tiempoEcoTotal < " << tiempoEcoTotal.Mean() + z[1]);
        NS_LOG_DEBUG (porcentajeErrorClientes.Mean() - z[2] << " < porcentajeErrorClientes < " << porcentajeErrorClientes.Mean() + z[2]);
        NS_LOG_DEBUG ("");
    }

        plot[0].AddDataset (dataset[0]);
        std::ofstream plotFile1 ("practica05-01.plt");
        plot[0].GenerateOutput (plotFile1);
        plotFile1 << "pause -1" << std::endl;
        plotFile1.close ();
       
        
        plot[1].AddDataset (dataset[1]);
        std::ofstream plotFile2 ("practica05-02.plt");
        plot[1].GenerateOutput (plotFile2);
        plotFile2 << "pause -1" << std::endl;
        plotFile2.close ();
 
        
        plot[2].AddDataset (dataset[2]);
        std::ofstream plotFile3 ("practica05-03.plt");
        plot[2].GenerateOutput (plotFile3);
        plotFile3 << "pause -1" << std::endl;
        plotFile3.close ();
   
    return 0;
}
