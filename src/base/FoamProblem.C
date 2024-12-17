#include "FoamProblem.h"
#include "FoamInterface.h"
#include "FoamMesh.h"

#include <AuxiliarySystem.h>
#include <MooseError.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <finiteVolume/solver/solver.H>
#include <libmesh/enum_order.h>
#include <libmesh/fe_type.h>

registerMooseObject("hippoApp", FoamProblem);

namespace
{
constexpr auto PARAM_VAR_FOAM_HF = "foam_heat_flux";
constexpr auto PARAM_VAR_FOAM_T = "foam_temp";
constexpr auto PARAM_VAR_T = "temp";
constexpr auto PARAM_VAR_HF = "heat_flux";

bool
is_constant_monomial(const MooseVariableFieldBase & var)
{
  return var.order() == libMesh::Order::CONSTANT && var.feType().family == FEFamily::MONOMIAL;
}

Real
variableValueAtElement(const libMesh::Elem * element, MooseVariableFieldBase * variable)
{
  auto & sys = variable->sys();
  auto dof = element->dof_number(sys.number(), variable->number(), 0);
  return sys.solution()(dof);
}
} // namespace

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();

  // Parameters to set variables to read from/write to.
  // Note that these can be omitted or point to the same variable to save memory.
  params.addParam<std::string>(
      PARAM_VAR_FOAM_HF, "The name of the aux variable to write the OpenFOAM wall heat flux into.");
  params.addParam<std::string>(
      PARAM_VAR_FOAM_T,
      "The name of the aux variable to write the OpenFOAM boundary temperature into.");
  params.addParam<std::string>(
      PARAM_VAR_HF, "The name of the aux variable to set the OpenFOAM wall heat flux from.");
  params.addParam<std::string>(
      PARAM_VAR_T, "The name of the aux variable to set the OpenFOAM boundary temperature from.");
  return params;
}

FoamProblem::FoamProblem(InputParameters const & params)
  : ExternalProblem(params),
    _foam_mesh(dynamic_cast<FoamMesh *>(&this->ExternalProblem::mesh())),
    _interface(_foam_mesh->getFoamInterface()),
    _solver(Foam::solver::New("fluid", _interface->getMesh()).ptr())
{
  assert(_foam_mesh);
  assert(_interface);

  auto t_var_name = params.get<std::string>(PARAM_VAR_T);
  auto hf_var_name = params.get<std::string>(PARAM_VAR_HF);
  if (t_var_name.empty() && hf_var_name.empty())
  {
    mooseWarning("Neither parameters '",
                 PARAM_VAR_T,
                 "' or '",
                 PARAM_VAR_HF,
                 "' are set. No quantities are being transferred to OpenFOAM.");
  }
  else if (t_var_name == hf_var_name)
  {
    mooseError("Parameters '",
               PARAM_VAR_T,
               "' and '",
               PARAM_VAR_HF,
               "' cannot refer to the same variable: '",
               t_var_name,
               "'.");
  }

  auto foam_t_var_name = params.get<std::string>(PARAM_VAR_FOAM_T);
  auto foam_hf_var_name = params.get<std::string>(PARAM_VAR_FOAM_HF);
  if (foam_t_var_name.empty() && foam_hf_var_name.empty())
  {
    mooseWarning("Neither parameters '",
                 PARAM_VAR_FOAM_T,
                 "', or '",
                 PARAM_VAR_FOAM_HF,
                 "' are set. No quantities are being copied from OpenFOAM.");
  }
  else if (foam_t_var_name == foam_hf_var_name)
  {
    mooseError("Parameters '",
               PARAM_VAR_FOAM_T,
               "' and '",
               PARAM_VAR_FOAM_HF,
               "' cannot refer to the same variable: '",
               foam_t_var_name,
               "'.");
  }
}

void
FoamProblem::externalSolve()
{
  _solver.run();
}
