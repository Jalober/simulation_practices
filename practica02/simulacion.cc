#include <ns3/core-module.h>

#include "llegadas.h"

#define MEDIA 10

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Simulacion");

int main (int argc, char *argv[])
{
  // Definimos el modelo a simular
  Llegadas modelo (MEDIA);

  // Programo la finalización de la simulación.
  Simulator::Stop(Seconds(10000));

  // Lanzamos la simulación.
  Simulator::Run();

  // Presentamos la media muestral
  NS_LOG_INFO ("Media: " << modelo.Media ());

  // Finalizamos la simulación
  Simulator::Destroy ();
}
