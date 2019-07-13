Dependencies:
- [ITK][https://github.com/insightsoftwareconsortium/itk]
    - External Module: ITKDVUtilities
    - Remote Modules: [ITKSubdivision][https://github.com/InsightSoftwareConsortium/itkSubdivisionQuadEdgeMeshFilter], [IOSTL][https://github.com/InsightSoftwareConsortium/ITKSTLMeshIO]

    $ ccmake ../src \
        -DCMAKE_CXX_FLAGS=-std=c++11 \
        -DCMAKE_BUILD_TYPE=Debug \
        -DITK_DOXYGEN_HTML=OFF \
        -DModule_IOSTL=ON \
        -DModule_ITKVtkGlue=ON \
        -DVTK_DIR=~/Developer/VTK/bin

- [VTK][https://github.com/kitware/vtk]

    $ ccmake ../src \
        -DCMAKE_CXX_FLAGS=-std=c++11 \
        -DCMAKE_BUILD_TYPE=Debug \
        -DVTK_Group_Qt=ON \
        -DVTK_USE_CXX11_FEATURES=ON

- [RapidJSON][https://github.com/miloyip/rapidjson]
- [Ceres][https://github.com/ceres-solver/ceres-solver]
- [Qt][https://www.qt.io/]

