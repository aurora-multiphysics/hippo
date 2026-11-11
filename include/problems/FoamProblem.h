#pragma once

#include "FoamMesh.h"
#include "FoamPostprocessorBase.h"
#include "FoamSolver.h"
#include "FoamFieldBase.h"
#include "FoamBCBase.h"
#include "TransferContainerBase.h"
#include "BoundaryTransferContainer.h"

#include <ExternalProblem.h>
#include <MooseError.h>
#include <MooseTypes.h>
#include <MooseVariableFieldBase.h>
#include <SystemBase.h>
#include <libmesh/elem.h>
#include <unordered_map>
#include <functional>
#include <memory>
#include <utility>

class FoamProblem : public ExternalProblem
{
public:
  FoamProblem(InputParameters const & params);
  static InputParameters validParams();
  virtual void externalSolve() override;
  virtual void syncSolutions(Direction /* dir */) override;
  virtual bool converged(const unsigned int /* nl_sys_num */) override { return true; }
  virtual void addExternalVariables() override {};
  virtual void initialSetup() override;

  /// Register borrowed fields, or merge compatible references into an unlocked container.
  /// Existing boundaries must reference the same field object; locked containers reject all calls.
  template <typename T>
  BoundaryTransferContainer<T> &
  addBoundaryTransfer(const std::string & name,
                      TransferQuantity quantity,
                      typename BoundaryTransferContainer<T>::FieldMap & fields);

  template <typename T>
  BoundaryTransferContainer<T> &
  addBoundaryTransfer(const std::string & name,
                      TransferQuantity quantity,
                      typename BoundaryTransferContainer<T>::FieldRefMap & fields);

  /// Check the exact name/type/quantity key without creating a registration.
  template <typename T>
  bool hasBoundaryTransfer(const std::string & name, TransferQuantity quantity) const;

  /// Retrieve a registered boundary container, reporting missing or incompatible entries.
  template <typename T>
  BoundaryTransferContainer<T> & getBoundaryTransfer(const std::string & name,
                                                     TransferQuantity quantity);

  using ExternalProblem::mesh;
  virtual FoamMesh const & mesh() const override { return *_foam_mesh; }
  virtual FoamMesh & mesh() override { return *_foam_mesh; }

  enum class SyncVariables
  {
    WallTemperature,
    WallHeatFlux,
    Both
  };

  Hippo::FoamSolver & solver() { return _solver; }

protected:
  // check FoamVariables and print summarising table
  void verifyFoamVariables();

  // check FoamBCs and print summarising table
  void verifyFoamBCs();

  // check FoamPostprocessors and print summarising table
  void verifyFoamPostprocessors();

  FoamMesh * _foam_mesh = nullptr;
  Hippo::FoamSolver _solver;

  std::vector<FoamFieldBase *> _foam_variables;
  std::vector<FoamBCBase *> _foam_bcs;
  std::vector<FoamPostprocessorBase *> _foam_postprocessor;

  TransferRegistry _transfers;
};

template <typename T>
BoundaryTransferContainer<T> &
FoamProblem::addBoundaryTransfer(const std::string & name,
                                 TransferQuantity quantity,
                                 typename BoundaryTransferContainer<T>::FieldMap & fields)
{
  typename BoundaryTransferContainer<T>::FieldRefMap references;
  for (auto & [boundary, field] : fields)
    references.emplace(boundary, std::ref(field));

  return addBoundaryTransfer<T>(name, quantity, references);
}

template <typename T>
BoundaryTransferContainer<T> &
FoamProblem::addBoundaryTransfer(const std::string & name,
                                 TransferQuantity quantity,
                                 typename BoundaryTransferContainer<T>::FieldRefMap & fields)
{
  const TransferKey key{name, TransferFieldTraits<T>::value, quantity};
  if (_transfers.find(key) != _transfers.end())
  {
    auto & container = getBoundaryTransfer<T>(name, quantity);
    if (!container.modifiable())
      mooseError("Cannot extend locked boundary transfer '",
                 name,
                 "' (type ",
                 static_cast<int>(key.type),
                 ", quantity ",
                 static_cast<int>(quantity),
                 ").");
    container.addBoundaries(fields);
    return container;
  }

  auto container = std::make_unique<BoundaryTransferContainer<T>>(fields);
  auto & result = *container;
  _transfers.emplace(key, std::move(container));
  return result;
}

template <typename T>
bool
FoamProblem::hasBoundaryTransfer(const std::string & name, TransferQuantity quantity) const
{
  const TransferKey key{name, TransferFieldTraits<T>::value, quantity};
  return _transfers.find(key) != _transfers.end();
}

template <typename T>
BoundaryTransferContainer<T> &
FoamProblem::getBoundaryTransfer(const std::string & name, TransferQuantity quantity)
{
  const TransferKey key{name, TransferFieldTraits<T>::value, quantity};
  const auto it = _transfers.find(key);
  if (it == _transfers.end())
    mooseError("Boundary transfer '",
               name,
               "' is not registered (type ",
               static_cast<int>(key.type),
               ", quantity ",
               static_cast<int>(quantity),
               ").");

  auto * container = dynamic_cast<BoundaryTransferContainer<T> *>(it->second.get());
  if (!container)
    mooseError("Transfer '",
               name,
               "' is not a compatible boundary container (type ",
               static_cast<int>(key.type),
               ", quantity ",
               static_cast<int>(quantity),
               ").");

  return *container;
}
