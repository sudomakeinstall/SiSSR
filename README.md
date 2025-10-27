# Simultaneous Subdivision Surface Registration (SiSSR)

## Background

This repository provides the reference implementation of the [Simultaneous Subdivision Surface Registration (SiSSR)](https://www.ncbi.nlm.nih.gov/pubmed/29627686) algorithm by Vigneault, et al.:

> Vigneault DM, Pourmorteza A, Thomas ML, Bluemke DA, Noble JA. SiSSR: Simultaneous subdivision surface registration for the quantification of cardiac function from computed tomography in canines. Med Image Anal. 2018 May;46:215-228. doi: 10.1016/j.media.2018.03.009. Epub 2018 Mar 29. PMID: 29627686; PMCID: PMC5942600.

SiSSR is a mesh to point-set registration algorithm which is designed to work with time series data.
The inputs to the algorithm are (a) a sequence of "candidate" meshes describing the surface of interest at successive time points and (b) an "initial model" in the form of a Loop subdivision surface, approximating the same surface of interest.
SiSSR registers the initial model (or, more correctly, registers a sequence of initial models equal in length to the number of time points) to the sequence of candidate meshes.
The candidate meshes are in fact treated as point clouds, so there are no specific requirements on their connectivity.
The template mesh is treated as a Loop subdivision surface and so has several requirements:

1. Must be watertight and manifold. In most cases, this can be achieved using a meshing algorithm such as Poisson Surface Reconstruction on one of the candidate meshes.
2. The total number of triangles should be relatively small (usually <1000 triangles) in order to fit into memory and keep the registration process relatively fast. This can generally be achieved by decimating the result of your meshing algorithm.
2. All vertices must have valence >3. This can generally be achieved by iteratively replacing the three triangles surrounding a valence three vertex with a single triangle until all have been removed.
3. Any edge can be connected to at most one "extraordinary" vertex (where an "ordinary" vertex has valence 6). This can generally be achieved by applying a single Loop subdivision surface iteration to your input mesh.

## Installation

### Docker

The recommended installation method is using [Podman](https://podman.io/) (our preference, particularly on machines where sudo is not available) or [Docker](https://www.docker.com/), which handles all dependencies automatically and ensures consistent behavior across platforms.

To build the container image, ensure Podman or Docker is installed on your system, then use the provided helper script:

```bash
./docker-helper.sh build
```

### Build from Source

For advanced users who prefer to build from source, the primary dependencies include:

- [ITK](https://github.com/InsightSoftwareConsortium/ITK/)
- [Ceres Solver](https://github.com/ceres-solver/ceres-solver)
- [Eigen](https://eigen.tuxfamily.org/index.php?title=Main_Page)
- [SuiteSparse](https://github.com/DrTimothyAldenDavis/SuiteSparse)
- [LAPACKE](https://github.com/Reference-LAPACK/lapack)
- [Boost Program Options](https://www.boost.org/doc/libs/latest/doc/html/program_options.html)
- [RapidJSON](https://github.com/Tencent/rapidjson/)
- [Google Glog](https://github.com/google/glog)
- [GFlags](https://github.com/gflags/gflags)
- Standard build tools (CMake, Ninja or Make, C++ compiler)

Refer to the Dockerfile for detailed build instructions and specific dependency versions.

## Usage

### Running `dv-sissr` Directly

If you have `dv-sissr` installed on your system and in your path, basic usage is as follows:

```bash
dv-sissr \
    --candidate-dir ./path/to/candidate-dir/ \
    --initial-model ./path/to/initial-model.obj \
    --output-dir    ./path/to/output-dir/
```

Additional options are documented in `dv-sissr --help`:

```bash
Allowed options:
  --help                              Print usage information.
  --candidate-dir arg                 Candidate directory. Should be named 
                                      0.obj, 1.obj, etc. [required]
  --initial-model arg                 Path to initial watertight mesh model. 
                                      [required]
  --output-dir arg                    Output directory. [required]
  --weight-ew arg                     Edge weight multiplier.
  --weight-tp arg                     Thin plate energy weight.
  --weight-ac arg                     Acceleration weight.
  --weight-vc arg                     Velocity weight.
  --weight-el arg                     Edge length weight.
  --weight-ar arg                     Aspect ratio weight.
  --registration-use-labels           Use labels in registration.
  --registration-ignore-labels        Ignore labels in registration.
  --registration-sampling-density arg Samples per triangle.
  --max-iterations arg                Maximum number of solver iterations.
  --max-time arg                      Maximum solver time in seconds.
  --function-tolerance arg            Function tolerance for convergence.
  --parameter-tolerance arg           Parameter tolerance for convergence.
  --dynamic-sparsity                  Enable dynamic sparsity in solver.
  --register arg                      Register model to candidates.
```

### Running `dv-sissr` Using Podman/Docker

If you built the container image following the installation instructions, use the helper script to run `dv-sissr`:

```bash
./docker-helper.sh run <candidate-dir> <initial-model> <output-dir> [additional args...]
```

For example:

```bash
./docker-helper.sh run \
    ~/datasets/candidates \
    ~/datasets/initial-model.obj \
    ~/output \
    --max-iterations 100
```

The helper script automatically handles volume mounting and ensures files in the output directory are owned by your user. Any additional arguments are passed directly to `dv-sissr`.
