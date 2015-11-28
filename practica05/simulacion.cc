/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/object.h"
#include "ns3/global-value.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"

#include "Observador.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("practica05");

int
main (int argc, char *argv[])
{
    GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));
    Time::SetResolution (Time::US);

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
    serverApp.Start (Seconds (1.0));
    serverApp.Stop (Seconds (10.0));
    // Clientes
    UdpEchoClientHelper echoClient (csmaInterfaces.GetAddress (nCsma - 1), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (10000));
    echoClient.SetAttribute ("Interval", TimeValue (intervalo));
    echoClient.SetAttribute ("PacketSize", UintegerValue (tamPaquete));
    NodeContainer clientes;

    for (uint32_t i = 0; i < nCsma - 1; i++)
    {
    	clientes.Add (csmaNodes.Get (i));
        //Añadimos un observador por cliente
    }
    NS_LOG_FUNCTION ("Fin bucle de creacion de observadores");

    ApplicationContainer clientApps = echoClient.Install (clientes);
    NS_LOG_FUNCTION ("Instalacion de clientes de echo realizada");
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));

    // Cálculo de rutas
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    csma.EnablePcap ("practica05", csmaDevices.Get (nCsma - 1), true);
    
    Observador observadores[nCsma]; 
    for (uint32_t i = 0; i < nCsma -1; i++) {
        csmaDevices.Get(i)->TraceConnectWithoutContext ("PhyTxEnd", MakeCallback(&Observador::PaqueteEnviado, &observadores[i]));
        csmaDevices.Get(i)->TraceConnectWithoutContext ("MacTxDrop", MakeCallback(&Observador::PaquetePerdido, &observadores[i]));
        csmaDevices.Get(i)->TraceConnectWithoutContext ("MacTxBackoff", MakeCallback(&Observador::PaqueteEnBackoff, &observadores[i]));
        csmaDevices.Get(i)->TraceConnectWithoutContext ("MacTx", MakeCallback(&Observador::PaqueteParaEnviar, &observadores[i]));
        csmaDevices.Get(i)->TraceConnectWithoutContext ("MacRx", MakeCallback(&Observador::PaqueteRecibidoParaEntregar, &observadores[i])); 
    
        Ptr<CsmaNetDevice> csma_device =  csmaDevices.Get (i)->GetObject<CsmaNetDevice> ();
        csma_device -> SetBackoffParams (Time ("1us"), 10, 1000, 5, 8);

    }
 
    NS_LOG_FUNCTION ("Va a comenzar la simulacion");
    Simulator::Run ();
    Simulator::Destroy ();
    NS_LOG_FUNCTION ("Finalizada simulacion");

    for (uint32_t i = 0; i < nCsma - 1; i++) {
        NS_LOG_INFO ("Media intentos observador " << i << ": " << observadores[i].GetMediaNumIntentos());
        NS_LOG_INFO ("Tiempo medio observador " << i << ": " << Time(observadores[i].GetMediaTiempoEco ())); 
   
        
    }

    return 0;
}
