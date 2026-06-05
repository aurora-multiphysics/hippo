#pragma once

#include "FEProblemBase.h"
#include "FoamFieldBase.h"
#include "InputParameters.h"
#include "FoamMesh.h"
#include "MooseObject.h"

#include "MooseTypes.h"
#include "fvCFD_moose.h"

class FoamHeatTransferCoeff : public FoamFieldBase
{
public:
  static InputParameters validParams();

  explicit FoamHeatTransferCoeff(const InputParameters & params);

  virtual void transferVariable();

  std::string foamVariable() const { return ""; };

protected:
  void calculateHTC();
  const Foam::Field<Foam::scalar> calculate_qw();
  // Pointer to the FoamMesh object
  FoamMesh & _mesh;
  const SubdomainID _subdomain;
  Foam::Field<Foam::scalar> _field;
  const UserObjectName & _t_bulk_uo_name;
};
