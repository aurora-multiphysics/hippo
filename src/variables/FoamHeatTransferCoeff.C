#include "FoamFieldBase.h"
#include "FoamHeatTransferCoeff.h"
#include "FoamProblem.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "Registry.h"
#include "UserObject.h"
#include "hippoUtils.h"
#include <scalar.H>
#include <scalarAndError.H>
#include <scalarField.H>
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
    _t_bulk_uo_name(getParam<UserObjectName>("bulk_temperature_uo"))
{
}

void
FoamHeatTransferCoeff::transferVariable()
{
  THREAD_ID tid = getParam<THREAD_ID>("_tid");
  auto & moose_var = getFoamProblem().getVariable(tid, _name);

  Foam::scalarField htc = calculateHTC();
  Hippo::internal::copyFieldFoamToMoose(_mesh, htc, moose_var, _subdomain);
  moose_var.sys().solution().close();
}

Foam::scalarField
FoamHeatTransferCoeff::calculateHTC()
{
  const std::string & subdomain{getParam<SubdomainName>("boundary")};
  const std::string & Tname{getParam<std::string>("T_name")};
  Foam::fvMesh & foam_mesh{_mesh.fvMesh()};

  const auto & Tbf =
      foam_mesh.boundary()[subdomain].lookupPatchField<Foam::volScalarField, double>(Tname);
  Foam::scalarField q = calculate_qw(Tbf);
  const UserObject & t_bulk_uo = getFoamProblem().getUserObject<UserObject>(_t_bulk_uo_name);

  Foam::scalarField htc{Tbf.size(), 0};
  const Foam::vectorField & cellCenters{foam_mesh.boundary()[subdomain].Cf()};
  const Foam::scalar eps = Foam::ROOTVSMALL;
  for (int i = 0; i < htc.size(); ++i)
  {
    const Point p{cellCenters[i].x(), cellCenters[i].y(), cellCenters[i].z()};
    Foam::scalar T_ref = t_bulk_uo.spatialValue(p);
    htc[i] = q[i] / (Tbf[i] - T_ref + eps);
  }

  return htc;
}

const Foam::Field<Foam::scalar>
FoamHeatTransferCoeff::calculate_qw(const Foam::fvPatchScalarField & Tbf)
{
  auto & foam_mesh{_mesh.fvMesh()};
  Foam::Field<Foam::scalar> q_w(Tbf.size(), 0.);
  const Foam::thermophysicalTransportModel & ttm =
      foam_mesh.lookupType<Foam::thermophysicalTransportModel>();

  // use kappaEff as this would also account for turbulence modelling while being the same as
  // molecular in other cases
  const auto & kappaEffbf = ttm.kappaEff(Tbf.patch().index());

  q_w = kappaEffbf * Tbf.snGrad();

  return q_w;
}
