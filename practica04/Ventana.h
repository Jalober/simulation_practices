/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include <iostream>

class Ventana
{
public:
  Ventana (uint8_t tamanio, uint32_t rango) :
    m_tam (tamanio),
    m_mod (rango),
    // Inicialmente la ventana está en el extremo izquierdo del rango
    m_izq (0),
    // Inicialmente la ventana está vacía
    m_der (m_izq)
  {}

  // Devuelve el crédito disponible
  uint32_t Credito   ()               { return m_tam - Offset (m_der);}
  // Indica si el valor está dentro de la parte llena de la ventana
  bool     EnVentana (uint32_t valor)
  {
    return (valor != m_izq) && (Offset (m_der) >= Offset (valor));
  }
  // Vacía la ventana.
  void     Vacia     ()               { m_der = m_izq; }
  // Desplaza la ventana hacial la derecha
  void     Asentida  (uint32_t valor) { m_izq = valor % m_mod; }
  // Obtiene el siguiente número de secuencia y lo incrementa
  uint32_t Pendiente ()
  {
    uint32_t aux = m_der;
    m_der = (m_der + 1) % m_mod;
    return aux;
  }
  friend std::ostream& operator<< (std::ostream& os, const Ventana& vt)
  {
    os << "(" << (int) vt.m_izq << ", " << (int) vt.m_der << ")";
    return os;
  }
private:
  // Calcula valor referido al extremo izquierdo de la ventana
  uint32_t Offset    (uint32_t valor)  { return (valor + m_mod - m_izq) % m_mod; }

  // Tamaño de la ventana
  uint8_t  m_tam;
  // Rango de numeración de la secuencia
  uint32_t m_mod;
  // Primer valor no asentido
  uint32_t m_izq;
  // Siguiente valor a transmitir
  uint32_t m_der;  
};
