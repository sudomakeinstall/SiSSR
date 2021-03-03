// STD
#include <iostream>

// Custom
#include <dvCameraState.h>
#include <dvRapidJSONHelper.h>
#include <dvVectorOperations.h>

// SiSSR
#include <sissrStateMachine.h>

namespace sissr {

StateMachine
::StateMachine() :
  InitialModelFrame(0),
  InitialModelNumberOfFaces(512),
  InitialModelSigma(0.1),
  InitialModelLVClosingRadius(10),
  InitialModelGeneralClosingRadius(5),
  InitialModelPreserveEdges(true),
  InitialModelDecimationTechnique(DecimationTechnique::LindstromTurk),
  BoundaryCandidateDilationRadius(3)
{}

vtkSmartPointer<vtkFloatArray>
StateMachine
::CalculateSQUEEZ(unsigned int frame)
{
  const auto ed = this->SurfaceAreas.get_column(this->EDFrame);
  const auto cu = this->SurfaceAreas.get_column(frame);
  const auto sq = element_quotient(cu, ed).apply(std::sqrt);

  const auto cellData = vtkSmartPointer<vtkFloatArray>::New();
  cellData->SetNumberOfComponents(1);
  cellData->SetNumberOfValues( sq.size() );

  for (unsigned int i = 0; i < sq.size(); ++i) cellData->SetValue(i, sq.get(i));
  return cellData;
}

vtkSmartPointer<vtkFloatArray>
StateMachine
::CalculateResidualsForFrame(unsigned int frame)
{

  const auto logical = dv::ElementwiseEqual(this->costFunctionFrames, frame);
  auto residualX     = dv::BooleanIndexing(this->costFunctionResidualX, logical);
  auto residualY     = dv::BooleanIndexing(this->costFunctionResidualY, logical);
  auto residualZ     = dv::BooleanIndexing(this->costFunctionResidualZ, logical);
  const auto cellIDs = dv::BooleanIndexing(this->costFunctionCellIDs, logical);

  const auto square = [](double x) { return x * x; };
  std::transform(residualX.cbegin(), residualX.cend(), residualX.begin(), square);
  std::transform(residualY.cbegin(), residualY.cend(), residualY.begin(), square);
  std::transform(residualZ.cbegin(), residualZ.cend(), residualZ.begin(), square);

  std::map<unsigned int, double> mp;
  for (unsigned int i = 0; i < cellIDs.size(); ++i)
    {
    if (mp.count(cellIDs.at(i)) == 0) mp[cellIDs.at(i)] = 0;
    mp[cellIDs.at(i)] += residualX.at(i);
    mp[cellIDs.at(i)] += residualY.at(i);
    mp[cellIDs.at(i)] += residualZ.at(i);
    }

  const auto cellData = vtkSmartPointer<vtkFloatArray>::New();
  cellData->SetNumberOfComponents(1);
  cellData->SetNumberOfValues( mp.size() );

  for (auto it = mp.cbegin(); it != mp.cend(); ++it)
    {
    cellData->SetValue(it->first, sqrt(it->second / 6));
    }

  return cellData;

}

void
StateMachine
::SerializeSegmentation(std::ofstream& s, vtkFloatArray* ids) const
{
  if (nullptr == ids)
    {
    std::cerr << "WARNING: Segment IDs have not been calculated.  Returning." << std::endl;
    return;
    }

  s << "CellID,SegmentID\n";

  for (size_t i = 0; i < static_cast<size_t>(ids->GetSize()); ++i)
    s << i << ',' << ids->GetValue( i ) << '\n';
  s << std::flush;
}

void
StateMachine
::SerializeResiduals(std::ofstream& s)
{
  if (this->costFunctionFrames.size() != this->costFunctionCellIDs.size()   ||
      this->costFunctionFrames.size() != this->costFunctionResidualX.size() ||
      this->costFunctionFrames.size() != this->costFunctionResidualY.size() ||
      this->costFunctionFrames.size() != this->costFunctionResidualZ.size()
     )
    {
    std::cerr << "WARNING: Residual sizes don't match.\n"
              << "costFunctionFrames: "    << this->costFunctionFrames.size()    << '\n'
              << "costFunctionCellIDs: "   << this->costFunctionCellIDs.size()   << '\n'
              << "costFunctionResidualX: " << this->costFunctionResidualX.size() << '\n'
              << "costFunctionResidualY: " << this->costFunctionResidualY.size() << '\n'
              << "costFunctionResidualZ: " << this->costFunctionResidualZ.size() << '\n'
              << std::flush;
    return;
    }

  s << "Frame,CellID,ResidualX,ResidualY,ResidualZ\n";
  for (size_t i = 0; i < this->costFunctionFrames.size(); ++i)
    {
    s << this->costFunctionFrames.at(i) << ','
      << this->costFunctionCellIDs.at(i) << ','
      << this->costFunctionResidualX.at(i) << ','
      << this->costFunctionResidualY.at(i) << ','
      << this->costFunctionResidualZ.at(i) << '\n';
    }

}

void
StateMachine
::DeserializeResiduals(std::ifstream &s)
{

  // Discard first lines
  std::string trash;
  std::getline(s, trash);

  std::string frame, cell, residual;

  while (s.good() && s.peek() != std::ifstream::traits_type::eof())
    {
    std::getline(s, frame, ',');
    this->costFunctionFrames.emplace_back(std::stoi(frame));
    std::getline(s, cell, ',');
    this->costFunctionCellIDs.emplace_back(std::stoi(cell));
    std::getline(s, residual, ',');
    this->costFunctionResidualX.emplace_back(std::stod(residual));
    std::getline(s, residual, ',');
    this->costFunctionResidualY.emplace_back(std::stod(residual));
    std::getline(s, residual, '\n');
    this->costFunctionResidualZ.emplace_back(std::stod(residual));
    }

}

void
StateMachine
::SerializeSurfaceArea(std::ofstream& s, const vnl_matrix<double> &areas)
{
  if (areas.empty()) { return; }
  s << "# Surface Area"
    << "\n# End Diastolic Frame: " << this->EDFrame
    << "\n# Rows: " << areas.rows()
    << "\n# Cols: " << areas.cols()
    << "\nCellID,Frame,Area\n";

  for (unsigned int row = 0; row < areas.rows(); ++row)
    {
    for (unsigned int col = 0; col < areas.cols(); ++col)
      {
      s << row << ',' << col << ',' << areas.get(row,col) << '\n';
      }
    }
  s << std::flush;
}

void
StateMachine
::DeserializeSurfaceArea(std::ifstream& s)
{

  unsigned int nRows = 0;
  unsigned int nCols = 0;
  // Parse header
    {
    std::string trash;

    // Discard first two lines
    std::getline(s, trash);
    std::getline(s, trash);

    // Get rows
    std::getline(s, trash, ' ');
    std::getline(s, trash, ' ');
    std::getline(s, trash, '\n');
    nRows = std::stoi(trash);

    // Get cols
    std::getline(s, trash, ' ');
    std::getline(s, trash, ' ');
    std::getline(s, trash, '\n');
    nCols = std::stoi(trash);

    std::getline(s, trash);
    }

  this->SurfaceAreas.set_size(nRows,nCols);

  std::string line;
  while (s.good() && s.peek() != std::ifstream::traits_type::eof())
    {
    std::string row, col, v;
    std::getline(s, row, ',');
    std::getline(s, col, ',');
    std::getline(s, v);
    this->SurfaceAreas.put(std::stoi(row), std::stoi(col), std::stod(v));
    }

  for (unsigned int f = 0; f < this->SurfaceAreas.cols(); ++f)
    {
    this->SQUEEZ[f] = this->CalculateSQUEEZ(f);
    }

}

void
StateMachine
::SerializeJSON(rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer)
{

  writer.Key("InitialModelFrame");
  writer.Uint(this->InitialModelFrame);
  writer.Key("InitialModelNumberOfFaces");
  writer.Uint(this->InitialModelNumberOfFaces);
  writer.Key("InitialModelSigma");
  writer.Uint(this->InitialModelSigma);
  writer.Key("InitialModelLVClosingRadius");
  writer.Uint(this->InitialModelLVClosingRadius);
  writer.Key("InitialModelGeneralClosingRadius");
  writer.Uint(this->InitialModelGeneralClosingRadius);
  writer.Key("InitialModelGeneralClosingRadius");
  writer.Uint(this->InitialModelGeneralClosingRadius);
  writer.Key("InitialModelPreserveEdges");
  writer.Bool(this->InitialModelPreserveEdges);

  writer.Key("BoundaryCandidateDilationRadius");
  writer.Uint(this->BoundaryCandidateDilationRadius);

  if (this->EDFrameHasBeenSet)
    {
    writer.Key("EDFrame");
    writer.Uint(this->EDFrame);
    }
  writer.Key("CurrentFrame");
  writer.Uint(this->CurrentFrame);

  writer.Key("RegistrationUseLabels");
  writer.Bool(this->RegistrationUseLabels);
  writer.Key("RegistrationWeights.Primary");
  writer.Double(this->RegistrationWeights.Primary);
  writer.Key("RegistrationWeights.EdgeWeight");
  writer.Double(this->RegistrationWeights.EdgeWeight);
  writer.Key("RegistrationWeights.Velocity");
  writer.Double(this->RegistrationWeights.Velocity);
  writer.Key("RegistrationWeights.Acceleration");
  writer.Double(this->RegistrationWeights.Acceleration);
  writer.Key("RegistrationWeights.ThinPlate");
  writer.Double(this->RegistrationWeights.ThinPlate);
  writer.Key("RegistrationWeights.TriangleAspectRatio");
  writer.Double(this->RegistrationWeights.TriangleAspectRatio);
  writer.Key("RegistrationWeights.EdgeLength");
  writer.Double(this->RegistrationWeights.EdgeLength);

  // Camera
  this->camera.SerializeJSON(writer);

  writer.Key("ImagePlanesAreVisible");
  writer.Bool(this->ImagePlanesAreVisible);
  writer.Key("CandidatesAreVisible");
  writer.Bool(this->CandidatesAreVisible);
  writer.Key("ModelIsVisible");
  writer.Bool(this->ModelIsVisible);

  if (this->planeWidgetState.HasBeenCaptured) {
    writer.Key("planeWidgetState.Origin");
    writer.StartArray();
    for (size_t i = 0; i < 3; ++i) {
      writer.Double(this->planeWidgetState.Origin[i]);
    }
    writer.EndArray();
  }

  writer.Key("planeWidgetState.Point1");
  writer.StartArray();
  for (size_t i = 0; i < 3; ++i) writer.Double(this->planeWidgetState.Point1[i]);
  writer.EndArray();

  writer.Key("planeWidgetState.Point2");
  writer.StartArray();
  for (size_t i = 0; i < 3; ++i) writer.Double(this->planeWidgetState.Point2[i]);
  writer.EndArray();

  writer.Key("CellDataToDisplay");
  writer.Int( static_cast<int>(this->CellDataToDisplay) );

}

void
StateMachine
::DeserializeJSON(const rapidjson::Document &d)
{

  dv::check_and_set_uint(d, this->EDFrame, "EDFrame");
  if (d.HasMember("EDFrame")) { this->EDFrameHasBeenSet = true; }

  dv::check_and_set_uint(d, this->InitialModelFrame, "InitialModelFrame");
  dv::check_and_set_uint(d, this->InitialModelNumberOfFaces, "InitialModelNumberOfFaces");
  dv::check_and_set_double(d, this->InitialModelSigma, "InitialModelSigma");
  dv::check_and_set_uint(d, this->InitialModelLVClosingRadius, "InitialModelLVClosingRadius");
  dv::check_and_set_uint(d, this->InitialModelGeneralClosingRadius, "InitialModelGeneralClosingRadius");
  dv::check_and_set_bool(d, this->InitialModelPreserveEdges, "InitialModelPreserveEdges");
  dv::check_and_set_uint(d, this->BoundaryCandidateDilationRadius, "BoundaryCandidateDilationRadius");

  dv::check_and_set_uint(d, this->CurrentFrame,      "CurrentFrame");

  dv::check_and_set_bool(d, this->RegistrationUseLabels, "RegistrationUseLabels");
  dv::check_and_set_double(d, this->RegistrationWeights.Primary, "RegistrationWeights.Primary");
  dv::check_and_set_double(d, this->RegistrationWeights.EdgeWeight, "RegistrationWeights.EdgeWeight");
  dv::check_and_set_double(d, this->RegistrationWeights.Velocity, "RegistrationWeights.Velocity");
  dv::check_and_set_double(d, this->RegistrationWeights.Acceleration, "RegistrationWeights.Acceleration");
  dv::check_and_set_double(d, this->RegistrationWeights.ThinPlate, "RegistrationWeights.ThinPlate");
  dv::check_and_set_double(d, this->RegistrationWeights.TriangleAspectRatio, "RegistrationWeights.TriangleAspectRatio");
  dv::check_and_set_double(d, this->RegistrationWeights.EdgeLength, "RegistrationWeights.EdgeLength");

  this->camera.DeserializeJSON(d);

  dv::check_and_set_bool(d, this->ImagePlanesAreVisible,   "ImagePlanesAreVisible");

  dv::check_and_set_bool(d, this->CandidatesAreVisible,    "CandidatesAreVisible");
  dv::check_and_set_bool(d, this->ModelIsVisible,          "ModelIsVisible");

  if (d.HasMember("planeWidgetState.Origin") && d.HasMember("planeWidgetState.Point1") && d.HasMember("planeWidgetState.Point2")) {
    dv::check_and_set_double_array(d, this->planeWidgetState.Origin, "planeWidgetState.Origin");
    dv::check_and_set_double_array(d, this->planeWidgetState.Point1, "planeWidgetState.Point1");
    dv::check_and_set_double_array(d, this->planeWidgetState.Point2, "planeWidgetState.Point2");
    this->ImagePlaneHasBeenSet = true;
  }

  if (d.HasMember("CellDataToDisplay") && d["CellDataToDisplay"].IsInt())
      this->CellDataToDisplay = static_cast<CellData>(d["CellDataToDisplay"].GetInt());

  }

} // namespace sissr
