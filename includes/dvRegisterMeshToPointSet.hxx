
#ifndef dvRegisterMeshToPointSet_hxx
#define dvRegisterMeshToPointSet_hxx

#include <dvRegisterMeshToPointSet.h>

// Ceres
#include <ceres/ceres.h>
#include <glog/logging.h>
#include <thread>

namespace dv
{

template < typename TFixedMesh, typename TMovingMesh >
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::RegisterMeshToPointSet(const unsigned int& _EDFrame,
                         const TLocatorVector &_locatorVector,
                         const TMovingVector &_movingVector) :
  EDFrame(_EDFrame),
  locatorVector(_locatorVector),
  movingVector(_movingVector),
  NumberOfFrames(this->CalculateNumberOfFrames()),
  NumberOfControlPoints(this->CalculateNumberOfControlPoints()),
  NumberOfSurfacePoints(this->CalculateNumberOfSurfacePoints()),
  NumberOfCells(this->CalculateNumberOfCells())
{
  this->SanityCheck();
}

template < typename TFixedMesh, typename TMovingMesh >
void
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::Register()
{

  //
  // The parameters are all the control points over all the frames
  //

  TParameterVector parameterVector(this->NumberOfFrames);
  for (unsigned int f = 0; f < this->NumberOfFrames; ++f)
    {
    std::vector<double*> parameters(this->NumberOfControlPoints);
    for (unsigned int i = 0; i < this->NumberOfControlPoints; ++i)
      {
      parameters[i] = new double[3] {0.0, 0.0, 0.0};
      }
    parameterVector[f] = parameters;
    }

  //
  // In order to compute the offsets/residuals, we need to store the
  // initial state of the mesh control points over all the frames
  //

  std::cout << "Copying initial points...";

  for (auto moving : movingVector)
    {

    const auto initialPoints = TMoving::PointsContainer::New();

    for (auto it = moving->GetPoints()->Begin();
         it != moving->GetPoints()->End();
         ++it)
      {
      initialPoints->InsertElement(it.Index(),it.Value());
      }

    this->initialPointsVector.emplace_back(initialPoints);
    
    }
  std::cout << "done." << std::endl;

  //
  // Create the problem
  //

  ceres::Problem problem;

  //
  // Minimize distance between surface points and boundary candidates
  //

  if (this->RegistrationWeights.Robust > 1e-6)
    {
    this->AddPrimaryCost(problem, parameterVector);
    }
  if ((this->RegistrationWeights.Velocity > 1e-6) && (this->NumberOfFrames > 1))
    {
    this->AddVelocityRegularizer(problem, parameterVector);
    }
  if ((this->RegistrationWeights.Acceleration) > 1e-6 && (this->NumberOfFrames > 2))
    {
    this->AddAccelerationRegularizer(problem, parameterVector);
    }
  if (this->RegistrationWeights.ThinPlate > 1e-6)
    {
    this->AddThinPlateRegularizer(problem, parameterVector);
    }
  if (this->RegistrationWeights.TriangleAspectRatio > 1e-6)
    {
    this->AddTriangleAspectRatioRegularizer(problem, parameterVector);
    }
  if (this->RegistrationWeights.EdgeLength > 1e-6)
    {
    this->AddEdgeLengthRegularizer(problem, parameterVector);
    }

  ///////////
  // Solve //
  ///////////

  // Processors * Cores * Threads
  const auto threads = std::thread::hardware_concurrency();
  std::cout << "Threads: " << threads << std::endl;

  ceres::Solver::Options solverOptions;
  solverOptions.minimizer_progress_to_stdout = true;
  solverOptions.max_num_iterations = this->MaximumNumberOfIterations;
  solverOptions.function_tolerance = this->FunctionTolerance;
  solverOptions.parameter_tolerance = this->ParameterTolerance;
  solverOptions.max_solver_time_in_seconds = MaximumSolverTimeInSeconds;
  solverOptions.num_threads = threads * 4;
  solverOptions.linear_solver_type = ceres::SPARSE_NORMAL_CHOLESKY;
  solverOptions.dynamic_sparsity = false;
  solverOptions.minimizer_type = ceres::TRUST_REGION;
  ceres::Solver::Summary summary;
  ceres::Solve(solverOptions, &problem, &summary);

  std::cout << summary.FullReport() << std::endl;

  //
  // Serialize summary
  //

  this->summaryString =  "# preprocessor_time_in_seconds: "        + std::to_string(summary.preprocessor_time_in_seconds)        + '\n';
  this->summaryString += "# minimizer_time_in_seconds: "           + std::to_string(summary.minimizer_time_in_seconds)           + '\n';
  this->summaryString += "# postprocessor_time_in_seconds: "       + std::to_string(summary.postprocessor_time_in_seconds)       + '\n';
  this->summaryString += "# total_time_in_seconds: "               + std::to_string(summary.total_time_in_seconds)               + '\n';
  this->summaryString += "# linear_solver_time_in_seconds: "       + std::to_string(summary.linear_solver_time_in_seconds)       + '\n';
  this->summaryString += "# residual_evaluation_time_in_seconds: " + std::to_string(summary.residual_evaluation_time_in_seconds) + '\n';
  this->summaryString += "# jacobian_evaluation_time_in_seconds: " + std::to_string(summary.jacobian_evaluation_time_in_seconds) + '\n';
  this->summaryString += "# inner_iteration_time_in_seconds: "     + std::to_string(summary.inner_iteration_time_in_seconds)     + '\n';
  
  this->summaryString += "Iteration,Cost,CostChange,IterTime,TotalTime,Success\n";
  for (const auto it : summary.iterations)
    {
    summaryString += std::to_string(it.iteration) + ',';
    summaryString += std::to_string(it.cost) + ',';
    summaryString += std::to_string(it.cost_change) + ',';
    summaryString += std::to_string(it.iteration_time_in_seconds) + ',';
    summaryString += std::to_string(it.cumulative_time_in_seconds) + ',';
    summaryString += std::to_string(it.step_is_successful) + '\n';
    }

  //
  // Evaluate residuals to identify cells which are too coarse
  //

  ceres::Problem::EvaluateOptions residualOptions;
  residualOptions.residual_blocks = this->costFunctionResidualIDs;
  double totalCost = 0.0;
  problem.Evaluate(residualOptions,
                   &totalCost,
                   &(this->costFunctionResiduals), nullptr, nullptr);

  //
  // Cleanup
  //

  for (auto v : parameterVector)
    {
    for (auto p : v)
      {
      delete[] p;
      }
    }

}

template < typename TFixedMesh, typename TMovingMesh >
void
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::SanityCheck()
{
  itkAssertOrThrowMacro(this->locatorVector.size() > 0, "");
  itkAssertOrThrowMacro(this->movingVector.size() > 0, "");
  itkAssertOrThrowMacro(this->locatorVector.size() == this->movingVector.size(), "");

  for (const auto& moving : movingVector) {
    for (auto it = moving->GetCellData()->Begin(); it != moving->GetCellData()->End(); ++it) {
      itkAssertOrThrowMacro(it.Value() != 0, "label == 0");
    }
  }
}

template < typename TFixedMesh, typename TMovingMesh >
unsigned int
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::CalculateNumberOfFrames() const
{
  itkAssertOrThrowMacro(this->locatorVector.size() == this->movingVector.size(), "");
  return this->locatorVector.size();
}

template < typename TFixedMesh, typename TMovingMesh >
unsigned int
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::CalculateNumberOfControlPoints() const
{
  const unsigned int n = this->movingVector.front()->GetNumberOfPoints();
  for (const auto v : this->movingVector)
    itkAssertOrThrowMacro(n == v->GetNumberOfPoints(),
      "Mismatch in number of control points.");
  return n;
}

template < typename TFixedMesh, typename TMovingMesh >
unsigned int
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::CalculateNumberOfSurfacePoints() const
{
  const unsigned int n = this->movingVector.front()->GetSurfaceParameterList().size();
  for (const auto v : this->movingVector)
    itkAssertOrThrowMacro(n == v->GetSurfaceParameterList().size(),
      "Mismatch in size of surface parameter list.");
  return n;
}

template < typename TFixedMesh, typename TMovingMesh >
unsigned int
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::CalculateNumberOfCells() const
{
  const unsigned int n = this->movingVector.front()->GetNumberOfCells();
  for (const auto v : this->movingVector)
    itkAssertOrThrowMacro(n == v->GetNumberOfCells(),
      "Mismatch in number of cells.");
  return n;
}

template < typename TFixedMesh, typename TMovingMesh >
void
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::AddPrimaryCost(ceres::Problem& problem, TParameterVector& parameterVector)
{

  std::cout << "Adding primary cost function to problem..." << std::endl;

  auto progress = dv::Progress(this->NumberOfFrames);

  ceres::LossFunction* cost_loss = new ceres::ScaledLoss(nullptr,
                                         this->RegistrationWeights.Robust,
                                         ceres::DO_NOT_TAKE_OWNERSHIP);

  for (unsigned int frame = 0; frame < this->NumberOfFrames; ++frame)
    {

    for (unsigned int index = 0;
         index < this->NumberOfSurfacePoints;
         ++index)
      {

      ceres::CostFunction* cost_function = new TCost(
                                                     this->locatorVector.at(frame),
                                                     this->movingVector.at(frame),
                                                     this->initialPointsVector.at(frame),
                                                     index
                                                    );

      std::vector<double*> params;

      const auto u = this->movingVector.at(frame)->GetSurfaceParameterList().at(index);
      const auto cellList = this->movingVector.at(frame)->GetPointListForCell(u.first);
      for (const auto &cell : cellList)
        {
        params.push_back(parameterVector.at(frame).at(cell));
        }
      const auto id = problem.AddResidualBlock(
                                               cost_function,
                                               cost_loss,
                                               params
                                              );

      costFunctionResidualIDs.emplace_back(id);
      costFunctionFrames.emplace_back(frame);
      const auto cellID = std::get<0>(movingVector.at(frame)->GetSurfaceParameter(index));
      costFunctionCellIDs.emplace_back(cellID);
      }

    progress.UnitCompleted();

    }

}

template < typename TFixedMesh, typename TMovingMesh >
void
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::AddVelocityRegularizer(ceres::Problem& problem, TParameterVector& parameterVector)
{

  std::cout << "Adding velocity regularizer to problem..." << std::endl;

  auto progress = dv::Progress(this->NumberOfFrames);

  ceres::LossFunction* velocity_loss = new ceres::ScaledLoss(nullptr,
                                             this->RegistrationWeights.Velocity,
                                             ceres::DO_NOT_TAKE_OWNERSHIP);

  for (unsigned int frame = 0; frame < this->NumberOfFrames; ++frame)
    {

    for (unsigned int index = 0; index < this->NumberOfControlPoints; ++index)
      {

      ceres::CostFunction* cost_function = new TVelocityRegularizer(
                                                  this->movingVector,
                                                  this->initialPointsVector,
                                                  frame,
                                                  index
                                                  );

      const auto next = (frame + 1) % parameterVector.size();

      std::vector<double*> params;
      params.push_back(parameterVector.at(frame)[index]);
      params.push_back(parameterVector.at(next)[index]);
      problem.AddResidualBlock(cost_function,
                               velocity_loss,
                               params
                               );

      }

    progress.UnitCompleted();

    }

}

template < typename TFixedMesh, typename TMovingMesh >
void
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::AddAccelerationRegularizer(ceres::Problem& problem, TParameterVector& parameterVector)
{

  std::cout << "Adding acceleration regularizer to problem..." << std::endl;

  auto progress = dv::Progress(this->NumberOfFrames);

  ceres::LossFunction* acceleration_loss = new ceres::ScaledLoss(nullptr,
                                                 this->RegistrationWeights.Acceleration,
                                                 ceres::DO_NOT_TAKE_OWNERSHIP);

  for (unsigned int frame = 0; frame < this->NumberOfFrames; ++frame)
    {

    for (unsigned int index = 0; index < this->NumberOfControlPoints; ++index)
      {

      ceres::CostFunction* cost_function = new TAccelerationRegularizer(
                                                 this->movingVector,
                                                 this->initialPointsVector,
                                                 frame,
                                                 index
                                                 );

      const auto prev
        = vnl_math::remainder_floored((int(frame) - 1), int(parameterVector.size()));
      const auto next = (frame + 1) % parameterVector.size();

      std::vector<double*> params;
      params.push_back(parameterVector.at(prev)[index]);
      params.push_back(parameterVector.at(frame)[index]);
      params.push_back(parameterVector.at(next)[index]);
      problem.AddResidualBlock(cost_function,
                               acceleration_loss,
                               params
                               );

      }

    progress.UnitCompleted();

    }

}

template < typename TFixedMesh, typename TMovingMesh >
void
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::AddThinPlateRegularizer(ceres::Problem& problem, TParameterVector& parameterVector)
{

  std::cout << "Adding thin plate regularizer to problem..." << std::endl;

  auto progress = dv::Progress(this->NumberOfFrames);

  ceres::LossFunction* thin_plate_loss = new ceres::ScaledLoss(nullptr,
                                               this->RegistrationWeights.ThinPlate,
                                               ceres::DO_NOT_TAKE_OWNERSHIP);

  for (unsigned int frame = 0; frame < this->NumberOfFrames; ++frame)
    {

    for (unsigned int index = 0; index < this->NumberOfCells; ++index)
      {

      ceres::CostFunction* cost_function
        = new TThinPlateRegularizer(this->movingVector.at(frame),
                                    this->initialPointsVector.at(frame),
                                    index);
  
      const auto cellList = this->movingVector.at(frame)->GetPointListForCell(index);
      std::vector<double*> params;
      for (const auto &cell : cellList)
        {
        params.push_back(parameterVector.at(frame).at(cell));
        }      
      problem.AddResidualBlock(
                               cost_function,
                               thin_plate_loss,
                               params
                              );

      }

    progress.UnitCompleted();

    }

}

template < typename TFixedMesh, typename TMovingMesh >
void
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::AddTriangleAspectRatioRegularizer(ceres::Problem& problem, TParameterVector& parameterVector)
{

  std::cout << "Adding triangle aspect ratio regularizer to problem..." << std::endl;

  auto progress = dv::Progress(this->NumberOfFrames);

  ceres::LossFunction* aspect_ratio_loss = new ceres::ScaledLoss(nullptr,
                                                 this->RegistrationWeights.TriangleAspectRatio,
                                                 ceres::DO_NOT_TAKE_OWNERSHIP);

  for (unsigned int frame = 0; frame < this->NumberOfFrames; ++frame)
    {

    typename TMovingMesh::CellAutoPointer cell;
    for (unsigned int index = 0; index < this->NumberOfCells; ++index)
      {
  
      ceres::CostFunction* cost_function
        = new TTriangleAspectRatioRegularizer(this->movingVector.at(frame),
                                              this->initialPointsVector.at(frame),
                                              index);

      this->movingVector.at(frame)->GetCell( index, cell );
      std::vector<double*> params;
      for (auto it = cell->PointIdsBegin(); it != cell->PointIdsEnd(); ++it)
        {
        params.push_back(parameterVector.at(frame).at(*it));
        }
      problem.AddResidualBlock(
                               cost_function,
                               aspect_ratio_loss,
                               params
                              );

      }

    progress.UnitCompleted();

    }

}

template < typename TFixedMesh, typename TMovingMesh >
void
RegisterMeshToPointSet< TFixedMesh, TMovingMesh >
::AddEdgeLengthRegularizer(ceres::Problem& problem, TParameterVector& parameterVector)
{

  std::cout << "Adding edge length regularizer to problem..." << std::endl;

  auto progress = dv::Progress(this->NumberOfFrames);

  ceres::LossFunction* edge_length_loss = new ceres::ScaledLoss(nullptr,
                                                 this->RegistrationWeights.EdgeLength,
                                                 ceres::DO_NOT_TAKE_OWNERSHIP);

  for (unsigned int frame = 0; frame < this->NumberOfFrames; ++frame)
    {

    for (size_t i = 0;
         i < this->movingVector.at(frame)->GetNumberOfEdges();
         ++i)
      {
  
      ceres::CostFunction* cost_function
        = new TEdgeLengthRegularizer(this->movingVector.at(frame),
                                     this->initialPointsVector.at(frame),
                                     i);

      std::vector<double*> params;

      params.push_back(parameterVector.at(frame).at(
        this->movingVector.at(frame)->GetEdge( i )->GetOrigin()
                                                   ));
      params.push_back(parameterVector.at(frame).at(
        this->movingVector.at(frame)->GetEdge( i )->GetDestination()
                                                   ));

      problem.AddResidualBlock(
                               cost_function,
                               edge_length_loss,
                               params
                              );

      }

    progress.UnitCompleted();

    }

}

}

#endif

