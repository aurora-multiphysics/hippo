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

  params.remove("foam_variable");
  params.addPrivateParam<std::string>("foam_variable", "U");

  return params;
}

FoamMassFlowRateMappedInletBC::FoamMassFlowRateMappedInletBC(const InputParameters & params)
  : FoamMappedInletBCBase(params)
{
}

void
FoamMassFlowRateMappedInletBC::imposeBoundaryCondition()
{
  auto & foam_mesh = _mesh->fvMesh();
  auto & boundary_patch = foam_mesh.boundary()[_boundary[0]];

  // should we mapping rho U or just U? Fo now U but we can change it
  auto && U_map = getMappedArray<Foam::vector>("U");
  auto && rho_map = getMappedArray<Foam::scalar>("rho");
  auto g_map = rho_map * U_map;

  auto & rho = boundary_patch.lookupPatchField<Foam::volScalarField, double>("rho");
  auto & Sf = boundary_patch.Sf();

  auto m_dot = Foam::sum(g_map & -Sf);
  Foam::reduce(m_dot, Foam::sumOp<Real>());

  auto & U_var = const_cast<Foam::fvPatchField<Foam::vector> &>(
      boundary_patch.lookupPatchField<Foam::volVectorField, double>("U"));

  Foam::vectorField g_var(U_var.size());
  g_var = rho_map * U_map * _pp_value / m_dot;
  U_var == g_var / rho;
}
