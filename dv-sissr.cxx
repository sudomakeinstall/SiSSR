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
    ("weight-tp", po::value<double>(), "Thin plate energy weight.")
    ("weight-ac", po::value<double>(), "Acceleration weight.")
    ("weight-vc", po::value<double>(), "Velocity weight.")
    ("weight-el", po::value<double>(), "Edge length weight.")
    ("weight-ar", po::value<double>(), "Aspect ratio weight.")
    ("model-num-faces", po::value<unsigned int>(), "Model initial number of faces.")
    ("use-labels", "Use labels in registration.")
    ("ignore-labels", "Ignore labels in registration.")
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

  // QT Stuff
  vtkOpenGLRenderWindow::SetGlobalMaximumNumberOfMultiSamples(0);
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLStereoWidget::defaultFormat());

  QApplication application(argc,argv);

  sissr::Controller controller(argc, argv);
  controller.show();

  if (vm.count("model-num-faces")) {
    controller.State.InitialModelNumberOfFaces = vm["model-num-faces"].as<unsigned int>();
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
  if (vm.count("use-labels") && vm.count("ignore-labels")) {
    std::cerr << "Setting both 'use-labels' and 'ignore-labels' is disallowed." << std::endl;
    return EXIT_FAILURE;
  }
  if (vm.count("use-labels")) {
    controller.State.RegistrationUseLabels = true;
  }
  if (vm.count("ignore-labels")) {
    controller.State.RegistrationUseLabels = false;
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

  return application.exec();

}
