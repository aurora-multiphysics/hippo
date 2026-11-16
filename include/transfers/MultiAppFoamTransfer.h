#pragma once

#include "InputParameters.h"
#include "MultiAppTransfer.h"
#include <functional>
#include <optional>

class FoamProblem;

class MultiAppFoamTransfer : public MultiAppTransfer
{
public:
  static InputParameters validParams();
  MultiAppFoamTransfer(const InputParameters & params);

protected:
  /// Set the active source problem and its global app index.
  void setActiveFromProblem(FEProblemBase & problem, unsigned int global_app_index)
  {
    _active_from_problem = std::ref(problem);
    _active_from_global_app_index = global_app_index;
  }
  /// Set the active destination problem and its global app index.
  void setActiveToProblem(FEProblemBase & problem, unsigned int global_app_index)
  {
    _active_to_problem = std::ref(problem);
    _active_to_global_app_index = global_app_index;
  }
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
  /// Casts problem to a FoamProblem with checks
  FoamProblem & foamProblem(FEProblemBase & problem) const;

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
