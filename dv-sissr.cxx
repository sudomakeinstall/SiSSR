// System
#include <iostream>

// Third Party
#include <boost/program_options.hpp>
namespace po = boost::program_options;

// Internal
#include <sissrAlgorithm.h>

int
main(int argc, char** argv)
{

  po::options_description description("Allowed options");
  description.add_options()
    ("help", "Print usage information.")
    ("candidate-dir", po::value<std::string>()->required(), "Candidate directory. Should be named 0.obj, 1.obj, etc. [required]")
    ("initial-model", po::value<std::string>()->required(), "Path to initial watertight mesh model. [required]")
    ("output-dir", po::value<std::string>()->required(), "Output directory. [required]")
    ("weight-ew", po::value<double>(), "Edge weight multipler.")
    ("weight-tp", po::value<double>(), "Thin plate energy weight.")
    ("weight-ac", po::value<double>(), "Acceleration weight.")
    ("weight-vc", po::value<double>(), "Velocity weight.")
    ("weight-el", po::value<double>(), "Edge length weight.")
    ("weight-ar", po::value<double>(), "Aspect ratio weight.")
    ("registration-use-labels", "Use labels in registration.")
    ("registration-ignore-labels", "Ignore labels in registration.")
    ("registration-sampling-density", po::value<unsigned int>(), "Samples per triangle.")
    ("max-iterations", po::value<int>(), "Maximum number of solver iterations.")
    ("max-time", po::value<int>(), "Maximum solver time in seconds.")
    ("function-tolerance", po::value<double>(), "Function tolerance for convergence.")
    ("parameter-tolerance", po::value<double>(), "Parameter tolerance for convergence.")
    ("dynamic-sparsity", "Enable dynamic sparsity in solver.")
    ("register", po::value<int>(), "Register model to candidates.");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(description).run(), vm);

  if (vm.count("help") || 1 == argc) {
    std::cout << description << '\n';
    return EXIT_SUCCESS;
  }

  po::notify(vm);

  const std::string CandidateDir(vm["candidate-dir"].as<std::string>());
  const std::string InitialModel(vm["initial-model"].as<std::string>());
  const std::string ODir(vm["output-dir"].as<std::string>());

  std::cout << "Candidate directory: " << CandidateDir << std::endl;
  std::cout << "Initial model: " << InitialModel << std::endl;
  std::cout << "Output directory: " << ODir << std::endl;

  // Initialize algorithm
  sissr::Algorithm algorithm(CandidateDir, InitialModel, ODir);

  // SiSSR Registration
  if (vm.count("weight-ew")) {
    algorithm.GetParameters().RegistrationWeights.EdgeWeight = vm["weight-ew"].as<double>();
  }
  if (vm.count("weight-tp")) {
    algorithm.GetParameters().RegistrationWeights.ThinPlate = vm["weight-tp"].as<double>();
  }
  if (vm.count("weight-ac")) {
    algorithm.GetParameters().RegistrationWeights.Acceleration = vm["weight-ac"].as<double>();
  }
  if (vm.count("weight-vc")) {
    algorithm.GetParameters().RegistrationWeights.Velocity = vm["weight-vc"].as<double>();
  }
  if (vm.count("weight-el")) {
    algorithm.GetParameters().RegistrationWeights.EdgeLength = vm["weight-el"].as<double>();
  }
  if (vm.count("weight-ar")) {
    algorithm.GetParameters().RegistrationWeights.TriangleAspectRatio = vm["weight-ar"].as<double>();
  }
  if (vm.count("registration-use-labels") && vm.count("registration-ignore-labels")) {
    std::cerr << "Setting both 'registration-use-labels' and 'registration-ignore-labels' is disallowed." << std::endl;
    return EXIT_FAILURE;
  }
  if (vm.count("registration-use-labels")) {
    algorithm.GetParameters().RegistrationUseLabels = true;
  }
  if (vm.count("registration-ignore-labels")) {
    algorithm.GetParameters().RegistrationUseLabels = false;
  }
  if (vm.count("registration-sampling-density")) {
    algorithm.GetParameters().RegistrationSamplingDensity = vm["registration-sampling-density"].as<unsigned int>();
  }

  // Solver parameters
  if (vm.count("max-iterations")) {
    algorithm.GetParameters().MaximumNumberOfIterations = vm["max-iterations"].as<int>();
  }
  if (vm.count("max-time")) {
    algorithm.GetParameters().MaximumSolverTimeInSeconds = vm["max-time"].as<int>();
  }
  if (vm.count("function-tolerance")) {
    algorithm.GetParameters().FunctionTolerance = vm["function-tolerance"].as<double>();
  }
  if (vm.count("parameter-tolerance")) {
    algorithm.GetParameters().ParameterTolerance = vm["parameter-tolerance"].as<double>();
  }
  if (vm.count("dynamic-sparsity")) {
    algorithm.GetParameters().DynamicSparsity = true;
  }


  // Misc
  if (vm.count("register")) {

    const auto requested_passes = vm["register"].as<int>();

    while (algorithm.GetDirectoryStructure().NumberOfRegistrationPasses() < requested_passes) {
      algorithm.Register();
    }

  }

  return EXIT_SUCCESS;
}
