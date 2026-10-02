#include "FoamScalarBulkMappedInletBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "Registry.h"

#include "PstreamReduceOps.H"
#include "ops.H"
#include "volFieldsFwd.H"
#include <label.H>
#include <scalar.H>

#include <cmath>

registerMooseObject("hippoApp", FoamScalarBulkMappedInletBC);

InputParameters
FoamScalarBulkMappedInletBC::validParams()
{
  auto params = FoamMappedInletBCBase::validParams();
  MooseEnum scaleEnum("SCALE SUBTRACT NONE", "SCALE");
  params.addParam<MooseEnum>("scale_method",
                             scaleEnum,
                             "Method used to maintain inlet bulk variable. "
                             "SCALE means the variable is multiplied by a factor, "
                             "SUBTRACT means the variable is reduced by constant,"
                             "NONE means the variable is not scaled.");

  return params;
}

FoamScalarBulkMappedInletBC::FoamScalarBulkMappedInletBC(const InputParameters & params)
  : FoamMappedInletBCBase(params), _scale_method(params.get<MooseEnum>("scale_method"))
{
}

void
FoamScalarBulkMappedInletBC::imposeBoundaryCondition()
{
  auto & boundary_patch = getFvMesh().boundary()[_boundary[0]];

  auto && var_map = getMappedArray<Foam::scalar>(_foam_variable);
  auto & Sf = boundary_patch.magSf();

  auto totalArea = Foam::sum(Sf);
  Foam::reduce(totalArea, Foam::sumOp<Real>());

  auto var_bulk = Foam::sum(var_map * Sf) / totalArea;

  Foam::reduce(var_bulk, Foam::sumOp<Real>());

  auto & var = const_cast<Foam::fvPatchField<Foam::scalar> &>(
      boundary_patch.lookupPatchField<Foam::volScalarField, double>(_foam_variable));

  var == applyScaleMethod(var_map, _pp_value, var_bulk);
}

template <typename T>
T
FoamScalarBulkMappedInletBC::applyScaleMethod(T & var, const Real bulk_ref, const Real bulk)
{
  if (_scale_method == "SCALE")
  {
    if (std::abs(bulk) < Foam::SMALL)
      mooseError("Cannot scale mapped inlet field '", _foam_variable, "', bulk value is 0.");

    return (var * bulk_ref / bulk)();
  }
  else if (_scale_method == "SUBTRACT")
  {
    return (var + bulk_ref - bulk)();
  }
  else if (_scale_method == "NONE")
  {
    return var;
  }
  else
  {
    mooseError("Invalid scale method '", _scale_method, "'.");
  }
}
