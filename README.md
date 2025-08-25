# Simultaneous Subdivision Surface Registration (SiSSR)

This repository provides an implementation of the Simultaneous Subdivision Surface Registration (SiSSR) algorithm.  The algorithm is detailed in a 2018 article in Medical Image Analysis [1].  Please cite this article in any work making use of this code.

[1] https://www.ncbi.nlm.nih.gov/pubmed/29627686

```bash
dv-sissr --help

Allowed options:
  --help                              Print usage information.
  --candidate-dir arg                 Candidate directory. Should be named
                                      0.obj, 1.obj, etc.
  --initial-model arg                 Path to initial watertight mesh model.
  --output-dir arg                    Output directory.
  --weight-ew arg                     Edge weight multipler.
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
  --register                          Register model to candidates.
```
