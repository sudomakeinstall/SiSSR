int main(int argc, char** argv) {
  return 0;
}

//// Boost
//#include <boost/program_options.hpp>
//
//namespace po = boost::program_options;
//
//// ITK
//#include <itkImage.h>
//#include <itkImageFileReader.h>
//#include <itkQuadEdgeMesh.h>
//#include <itkCuberilleImageToMeshFilter.h>
//#include <itkMeshFileWriter.h>
//
////#include <itkPointSet.h>
////#include <itkPointsLocator.h>
//
////#include <itkLoopSubdivisionSurfaceMesh.h>
////#include <itkLoopTriangleCellSubdivisionQuadEdgeMeshFilter.h>
//
//// Custom
//#include <itkCleanSegmentationImageFilter.h>
//#include <itkGenerateInitialModelImageToMeshFilter.h>
//#include <dvITKMeshToVTKPolyData.h>
//#include <dvQuickViewMultiplePolyData.h>
//
//int main(int argc, char** argv) {
//
//  // Declare the supported options.
//  po::options_description description("Allowed options");
//  description.add_options()("help", "Print usage information.")
//    ("input-image",
//     po::value<std::string>()->required(),
//    "Filename of the input image."
//    )
//    ("output-mesh",
//     po::value<std::string>()->required(),
//     "Filename of the output mesh."
//     );
//
//  po::variables_map vm;
//  po::store(po::parse_command_line(argc, argv, description), vm);
//
//  if (vm.count("help") || 1 == argc) {
//    std::cout << description << '\n';
//    return EXIT_SUCCESS;
//  }
//
//  po::notify(vm);
//
//  const std::string input_file_name(vm["input-image"].as<std::string>());
//  const std::string output_file_name(vm["output-mesh"].as<std::string>());
//
//  const double sigma = 0.1;
//  const unsigned int count = 512;
//  const unsigned int lv_radius = 10;
//  const unsigned int gn_radius = 5;
//
//  using TPixel = unsigned char;
//  const unsigned int Dimension = 3;
//  using TCoordinate = float;
////  using TPointSet = itk::PointSet<TCoordinate, Dimension>;
//  using TMesh = itk::QuadEdgeMesh<TCoordinate, Dimension>;
////  using TMesh = itk::LoopSubdivisionSurfaceMesh<TCoordinate, Dimension>;
////  using TLocator = itk::PointsLocator< TPointSet::PointsContainer >;
//
//  using TImage = itk::Image<TPixel, Dimension>;
//  using TReader = itk::ImageFileReader<TImage>;
//  using TClean = itk::CleanSegmentationImageFilter<TImage>;
//  using TCuberille = itk::CuberilleImageToMeshFilter<TImage, TMesh>;
//  using TWriter = itk::MeshFileWriter<TMesh>;
//
//  using TModel = itk::GenerateInitialModelImageToMeshFilter<TImage,TMesh>;
////  using TLoop = itk::LoopTriangleCellSubdivisionQuadEdgeMeshFilter<TMesh>;
//
//  const auto reader = TReader::New();
//  reader->SetFileName(input_file_name);
//
//  const auto clean = TClean::New();
//  clean->SetInput( reader->GetOutput() );
//
//  const auto cuberille = TCuberille::New();
//  cuberille->SetInput(clean->GetOutput());
//  cuberille->GenerateTriangleFacesOff();
//  cuberille->RemoveProblematicPixelsOn();
////  cuberille->ProjectVerticesToIsoSurfaceOn();
//  cuberille->SavePixelAsCellDataOn();
//
//  const auto writer = TWriter::New();
//  writer->SetInput( cuberille->GetOutput() );
//  writer->SetFileName( output_file_name );
//  writer->Update();
//
//
//
////  //
////  // Labeled mesh to point set map
////  //
////
////  const auto mesh = TMesh::New();
////  mesh->Graft( cuberille->GetOutput() );
////
////  std::map<size_t, TPointSet::Pointer> pointset_map;
////
////  for (auto it = mesh->GetCells()->Begin();
////       it != mesh->GetCells()->End();
////       ++it) {
////
////    const auto cell = it.Value();
////
////    typename TImage::PointType centroid;
////    centroid.SetToMidPoint(
////      mesh->GetPoint(cell->GetPointIds()[0]),
////      mesh->GetPoint(cell->GetPointIds()[2])
////      );
////
////    const auto label = mesh->GetCellData()->ElementAt( it.Index() );
////
////    itkAssertOrThrowMacro(label != 0, "Label == 0");
////
////    if (0 == pointset_map.count(label)) {
////      pointset_map[label] = TPointSet::New();
////    }
////
////    pointset_map[label]->SetPoint( it.Index(), centroid );
////
////  }
////
////  std::map<unsigned char, TLocator::Pointer> locator_map;
////  for (const auto& pointset : pointset_map) {
////    locator_map[pointset.first] = TLocator::New();
////    locator_map[pointset.first]->SetPoints( pointset.second->GetPoints() );
////    locator_map[pointset.first]->Initialize();
////  }
////
////
////
////
////
////
//  const auto model = TModel::New();
//  model->SetInput(reader->GetOutput());
//  model->SetNumberOfCellsInDecimatedMesh(count);
//  model->SetMeshNoiseSigma(sigma);
//  model->SetLVClosingRadius(lv_radius);
//  model->SetGeneralClosingRadius(gn_radius);
//  model->Update();
//
////  const auto loop0 = TLoop::New();
////  loop0->SetInput( model->GetOutput() );
////
////  const auto loop1 = TLoop::New();
////  loop1->SetInput( loop0->GetOutput() );
////
////  const auto loop2 = TLoop::New();
////  loop2->SetInput( loop1->GetOutput() );
////  loop2->Update();
////  loop2->GetOutput()->Setup();
////
////  std::pair<double, double> params(0.5, 0.5);
////
////  for (auto it = loop2->GetOutput()->GetCells()->Begin();
////      it != loop2->GetOutput()->GetCells()->End();
////      ++it) {
////    const auto model_id = it.Index();
////    const auto l = loop2->GetOutput()->GetCellData()->ElementAt(model_id);
////    const auto surface_point = loop2->GetOutput()->GetPointOnSurface(model_id, params);
////    const auto locator_id = locator_map[l]->FindClosestPoint(surface_point);
////    const auto data_point = locator_map[l]->GetPoints()->ElementAt(locator_id);
////    const auto error = (data_point - surface_point).GetNorm();
////    loop2->GetOutput()->GetCellData()->SetElement(model_id, error);
////  }
////
////
////
////
////  const auto d = dv::ITKMeshToVTKPolyData< TMesh >( cuberille->GetOutput() );
////  const auto m = dv::ITKMeshToVTKPolyData< TMesh >( model->GetOutput() );
//////  const auto l = dv::ITKMeshToVTKPolyData< TMesh >( loop2->GetOutput() );
//////
////  std::vector<vtkPolyData*> poly_data_vector;
////  poly_data_vector.emplace_back(d);
////  poly_data_vector.emplace_back(m);
//////  poly_data_vector.emplace_back(l);
////  dv::QuickViewMultiplePolyData( poly_data_vector, true );
//
//  return EXIT_SUCCESS;
//
//}
