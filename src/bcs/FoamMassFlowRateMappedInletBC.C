#include "FoamMassFlowRateMappedInletBC.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "PstreamReduceOps.H"
#include "Registry.h"

#include "ops.H"
#include "vectorField.H"
#include "volFieldsFwd.H"

registerMooseObject("hippoApp", FoamMassFlowRateMappedInletBC);

InputParameters
FoamMassFlowRateMappedInletBC::validParams()
{
  auto params = FoamMappedInletBCBase::validParams();
  MooseEnum scaleEnum("SCALE NONE", "SCALE");
  params.addParam<MooseEnum>("scale_method",
                             scaleEnum,
                             "Method used to maintain inlet bulk variable. "
                             "SCALE means the variable is multiplied by a factor, "
                             "NONE means the variable is not scaled.");

  params.remove("foam_variable");
  params.addPrivateParam<std::string>("foam_variable", "U");

  return params;
}

FoamMassFlowRateMappedInletBC::FoamMassFlowRateMappedInletBC(const InputParameters & params)
  : FoamMappedInletBCBase(params), _scale_method(params.get<MooseEnum>("scale_method"))
{
}

void
FoamMassFlowRateMappedInletBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & boundary_patch = foam_mesh.boundary()[_boundary[0]];

  // currently we map mass flux rather than velocity, maybe useful to have option
  auto && U_map = getMappedArray<Foam::vector>("U");
  auto && rho_map = getMappedArray<Foam::scalar>("rho");
  auto g_map = rho_map * U_map;

  auto & rho = boundary_patch.lookupPatchField<Foam::volScalarField, double>("rho");
  auto & U_var = const_cast<Foam::fvPatchField<Foam::vector> &>(
      boundary_patch.lookupPatchField<Foam::volVectorField, double>("U"));

  Foam::vectorField g_var(U_var.size());
  g_var = rho_map * U_map;
  if (_scale_method == "SCALE")
  {
    auto & Sf = boundary_patch.Sf();

    auto m_dot = Foam::returnReduce(Foam::sum(g_map & -Sf), Foam::sumOp<Real>());
    g_var *= _pp_value / m_dot;
  }

  U_var == g_var / rho;
}
