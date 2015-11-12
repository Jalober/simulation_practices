/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

using namespace ns3;

#include "ns3/header.h"

class CabEnlace : public Header
{
public:
  virtual TypeId   GetInstanceTypeId (void) const           { return GetTypeId (); }

  virtual void     Print (std::ostream &os) const
  {
    os << "Tipo = " << m_tipo << "; secuencia = " << m_secuencia;
  }

  virtual uint32_t GetSerializedSize (void) const           { return 2; }

  virtual void     Serialize (Buffer::Iterator contenedor) const
  {
    contenedor.WriteU8 (m_tipo);
    contenedor.WriteU8 (m_secuencia);
  }

  virtual uint32_t Deserialize (Buffer::Iterator contenedor)
  {
    m_tipo      = contenedor.ReadU8 ();
    m_secuencia = contenedor.ReadU8 ();
    return 1;
  }

  void     SetData (uint8_t tipo, uint8_t secuencia)
  {
    m_tipo = tipo;
    m_secuencia = secuencia;
  }

  uint8_t  GetTipo (void) const { return m_tipo; }
  uint8_t  GetSecuencia (void) const { return m_secuencia; }


private:
  uint8_t m_tipo;
  uint8_t m_secuencia;
};
