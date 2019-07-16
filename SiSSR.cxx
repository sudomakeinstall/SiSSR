// STL
#include <iostream>

// Qt
#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLWidget.h>
#include <vtkOpenGLRenderWindow.h>

// Custom
#include <dvSubdivisionRegistrationController.h>

int
main(int argc, char** argv)
{

  if (argc < 3)
    {
    std::cerr << "Basic Usage:\n"
              << "  " << argv[0] << " <InputDirectory> <OutputDirectory>\n\n"
              << "File Structure:\n"
              << "  <InputDirectory>\n"
              << "    /candidates/*.obj\n"
              << "    /img-nii/*.nii.gz\n"
              << "  <OutputDirectory>\n"
              << "    /initial_models/initial_model.obj\n"
              << "                   /initial_smoothed_model.obj\n"
              << "                   /initial_subdivided_model.obj\n"
              << "    /registered_models/\n"
              << "    /residuals_models/\n"
              << "    /screenshots/\n"
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
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLWidget::defaultFormat());

  QApplication application(argc,argv);

  dv::SubdivisionRegistrationController controller(argc, argv);
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

