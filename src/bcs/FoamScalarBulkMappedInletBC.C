#include "FoamScalarBulkMappedInletBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PstreamReduceOps.H"
#include "Registry.h"

#include "ops.H"
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamScalarBulkMappedInletBC);

InputParameters
FoamScalarBulkMappedInletBC::validParams()
{
  auto params = FoamMappedInletBCBase::validParams();
  MooseEnum scaleEnum("SCALE SUBTRACT", "SCALE");
  params.addParam<MooseEnum>("scale_method",
                             scaleEnum,
                             "Method used to maintain inlet bulk variable. "
                             "SCALE means the variable is multiplied by a factor, "
                             "SUBTRACT means the variable is reduced by constant.");

  return params;
}

FoamScalarBulkMappedInletBC::FoamScalarBulkMappedInletBC(const InputParameters & params)
  : FoamMappedInletBCBase(params), _scale_method(params.get<MooseEnum>("scale_method"))
{
}

void
FoamScalarBulkMappedInletBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & boundary_patch = foam_mesh.boundary()[_boundary[0]];

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
    return (var * bulk_ref / bulk)();
  }
  else if (_scale_method == "SUBTRACT")
  {
    return (var + bulk_ref - bulk)();
  }
  else
  {
    mooseError("Invalid scale method '", _scale_method, "'.");
  }
}
