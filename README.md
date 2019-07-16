# Simultaneous Subdivision Surface Registration (SiSSR)

This repository provides an implementation of the Simultaneous Subdivision Surface Registration (SiSSR) algorithm.  The algorithm is detailed in a 2018 article in Medical Image Analysis [1].  Please cite this article in any work making use of this code.

[1] https://www.ncbi.nlm.nih.gov/pubmed/29627686

# Dependencies

- [Eigen3](http://eigen.tuxfamily.org)
- [RapidJSON](https://github.com/miloyip/rapidjson)
- [Ceres](https://github.com/ceres-solver/ceres-solver)
- [Qt](https://www.qt.io/)

Dependencies:
- [ITK](https://github.com/insightsoftwareconsortium/itk)
    - External Module: ITKDVUtilities
    - Remote Modules:
        - [ITKSubdivision](https://github.com/InsightSoftwareConsortium/itkSubdivisionQuadEdgeMeshFilter)
        - [ITKIOSTL](https://github.com/InsightSoftwareConsortium/ITKSTLMeshIO)
        - [ITKMeshNoise](https://github.com/InsightSoftwareConsortium/ITKMeshNoise)

```bash
ccmake ../src \
  -DBUILD_TESTING=OFF \
  -DCMAKE_CXX_FLAGS=-std=c++11 \
  -DModule_IOMeshSTL=ON \
  -DModule_MeshNoise=ON \
  -DModule_SubdivisionQuadEdgeMeshFilter=ON \
  -DModule_ITKVtkGlue=ON
```

- [VTK](https://github.com/kitware/vtk)

```bash
ccmake ../src \
  -DBUILD_SHARED_LIBS=OFF \
  -DCMAKE_CXX_FLAGS=-std=c++11 \
  -DVTK_BUILD_TESTING=OFF \
  -DVTK_GROUP_ENABLE_Qt=YES
```

