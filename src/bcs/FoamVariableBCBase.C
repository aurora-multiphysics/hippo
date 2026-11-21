
#include "FoamBCBase.h"
#include "FoamVariableBCBase.h"
#include "hippoUtils.h"
#include "FoamProblem.h"
#include "hippoDiffusionFluxFvPatchScalarField.h"
#include "fixedGradientFvPatchFields.H"

#include "FEProblemBase.h"

namespace
{
// Private function to check if variables are constant monomials
inline bool
is_constant_monomial(const MooseVariableFieldBase & var)
{
  return var.order() == libMesh::Order::CONSTANT && var.feType().family == FEFamily::MONOMIAL;
}
}

InputParameters
FoamVariableBCBase::validParams()
{
  InputParameters params = FoamBCBase::validParams();

  params.addParam<bool>("direct",
                        true,
                        "Receive transfers directly into registered patch storage. "
                        "Set false to supply values through a MOOSE AuxVariable.");
  params.addParam<VariableName>(
      "v",
      "When direct = false, use an existing AuxVariable in the BC. "
      "If v is omitted in that mode, an AuxVariable with this object's name is created.");
  // Get desired parameters from Variable objects
  params.transferParam<std::vector<Real>>(MooseVariable::validParams(), "initial_condition");

  return params;
}

FoamVariableBCBase::FoamVariableBCBase(const InputParameters & params, const FoamBCType bc_type)
  : FoamBCBase(params, bc_type), _direct(getParam<bool>("direct")), _bc_type(bc_type), _moose_var()
{
  if (_direct && isParamValid("v"))
    paramError("v", "Cannot supply v when direct = true.");
}

void
FoamVariableBCBase::initialSetup()
{
  if (_direct)
    return;

  // Check variable exists
  auto var_name = parameters().isParamValid("v") ? parameters().get<VariableName>("v") : _name;
  if (!_c_fe_problem.hasVariable(var_name))
    mooseError("Variable '", var_name, "' doesn't exist");

  THREAD_ID tid = parameters().get<THREAD_ID>("_tid");
  _moose_var = _c_fe_problem.getVariable(tid, var_name);

  // Check variable is constant monomial in case it is provided.
  if (!is_constant_monomial(_moose_var->get()))
    mooseError("Variable '", var_name, "' must be a constant monomial.");
}

BCInfoTableRow
FoamVariableBCBase::getInfoRow() const
{
  // List info about BC
  return std::make_tuple(name(),
                         type(),
                         foamVariable(),
                         _direct ? "None (direct)" : _moose_var->get().name(),
                         Hippo::internal::listFromVector(boundary()),
                         _patch_replaced ? "yes" : "no");
}

Real
FoamVariableBCBase::variableValueAtElement(const libMesh::Elem & elem) const
{
  auto & sys = _moose_var->get().sys();
  auto dof = elem.dof_number(sys.number(), _moose_var->get().number(), 0);
  return sys.solution()(dof);
}

std::vector<Real>
FoamVariableBCBase::getMooseVariableArray(int subdomainId) const
{
  size_t patch_count = getFoamMesh().getPatchCount(subdomainId);
  size_t patch_offset = getFoamMesh().getPatchOffset(subdomainId);

  std::vector<Real> var_array(patch_count);
  for (size_t j = 0; j < patch_count; ++j)
  {
    auto elem = patch_offset + j;
    const auto elem_ptr = getFoamMesh().getElemPtr(elem + getFoamMesh().rank_element_offset);
    assert(elem_ptr);
    var_array[j] = variableValueAtElement(*elem_ptr);
  }

  return var_array;
}

void
FoamVariableBCBase::registerFieldAccessor()
{
  FoamFieldAccessor<Foam::scalar>::FieldRefMap fields;
  auto & field = getFvMesh().lookupObjectRef<Foam::volScalarField>(_foam_variable);
  for (const auto & boundary : _boundary)
  {
    // Loop through boundaries and create reference for each one
    const auto id = getFvMesh().boundary().findIndex(boundary);
    auto & patch = field.boundaryFieldRef()[id];
    if (_bc_type == FoamBCType::fixedValue)
      fields.emplace(boundary, std::ref(static_cast<Foam::scalarField &>(patch)));
    else if (_bc_type == FoamBCType::fixedGradient)
      fields.emplace(
          boundary,
          std::ref(Foam::refCast<Foam::fixedGradientFvPatchScalarField>(patch).gradient()));
    else
      fields.emplace(
          boundary,
          std::ref(Foam::refCast<Foam::hippoDiffusionFluxFvPatchScalarField>(patch).flux()));
  }

  if (_direct && isParamSetByUser("initial_condition"))
  {
    const auto & initial = getParam<std::vector<Real>>("initial_condition");
    if (initial.size() != 1)
      paramError("initial_condition", "Expected one scalar value.");
    for (auto & entry : fields)
      entry.second.get() = initial[0];
  }
  getFoamProblem().registerField<Foam::scalar>(
      "FoamBCs/" + name(), std::make_unique<FoamFieldAccessor<Foam::scalar>>(fields));
}

void
FoamVariableBCBase::imposeBoundaryCondition()
{
  if (_direct)
    return;

  // If mirror transfer is used copy values from MOOSE variable data and copy to
  // Foam field via the accessor
  auto & accessor = getFoamProblem().getFieldAccessor<Foam::scalar>("FoamBCs/" + name());
  for (const auto & boundary : _boundary)
  {
    const auto id = getFvMesh().boundary().findIndex(boundary);
    const auto values = getMooseVariableArray(id);
    auto & destination = accessor.boundaryField(boundary);
    if (values.size() != static_cast<std::size_t>(destination.size()))
      mooseError("MOOSE and OpenFOAM patch sizes differ for '", boundary, "'.");
    std::copy(values.begin(), values.end(), destination.begin());
  }
}
