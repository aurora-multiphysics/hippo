#include "AdjacentCellBulkTemperature.h"
#include "HippoBase.h"
#include "FoamProblem.h"
#include "MooseTypes.h"
#include <ListOps.H>
#include <VectorSpace.H>
#include <fvMesh.H>
#include <scalar.H>
#include <scalarField.H>

registerMooseObject("hippoApp", AdjacentCellBulkTemperature);

InputParameters
AdjacentCellBulkTemperature::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<SubdomainName>("boundary", "Boundary this object applies to.");
  return params;
}

AdjacentCellBulkTemperature::AdjacentCellBulkTemperature(const InputParameters & params)
  : GeneralUserObject(params), HippoBase(this), _T_bulk{getTBoundaryField()}
{
}

const Foam::scalarField &
AdjacentCellBulkTemperature::getTBoundaryField()
{
  const Foam::fvMesh & foam_mesh{getFoamProblem().mesh().fvMesh()};
  const auto & T{foam_mesh.lookupObject<Foam::volScalarField>("T")};
  const Foam::label & boundaryId{
      foam_mesh.boundary().findIndex(getParam<SubdomainName>("boundary"))};
  return T.boundaryField()[boundaryId].patchInternalField();
}

Real
AdjacentCellBulkTemperature::spatialValue(const Point & point) const
{
  const Foam::fvMesh & foam_mesh{getFoamProblem().mesh().fvMesh()};
  const Foam::label & boundaryId{
      foam_mesh.boundary().findIndex(getParam<SubdomainName>("boundary"))};

  const Foam::fvPatch & patch = foam_mesh.boundary()[boundaryId];

  // Boundary face centres
  const Foam::vectorField & face_centres = patch.Cf();
  Foam::point p_moose{point(0), point(1), point(2)};
  Foam::scalar minDist = Foam::rootVGreat;
  Foam::label idx;
  for (int i = 0; i < face_centres.size(); ++i)
  {
    Foam::point p = face_centres[i];
    Foam::scalar dist{Foam::magSqr(p_moose - p)};
    if (dist < minDist)
    {
      idx = i;
      minDist = dist;
    }
  }

  return _T_bulk[idx];
}
