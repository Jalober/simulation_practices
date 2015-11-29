/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include "Observador.h"

#define STARTTIME 2.0
#define STOPTIME  10.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("practica05");

void simulacion (NetDeviceContainer * csmaDevices, uint32_t nCsma, uint32_t numReintentos) {
    Observador observadores[nCsma]; 
    for (uint32_t i = 0; i < nCsma; i++) {
        csmaDevices->Get(i)->TraceConnectWithoutContext ("PhyTxEnd",     MakeCallback(&Observador::PaqueteEnviado,              &observadores[i]));
        csmaDevices->Get(i)->TraceConnectWithoutContext ("PhyTxDrop",    MakeCallback(&Observador::PaquetePerdido,              &observadores[i]));
        csmaDevices->Get(i)->TraceConnectWithoutContext ("MacTxBackoff", MakeCallback(&Observador::PaqueteEnBackoff,            &observadores[i]));
        csmaDevices->Get(i)->TraceConnectWithoutContext ("MacTx",        MakeCallback(&Observador::PaqueteParaEnviar,           &observadores[i]));
        csmaDevices->Get(i)->TraceConnectWithoutContext ("MacRx",        MakeCallback(&Observador::PaqueteRecibidoParaEntregar, &observadores[i])); 
    
        Ptr<CsmaNetDevice> csma_device = csmaDevices.Get(i)->GetObject<CsmaNetDevice>();
        csma_device->SetBackoffParams (Time ("1us"), 10, 1000, 10, numReintentos);
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
 
    NS_LOG_DEBUG ("--------------------------------------------------------");
    NS_LOG_DEBUG ("Media de intentos de transmision en el escenario: " << mediaNumIntentosTotales);
    NS_LOG_DEBUG ("Tiempo de eco medio en el escenario: " << Time(mediaTiempoEcoTotal));
    NS_LOG_DEBUG ("Porcentaje de paquetes perdidos por los clientes: " << mediaPorcentajeErrorClientes << " %");
    NS_LOG_DEBUG ("Porcentaje de paquetes perdidos en el escenario: " << mediaPorcentajeErrorEscenario << " %");
    NS_LOG_DEBUG ("--------------------------------------------------------");
    
    
    //Desconectamos las trazas para reutilizar el escenario
    for (uint32_t i = 0; i < nCsma; i++) {
        csmaDevices->Get(i)->TraceDisconnectWithoutContext ("PhyTxEnd",     MakeCallback(&Observador::PaqueteEnviado,              &observadores[i]));
        csmaDevices->Get(i)->TraceDisconnectWithoutContext ("PhyTxDrop",    MakeCallback(&Observador::PaquetePerdido,              &observadores[i]));
        csmaDevices->Get(i)->TraceDisconnectWithoutContext ("MacTxBackoff", MakeCallback(&Observador::PaqueteEnBackoff,            &observadores[i]));
        csmaDevices->Get(i)->TraceDisconnectWithoutContext ("MacTx",        MakeCallback(&Observador::PaqueteParaEnviar,           &observadores[i]));
        csmaDevices->Get(i)->TraceDisconnectWithoutContext ("MacRx",        MakeCallback(&Observador::PaqueteRecibidoParaEntregar, &observadores[i])); 
    }

}

int
main (int argc, char *argv[])
{
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
    Time::SetResolution (Time::US);

    int dni[8] = {3,0,2,2,8,4,1,3};
    NS_LOG_INFO ("DNI: " << dni[0]<<dni[1]<<dni[2]<<dni[3]<<dni[4]<<dni[5]<<dni[6]<<dni[7]);

    //Parametros de simulacion
    uint32_t nCsma       = 10;
    Time     retardoProp = Time ("6560ns");
    DataRate capacidad   = DataRate ("100Mbps");
    uint32_t tamPaquete  = 1024;
    Time     intervalo   = Time ("1s");

    CommandLine cmd;
    cmd.AddValue ("nCsma", "Número de nodos de la red local", nCsma);
    cmd.AddValue ("retardoProp", "retardo de propagación en el bus", retardoProp);
    cmd.AddValue ("capacidad", "capacidad del bus", capacidad);
    cmd.AddValue ("tamPaquete", "tamaño de las SDU de aplicación", tamPaquete);
    cmd.AddValue ("intervalo", "tiempo entre dos paquetes consecutivos enviados por el mismo cliente", intervalo);
    cmd.Parse (argc,argv);

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
    serverApp.Stop (Seconds (STOPTIME));
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
    clientApps.Stop (Seconds (STOPTIME));

    // Cálculo de rutas
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    csma.EnablePcap ("practica05", csmaDevices.Get (nCsma - 1), true);
    
    for (int i = 0; i < 2; i++) {
        simulacion(&csmaDevices, nCsma, 8);
    }

    return 0;
}
