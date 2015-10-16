#include <ns3/core-module.h>
#include <ns3/gnuplot.h>
#include "llegadas.h"


#define TASA_MIN 10
#define TASA_MAX 50
#define NUM_SIMULATIONS 10
#define T_STUDENT_VALUE 1.8331

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Simulacion");

int main (int argc, char *argv[])
{
 
  double z; //Error del intervalo de confianza
  uint32_t seed = 1; //Semilla a utilizar
 
  //Variables para atrametros de linea de comandos
  double tasaMedia(10.0); //media inicial
  double paso(10.0); //paso entre medias
  uint32_t valores(5); //numero de valores
  
  //Obtencion de parametros por linea de comandos
  CommandLine cmd;
  cmd.AddValue("tasaMedia", "media inicial", tasaMedia);
  cmd.AddValue("paso", "paso entre medias", paso);
  cmd.AddValue("valores", "numero de valores", valores);
  cmd.Parse(argc, argv);

  //Generacion de grafica
  Gnuplot plot;
  plot.SetTitle("Media en funcion de tasa");
  
  Gnuplot2dDataset dataset;
  dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);
  
  int tasa = tasaMedia;
  
  for (uint32_t j = 0; j < valores; j++) {
    
    Average<double> medias; //Acumulamos las medias 
    for (uint32_t i = 0; i < NUM_SIMULATIONS; i++) {
  
      //Modificamos la semilla de la simulacion
      SeedManager::SetRun(seed++);
  
      // Definimos el modelo a simular
      Llegadas modelo (tasa);
  
      // Programamos la finalización de la simulación.
      Simulator::Stop(Seconds(10000));
  
      // Lanzamos la simulación.
      Simulator::Run();
  
      // Presentamos la media muestral
      NS_LOG_FUNCTION ("Media: " << modelo.Media ());

      //Añadimos el punto en la grafica
      dataset.Add(tasa, modelo.Media());

      // Acumulamos la media obtenida
      medias.Update(modelo.Media());    
  
      // Finalizamos la simulación
      Simulator::Destroy ();
    }  
    z = T_STUDENT_VALUE * std::sqrt( medias.Var() / (NUM_SIMULATIONS-1));
  
    NS_LOG_INFO("Intervalo Confianza para tasa = " << tasa << ": " << medias.Mean()-z << " < Media < " << medias.Mean() + z); 
  
    tasa = tasa + paso;
  }
  plot.AddDataset (dataset);
  std::ofstream plotFile ("practica02.plt");
  plot.GenerateOutput (plotFile);
  plotFile << "pause -1" << std::endl;
  plotFile.close ();

}
