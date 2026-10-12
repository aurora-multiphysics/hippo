#pragma once

#include "HippoInterface.h"
#include "InputParameters.h"
#include "MultiAppTransfer.h"
#include <functional>

class MultiAppFoamTransfer : public MultiAppTransfer, public HippoInterface
{
public:
  static InputParameters validParams();
  MultiAppFoamTransfer(const InputParameters & params);

protected:
  /// Getter for the active source transform
  const MultiAppCoordTransform & getActiveFromTransform() const
  {
    return *_from_transforms[_active_from_global_app_index];
  }
  /// Getter for the active destination transform
  const MultiAppCoordTransform & getActiveToTransform() const
  {
    return *_to_transforms[_active_to_global_app_index];
  }
  /// Map a point in the active destination app frame to the active source app frame
  libMesh::Point mapPointToActiveSourceFrame(const Point & point_in_target_frame) const
  {
    return getActiveFromTransform().mapBack(getActiveToTransform()(point_in_target_frame));
  }
  /// Getter for current problem containing source variables
  virtual FEProblemBase & getActiveFromProblem() { return *_active_from_problem; }
  /// Getter for current problem containing destination variables
  virtual FEProblemBase & getActiveToProblem() { return *_active_to_problem; }
  /// Get libMesh EquationSystem, which may or may not be displaced
  libMesh::EquationSystems & getlibMeshEquationSystem(FEProblemBase & problem,
                                                      bool use_displaced) const;

private:
  /// Pointer to active destination problem variable is being transferred to
  std::optional<std::reference_wrapper<FEProblemBase>> _active_to_problem;
  /// Pointer to active source problem variable is being transferred from
  std::optional<std::reference_wrapper<FEProblemBase>> _active_from_problem;
  /// Global app index for the active destination problem
  unsigned int _active_to_global_app_index{0};
  /// Global app index for the active source problem
  unsigned int _active_from_global_app_index{0};
};
