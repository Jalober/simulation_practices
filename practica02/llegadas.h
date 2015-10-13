#include <ns3/random-variable-stream.h>
#include <ns3/average.h>


using namespace ns3;


////////////////////////////////////////////////////////////////////////////////////////
//
// Modelo a simular: patrón de llegadas según Poisson de tasa determinada
//
class Llegadas
{
 public:
  // Constructor: el parámetro es la tasa de llegadas por unidad de tiempo
  Llegadas(double tasa);
  // Media del proceso
  double Media ();
  
 private:
  // Método que registra una nueva llegada y planifica la siguiente
  void NuevaLlegada (double tiempo);

  // Variable aleatoria para generar los intervalos entre llegadas
  Ptr<ExponentialRandomVariable> tEntreLlegadas;
  // Acumulador de los diferentes tiempos de llegada
  Average<double> acumulador;
};
