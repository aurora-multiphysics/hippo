#include "AdjacentCellBulkTemperature.h"
#include "HippoBase.h"
#include "FoamProblem.h"
#include "MooseTypes.h"
#include <ListOps.H>
#include <Pstream/mpi/PstreamGlobals.H>
#include <PstreamReduceOps.H>
#include <UList.H>
#include <VectorSpace.H>
#include <fvMesh.H>
#include <ops.H>
#include <scalar.H>
#include <scalarField.H>

registerMooseObject("hippoApp", AdjacentCellBulkTemperature);

InputParameters
AdjacentCellBulkTemperature::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<SubdomainName>("boundary", "Boundary this object applies to.");
  params.addParam<std::string>("T_name", "T", "Temperature name in the OpenFOAM simulation.");
  return params;
}

AdjacentCellBulkTemperature::AdjacentCellBulkTemperature(const InputParameters & params)
  : GeneralUserObject(params),
    HippoBase(this),
    _foam_patch{getFoamPatch(getParam<SubdomainName>("boundary"))}
{
}

const Foam::fvPatch &
AdjacentCellBulkTemperature::getFoamPatch(const std::string & boundary)
{
  const Foam::fvMesh & foam_mesh{getFoamProblem().mesh().fvMesh()};
  const Foam::label & boundaryId{foam_mesh.boundary().findIndex(boundary)};
  return foam_mesh.boundary()[boundaryId];
}

Real
AdjacentCellBulkTemperature::spatialValue(const Point & point) const
{
  const Foam::fvMesh & foam_mesh{getFoamProblem().mesh().fvMesh()};

  // Boundary face centres
  const Foam::vectorField & face_centres = _foam_patch.Cf();
  const Foam::point p_moose{point(0), point(1), point(2)};
  Foam::scalar minDist = Foam::rootVGreat;
  Foam::label idx;

  for (int i = 0; i < face_centres.size(); ++i)
  {
    const Foam::point & p = face_centres[i];
    const Foam::scalar dist{Foam::mag(p_moose - p)};
    if (dist < minDist)
    {
      idx = _foam_patch.faceCells()[i];
      minDist = dist;
    }
  }

  return foam_mesh.lookupObject<Foam::volScalarField>(getParam<std::string>("T_name"))[idx];
}
