// STL
#include <iostream>

// Boost
#include <boost/program_options.hpp>
namespace po = boost::program_options;

// Qt
#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLStereoWidget.h>

// VTK
#include <vtkOpenGLRenderWindow.h>

// SiSSR
#include <sissrController.h>

int
main(int argc, char** argv)
{

  // Declare the supported options.
  po::options_description description("Allowed options");
  description.add_options()
    ("help", "Print usage information.")
    ("input-dir", po::value<std::string>()->required(), "Input directory.")
    ("output-dir", po::value<std::string>()->required(), "Output directory.")

    // Initial Model
    ("model-frame", po::value<unsigned int>(), "Frame from which to generate the template mesh.")
    ("model-num-faces", po::value<unsigned int>(), "Model initial number of faces.")
    ("model-use-labels", "Use labels in initial model (should not be used with --model-ignore-labels).")
    ("model-ignore-labels", "Ignore labels in initial model (should not be used with --model-use-labels).")
    ("model-midpoint", "Use the midpoint decimation algorithm (should not be used with --model-lindstromturk).")
    ("model-lindstromturk", "Use the Lindstrom Turk decimation algorithm (should not be used with --model-midpoint).")

    // SiSSR Registration
    ("weight-ew", po::value<double>(), "Edge weight multipler.")
    ("weight-tp", po::value<double>(), "Thin plate energy weight.")
    ("weight-ac", po::value<double>(), "Acceleration weight.")
    ("weight-vc", po::value<double>(), "Velocity weight.")
    ("weight-el", po::value<double>(), "Edge length weight.")
    ("weight-ar", po::value<double>(), "Aspect ratio weight.")
    ("registration-use-labels", "Use labels in registration.")
    ("registration-ignore-labels", "Ignore labels in registration.")

    // Misc
    ("ed-frame", po::value<unsigned int>(), "ED frame.")
    ("candidates", "Calculate boundary candidates.")
    ("model", "Calculate initial model.")
    ("register", "Register mesh to candidates.")
    ("residuals", "Calculate residuals.")
    ("reset-camera", "Reset the camera based on visible actors.")
    ("quit", "Quit the application.");

  po::positional_options_description positional;
  positional.add("input-dir", 1).add("output-dir", 1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(description).positional(positional).run(), vm);

  if (vm.count("help") || 1 == argc) {
    std::cout << description << '\n';
    return EXIT_SUCCESS;
  }

  po::notify(vm);

  const std::string IDir(vm["input-dir"].as<std::string>());
  const std::string ODir(vm["output-dir"].as<std::string>());

  std::cout << "Input directory: " << IDir << std::endl;
  std::cout << "Output directory: " << ODir << std::endl;

  // Qt Stuff
  vtkOpenGLRenderWindow::SetGlobalMaximumNumberOfMultiSamples(0);
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLStereoWidget::defaultFormat());

  QApplication application(argc,argv);

  sissr::Controller controller(argc, argv);

  // Model
  if (vm.count("model-frame")) {
    controller.State.InitialModelFrame = vm["model-frame"].as<unsigned int>();
  }
  if (vm.count("model-num-faces")) {
    controller.State.InitialModelNumberOfFaces = vm["model-num-faces"].as<unsigned int>();
  }
  if (vm.count("model-use-labels") && vm.count("model-ignore-labels")) {
    std::cerr << "Setting both 'model-use-labels' and 'model-ignore-labels' is disallowed." << std::endl;
    return EXIT_FAILURE;
  }
  if (vm.count("model-use-labels")) {
    controller.State.InitialModelPreserveEdges = true;
  }
  if (vm.count("model-ignore-labels")) {
    controller.State.InitialModelPreserveEdges = false;
  }
  if (vm.count("model-midpoint") && vm.count("model-lindstromturk")) {
    std::cerr << "Setting both 'model-midpoint' and 'model-lindstromturk' is disallowed." << std::endl;
    return EXIT_FAILURE;
  }
  if (vm.count("model-midpoint")) {
    controller.State.InitialModelDecimationTechnique = sissr::DecimationTechnique::Midpoint;
  }
  if (vm.count("model-lindstromturk")) {
    controller.State.InitialModelDecimationTechnique = sissr::DecimationTechnique::LindstromTurk;
  }

  // SiSSR Registration
  if (vm.count("weight-ew")) {
    controller.State.RegistrationWeights.EdgeWeight = vm["weight-ew"].as<double>();
  }
  if (vm.count("weight-tp")) {
    controller.State.RegistrationWeights.ThinPlate = vm["weight-tp"].as<double>();
  }
  if (vm.count("weight-ac")) {
    controller.State.RegistrationWeights.Acceleration = vm["weight-ac"].as<double>();
  }
  if (vm.count("weight-vc")) {
    controller.State.RegistrationWeights.Velocity = vm["weight-vc"].as<double>();
  }
  if (vm.count("weight-el")) {
    controller.State.RegistrationWeights.EdgeLength = vm["weight-el"].as<double>();
  }
  if (vm.count("weight-ar")) {
    controller.State.RegistrationWeights.TriangleAspectRatio = vm["weight-ar"].as<double>();
  }
  if (vm.count("registration-use-labels") && vm.count("registration-ignore-labels")) {
    std::cerr << "Setting both 'registration-use-labels' and 'registration-ignore-labels' is disallowed." << std::endl;
    return EXIT_FAILURE;
  }
  if (vm.count("registration-use-labels")) {
    controller.State.RegistrationUseLabels = true;
  }
  if (vm.count("registration-ignore-labels")) {
    controller.State.RegistrationUseLabels = false;
  }

  // Misc
  if (vm.count("ed-frame")) {
    controller.State.EDFrame = vm["ed-frame"].as<unsigned int>();
  }
  if (vm.count("candidates")) {
    controller.CalculateBoundaryCandidates();
  }
  if (vm.count("model")) {
    controller.GenerateInitialModel();
  }
  if (vm.count("register")) {
    controller.Register();
  }
  if (vm.count("residuals")) {
    controller.CalculateResiduals();
  }
  if (vm.count("reset-camera")) {
    controller.ResetCamera();
  }
  if (vm.count("quit")) {
      application.quit();
      return EXIT_SUCCESS;
  }

  controller.show();

  return application.exec();

}
