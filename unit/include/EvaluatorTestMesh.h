#pragma once

#include "LibMeshNearestLocationEvaluator.h"
#include "LibMeshShapeFunctionEvaluator.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "libmesh/elem.h"
#include "libmesh/equation_systems.h"
#include "libmesh/explicit_system.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/parameters.h"
#include "libmesh/partitioner.h"
#include "libmesh/replicated_mesh.h"
#include <functional>
#include <memory>
#include <set>
#include <string>

namespace EvaluatorTest
{
using ShapeEvaluator = LibMeshShapeFunctionEvaluator<Foam::scalar>;
using NearestEvaluator = LibMeshNearestLocationEvaluator<Foam::scalar>;

enum class GeomType
{
  Square,
  Line
};

enum class Ownership
{
  Default,
  ThreeOnRankZero, // Element 0 belongs to rank 1.
  ThreeOnRankOne,  // Element 0 belongs to rank 0.
  AllOnRankZero
};

struct EvaluatorTestMesh
{
  const libMesh::Parallel::Communicator & comm;
  libMesh::ReplicatedMesh mesh;
  libMesh::EquationSystems equations;
  libMesh::ExplicitSystem & system;
  unsigned int variable;
  bool nodal;

  EvaluatorTestMesh(const libMesh::Parallel::Communicator & communicator,
                    GeomType geom_type,
                    libMesh::Order order = libMesh::FIRST,
                    libMesh::FEFamily family = libMesh::LAGRANGE,
                    bool restricted = false,
                    unsigned int p_level = 0,
                    Ownership ownership = Ownership::Default,
                    bool no_active_blocks = false)
    : comm(communicator),
      mesh(comm, geom_type == GeomType::Line ? 1 : 2),
      equations(mesh),
      system(equations.add_system<libMesh::ExplicitSystem>("source")),
      nodal(family == libMesh::LAGRANGE)
  {
    auto element_order = order;
    if (family == libMesh::LAGRANGE)
    {
      // Lagrange p-refinement requires geometry nodes for the increased order.
      element_order = static_cast<libMesh::Order>(order + p_level);
    }
    buildMesh(geom_type, element_order);

    // Block 0 contains the left half (centroid x < 1); block 1 contains the right half.
    for (auto * elem : mesh.element_ptr_range())
    {
      elem->subdomain_id() = elem->vertex_average()(0) < 1. ? 0 : 1;
      elem->set_p_level(p_level);
    }

    // Assign elements to MPI ranks.
    setOwnership(ownership);

    mesh.prepare_for_use();
    libMesh::subdomain_id_type block_id = 0;
    if (no_active_blocks)
    {
      // Restrict the variable to nonexistent block 99 so it has no active elements.
      block_id = 99;
    }

    const std::set<libMesh::subdomain_id_type> blocks{block_id};
    variable =
        system.add_variable("u", order, family, restricted || no_active_blocks ? &blocks : nullptr);
    equations.init();
  }

  void project(const std::function<Real(const Point &)> & source_value)
  {
    using SourceFunction = std::function<Real(const Point &)>;
    // Projection is synchronous; the callback borrows the function for this call only.
    libMesh::Parameters parameters;
    parameters.set<const SourceFunction *>("source_value") = &source_value;
    const auto value = [](const Point & point,
                          const libMesh::Parameters & parameters,
                          const std::string &,
                          const std::string &) -> libMesh::Number
    { return (*parameters.get<const SourceFunction *>("source_value"))(point); };
    system.project_solution(value, nullptr, parameters);
    system.solution->close();
    system.update();
  }

  void constants(Real bottom_left, Real bottom_right, Real top_left, Real top_right)
  {
    system.solution->zero();
    for (const auto * elem : mesh.active_local_element_ptr_range())
      if (elem->n_dofs(system.number(), variable))
      {
        const auto center = elem->vertex_average();
        Real value;
        if (center(1) > 1.)
          value = center(0) > 1. ? top_right : top_left;
        else
          value = center(0) > 1. ? bottom_right : bottom_left;
        system.solution->set(elem->dof_number(system.number(), variable, 0), value);
      }
    system.solution->close();
    system.update();
  }

  std::unique_ptr<ShapeEvaluator> getShapeEvaluator(libMesh::EquationSystems * geometry = nullptr)
  {
    return std::make_unique<ShapeEvaluator>(
        system, variable, geometry ? *geometry : equations, comm);
  }

  std::unique_ptr<NearestEvaluator>
  getNearestEvaluator(const std::string & mode = "variable_default",
                      unsigned int num_nearest_points = 1,
                      libMesh::MeshBase * geometry = nullptr)
  {
    return std::make_unique<NearestEvaluator>(system,
                                              variable,
                                              geometry ? *geometry : mesh,
                                              comm,
                                              MooseEnum("nodes centroids variable_default", mode),
                                              nodal,
                                              4,
                                              num_nearest_points);
  }

private:
  static libMesh::ElemType getElementType(GeomType geom_type, libMesh::Order order)
  {
    switch (order)
    {
      case libMesh::CONSTANT:
      case libMesh::FIRST:
        return geom_type == GeomType::Line ? libMesh::EDGE2 : libMesh::QUAD4;
      case libMesh::SECOND:
        return geom_type == GeomType::Line ? libMesh::EDGE3 : libMesh::QUAD9;
      case libMesh::THIRD:
        if (geom_type == GeomType::Line)
          return libMesh::EDGE4;
        break;
      default:
        break;
    }
    mooseError("Unsupported element order for EvaluatorTestMesh geometry.");
  }

  void buildMesh(GeomType geom_type, libMesh::Order order)
  {
    const auto elem_type = getElementType(geom_type, order);
    switch (geom_type)
    {
      case GeomType::Square:
        libMesh::MeshTools::Generation::build_square(mesh, 2, 2, 0., 2., 0., 2., elem_type);
        break;
      case GeomType::Line:
        libMesh::MeshTools::Generation::build_line(mesh, 2, 0., 2., elem_type);
        break;
    }
    mesh.allow_renumbering(false);
  }

  void setOwnership(Ownership ownership)
  {
    if (ownership == Ownership::Default)
      return;

    for (auto * elem : mesh.element_ptr_range())
    {
      switch (ownership)
      {
        case Ownership::Default:
          break;
        case Ownership::ThreeOnRankZero:
          elem->processor_id() = elem->id() == 0 ? 1u : 0u;
          break;
        case Ownership::ThreeOnRankOne:
          elem->processor_id() = elem->id() == 0 ? 0u : 1u;
          break;
        case Ownership::AllOnRankZero:
          elem->processor_id() = 0;
          break;
      }
    }
    mesh.skip_partitioning(true);
    libMesh::Partitioner::set_node_processor_ids(mesh);
  }
};

} // namespace EvaluatorTest
