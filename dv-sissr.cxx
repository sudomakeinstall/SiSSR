// STL
#include <iostream>

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

  if (argc < 3)
    {
    std::cerr << "Basic Usage:\n"
              << "  " << argv[0] << " <InputDirectory> <OutputDirectory>\n\n"
              << "File Structure:\n"
              << "  <InputDirectory>\n"
              << "    /img-nii/*.nii.gz\n"
              << "    /seg-nii/*.nii.gz\n"
              << "  <OutputDirectory>\n"
              << "    /candidates/*.txt\n"
              << "    /initial_models/initial_model.vtk\n"
              << "    /registered_models/<Pass>/*.vtk\n"
              << "    /residuals_models/<Pass>/*.vtk\n"
              << "    /screenshots/*.png\n"
              << "    /serialization/parameters.json\n\n"
              << "Optional arguments:\n"
              << "  --segment    : Calculate segment IDs.\n"
              << "  --areas      : Calculate surface areas.\n"
              << "  --candidates : Calculate Boundary Candidates.\n"
              << "  --register   : Register Mesh To Candidates.\n"
              << "  --quit       : Quit Application When Complete.\n" << std::flush;
    return EXIT_FAILURE;
    }

  std::cout << "Input directory: " << argv[1] << std::endl;
  std::cout << "Output directory: " << argv[2] << std::endl;

  // QT Stuff
  vtkOpenGLRenderWindow::SetGlobalMaximumNumberOfMultiSamples(0);
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLStereoWidget::defaultFormat());

  QApplication application(argc,argv);

  sissr::Controller controller(argc, argv);
  controller.show();

  for (int i = 3; i < argc; ++i)
    {
    if (std::string(argv[i]) == std::string("--quit"))
      {
      application.quit();
      return EXIT_SUCCESS;
      }
    }

  return application.exec();

}

