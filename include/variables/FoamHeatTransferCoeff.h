#pragma once

#include "FEProblemBase.h"
#include "FoamFieldBase.h"
#include "InputParameters.h"
#include "FoamMesh.h"
#include "MooseObject.h"

#include "MooseTypes.h"
#include "fvCFD_moose.h"
#include <fvPatchFieldsFwd.H>
#include <scalarField.H>

class FoamHeatTransferCoeff : public FoamFieldBase
{
public:
  static InputParameters validParams();

  explicit FoamHeatTransferCoeff(const InputParameters & params);

  virtual void transferVariable();

  std::string foamVariable() const { return ""; };

protected:
  Foam::scalarField calculateHTC();
  const Foam::scalarField calculate_qw(const Foam::fvPatchScalarField & Tbf);
  // Pointer to the FoamMesh object
  FoamMesh & _mesh;
  const SubdomainID _subdomain;
  const UserObjectName & _t_bulk_uo_name;
};
