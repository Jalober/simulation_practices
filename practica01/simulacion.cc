#include <ns3/simulator.h>
#include <ns3/random-variable-stream.h>
#include <ns3/core-module.h>
#include <ns3/config.h>
#include <ns3/average.h>

#define MAX 100

using namespace ns3;

int main (int argc, char *argv[])
{
  // Definición de variables
  int numRepeticiones;
  int i;
  Ptr<UniformRandomVariable> var;
  Ptr<UniformRandomVariable> var2;
  Average<double> av;
  Average<double> av2;

  double min = 4.0;
  double max = 10.0;
  double value = 0.0;

  numRepeticiones = MAX;
  var = CreateObject<UniformRandomVariable> ();
  var2 = CreateObject<UniformRandomVariable> ();
  //var->SetAttribute ("Min", DoubleValue (min));

  var->SetAttribute ("Max", DoubleValue (max));
  for ( i = 0; i < numRepeticiones; i++ ) {
    value = var->GetValue();
    //std::cout << value << std::endl;
    av.Update (value);
  }
  var->SetAttribute ("Min", DoubleValue (min));
  
  std::cout << "-----------------" <<  std::endl;

  for ( i = 0; i < numRepeticiones; i++) {
     //std::cout << var->GetValue () << std::endl;
  }

  std::cout << "-----------------" <<  std::endl;

  for ( i = 0; i < numRepeticiones; i++) {
     value = var->GetValue();
     //std::cout << value << std::endl;
     av2.Update(value);
  }
  std::cout << "-----------------" <<  std::endl;

  std::cout << "av = " << av.Mean() << std::endl;
  std::cout << "av2 = " << av2.Mean() << std::endl;  

}

