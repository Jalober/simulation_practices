#include <ns3/core-module.h>

#include "llegadas.h"

using namespace ns3;


NS_LOG_COMPONENT_DEFINE ("Llegadas");


Llegadas::Llegadas (double tasa)
{
  double mean = 1/tasa;  
  // Creamos la variable aleatoria ...
  tEntreLlegadas = CreateObject<ExponentialRandomVariable> ();
  // ... y ajustamos su valor medio
  tEntreLlegadas->SetAttribute ("Mean", DoubleValue (mean));  
  // Programamos la primera llegada.
  Simulator::Schedule (Seconds(tEntreLlegadas->GetValue()), &Llegadas::NuevaLlegada, this, Simulator::Now ().GetSeconds());
}

void
Llegadas::NuevaLlegada (double tiempo)
{
  // Calculamos el tiempo transcurrido desde la última llegada
  double t = Simulator::Now ().GetSeconds() - tiempo;
  // Presentamos el tiempo transcurrido entre llegadas.
  NS_LOG_INFO ("Tiempo transcurrido entre llegadas: " << t);
  // Programamos la siguiente llegada
  Simulator::Schedule (Seconds(tEntreLlegadas->GetValue()), &Llegadas::NuevaLlegada, this, Simulator::Now ().GetSeconds());
  // Acumulamos el valor del intervalo
  acumulador.Update(t);
}


double
Llegadas::Media ()
{
  // Devolvemos el valor medio de las muestras recogidas.
  return acumulador.Mean();
}
