Dependencies:
- [ITK](https://github.com/insightsoftwareconsortium/itk)
    - External Module: ITKDVUtilities
    - Remote Modules:
        - [ITKSubdivision](https://github.com/InsightSoftwareConsortium/itkSubdivisionQuadEdgeMeshFilter)
        - [IOSTL](https://github.com/InsightSoftwareConsortium/ITKSTLMeshIO)

    $ ccmake ../src \
        -DCMAKE_CXX_FLAGS=-std=c++11 \
        -DCMAKE_BUILD_TYPE=Debug \
        -DModule_IOSTL=ON \
        -DModule_ITKVtkGlue=ON

- [VTK](https://github.com/kitware/vtk)

    $ ccmake ../src \
        -DCMAKE_CXX_FLAGS=-std=c++11 \
        -DCMAKE_BUILD_TYPE=Debug \
        -DVTK_Group_Qt=ON

- [RapidJSON](https://github.com/miloyip/rapidjson)
- [Ceres](https://github.com/ceres-solver/ceres-solver)
- [Qt](https://www.qt.io/)

