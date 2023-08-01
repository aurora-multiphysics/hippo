#include "FoamProblem.h"
#include "FoamInterface.h"
#include "FoamMesh.h"
#include "AuxiliarySystem.h"

registerMooseObject("hippoApp", FoamProblem);

InputParameters
FoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();
  // std::vector<std::string> empty_vec;
  // params.addParam<std::vector<std::string>>(
  //    "foam_args", empty_vec, "List of arguments to be passed to openFoam solver");
  // TODO: needs to be a vector
  return params;
}

FoamProblem::FoamProblem(InputParameters const & params)
  : ExternalProblem(params),
    _foam_mesh(dynamic_cast<FoamMesh *>(&this->ExternalProblem::mesh())),
    _interface(_foam_mesh->getFoamInterface())
{
  assert(_foam_mesh);
  assert(_interface);
}

void
FoamProblem::externalSolve()
{
}

registerMooseObject("hippoApp", BuoyantFoamProblem);

InputParameters
BuoyantFoamProblem::validParams()
{
  auto params = ExternalProblem::validParams();
  return params;
}

BuoyantFoamProblem::BuoyantFoamProblem(InputParameters const & params)
  : FoamProblem(params), _app(_interface), _T_number(100000000)
// TODO: Assuming the temp var is "T" should pass the name in
{
}

void
BuoyantFoamProblem::addExternalVariables()
{
  // Plagerised from cardinal NekRSProblemBase.C
  InputParameters params = _factory.getValidParams("MooseVariable");
  params.set<MooseEnum>("family") = "LAGRANGE";
  params.set<MooseEnum>("order") = "FIRST";

  addAuxVariable("MooseVariable", "foamT", params);
  _T_number = _aux->getFieldVariable<Real>(0, "foamT").number();
}

void
BuoyantFoamProblem::externalSolve()
{
  _app.run();
}

void
BuoyantFoamProblem::syncSolutions(Direction dir)
{
  auto & mesh = static_cast<FoamMesh &>(this->mesh());
  auto serial = mesh.isSerial();

  if (dir == ExternalProblem::Direction::FROM_EXTERNAL_APP)
  {

    std::vector<Real> foamT;
    foamT.reserve(mesh.nNodes());

    auto subdomains = mesh.getSubdomainList();
    // First we get all the temperature data for every sobdomain
    std::vector<size_t> patch_counts(subdomains.size() + 1, 0);
    {
      int i = 0;
      for (auto const & subdom : subdomains)
      {
        patch_counts[i++] = _app.append_patchT(subdom, foamT);
      }
    }
    std::exclusive_scan(patch_counts.begin(), patch_counts.end(), patch_counts.begin(), 0);
    for (int i = 0; i < subdomains.size(); ++i)
    {
      auto subdomain = subdomains[i]; // should be the same as patch_id
      auto offset = patch_counts[i];
      for (int node = offset; node < patch_counts[i + 1]; ++node)
      {

        auto node_ptr = mesh.getNodePtr(node - offset, subdomain);
        assert(node_ptr);
        auto dof = node_ptr->dof_number(_aux->number(), _T_number, 0);
        _aux->solution().set(dof, foamT[node]);
      }
    }

    _aux->solution().close();
  }
  else if (dir == ExternalProblem::Direction::TO_EXTERNAL_APP)
  {
    ;
  }
}

// Local Variables:
// mode: c++
// End:
