#include "FoamFieldBase.h"
#include "FoamHeatTransferCoeff.h"
#include "FoamProblem.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "Registry.h"
#include "UserObject.h"
#include "hippoUtils.h"
#include <scalar.H>
#include <string>
#include <volFieldsFwd.H>

#include "fluidThermophysicalTransportModel.H"

registerMooseObject("hippoApp", FoamHeatTransferCoeff);

InputParameters
FoamHeatTransferCoeff::validParams()
{
  InputParameters params = FoamFieldBase::validParams();
  params.addRequiredParam<UserObjectName>("bulk_temperature_uo",
                                          "UserObject describing the bulk temperature");
  params.addRequiredParam<SubdomainName>("boundary", "Boundary used to calculate the HTC on.");
  params.addParam<std::string>("T_name", "T", "Temperature field name in OpenFOAM");
  return params;
}

FoamHeatTransferCoeff::FoamHeatTransferCoeff(const InputParameters & params)
  : FoamFieldBase(params),
    _mesh(getFoamProblem().mesh()),
    _subdomain(_mesh.getSubdomainID(getParam<SubdomainName>("boundary"))),
    _field(_mesh.fvMesh().boundary()[getParam<SubdomainName>("boundary")].size()),
    _t_bulk_uo_name(getParam<UserObjectName>("bulk_temperature_uo"))
{
}

void
FoamHeatTransferCoeff::transferVariable()
{
  THREAD_ID tid = getParam<THREAD_ID>("_tid");
  auto & moose_var = getFoamProblem().getVariable(tid, _name);

  calculateHTC();

  Hippo::internal::copyFieldFoamToMoose(_mesh, _field, moose_var, _subdomain);
  moose_var.sys().solution().close();
}

void
FoamHeatTransferCoeff::calculateHTC()
{
  const std::string & subdomain{getParam<SubdomainName>("boundary")};
  const std::string & Tname{getParam<std::string>("T_name")};
  Foam::fvMesh & foam_mesh{_mesh.fvMesh()};

  const auto & Tbf =
      foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(Tname);
  const Foam::scalar eps = Foam::ROOTVSMALL;

  // get reference T field

  const Foam::vectorField & cellCenters{foam_mesh.boundary()[subdomain].Cf()};

  Foam::Field<Foam::scalar> q = calculate_qw();
  const UserObject & t_bulk_uo = getFoamProblem().getUserObject<UserObject>(_t_bulk_uo_name);
  for (int i = 0; i < _field.size(); ++i)
  {
    const Point p{cellCenters[i].x(), cellCenters[i].y(), cellCenters[i].z()};
    Foam::scalar T_ref = t_bulk_uo.spatialValue(p);
    _field[i] = q[i] / (Tbf[i] - T_ref + eps);
  }
}

const Foam::Field<Foam::scalar>
FoamHeatTransferCoeff::calculate_qw()
{
  auto & foam_mesh{_mesh.fvMesh()};
  const std::string & Tname{getParam<std::string>("T_name")};
  const std::string subdomain{getParam<SubdomainName>("boundary")};
  const auto & Tbf =
      foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(Tname);

  Foam::Field<Foam::scalar> q_w(Tbf.size(), 0.);
  const Foam::thermophysicalTransportModel & ttm =
      foam_mesh.lookupType<Foam::thermophysicalTransportModel>();

  // compute molecular component
  int patchI = foam_mesh.boundary().findIndex(subdomain);

  const auto & kappaEffbf = ttm.kappaEff(patchI);

  q_w = kappaEffbf * Tbf.snGrad();

  return q_w;
}
