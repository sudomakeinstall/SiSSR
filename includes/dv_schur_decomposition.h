/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
// This is core/vnl/algo/dv_schur_decomposition.h
#ifndef dv_schur_decomposition_h_
#define dv_schur_decomposition_h_

//:
// \file
// \brief Perform the Schur decomposition of a square matrix using LAPACK.
// \author Davis Vigneault
//

#include <vnl/vnl_matrix.h>
#include <vnl/vnl_vector.hxx>
#include <vnl/vnl_c_vector.hxx>

#include <vector>

//: Schur decomposition of a square matrix.
//  The Schur decomposition decomposes a square matrix A as follows:
//
//    A = Z * T * Z^H
//
//  Here, Z^H represents the conjuate transpose of Z (or simply the
//  transpose, if Z is real).
//
//  In the *real* Schur decomposition:
//    Z: An invertible matrix.
//    T: A quasi-upper triangular matrix.
//
//  In the *complex* Schur decomposition:
//    Z: A unitary matrix.
//    T: An upper triangular matrix.
//

//// http://physics.oregonstate.edu/~landaur/nacphy/lapack/routines/sgees.html
//extern "C" void sgees_(const char* JOBVS,
//                       const char* SORT,
//                       bool(*SELECT)(float*,float*),
//                       const int* N,
//                       float* A,
//                       const int* LDA,
//                       int* SDIM,
//                       float* WR,
//                       float* WI,
//                       float* VS,
//                       const int* LDVS,
//                       float* WORK,
//                       const int* LWORK,
//                       bool* BWORK,
//                       int* INFO);
//
//// http://physics.oregonstate.edu/~landaur/nacphy/lapack/routines/dgees.html
//extern "C" void dgees_(const char* JOBVS,
//                       const char* SORT,
//                       bool(*SELECT)(double*,double*),
//                       const int* N,
//                       double* A,
//                       const int* LDA,
//                       int* SDIM,
//                       double* WR,
//                       double* WI,
//                       double* VS,
//                       const int* LDVS,
//                       double* WORK,
//                       const int* LWORK,
//                       bool* BWORK,
//                       int* INFO);

template<typename TValue>
class dv_schur_decomposition
{

  char JOBVS = 'V';
  char SORT = 'N';
  int(*SELECT)(const TValue*,const TValue*) = nullptr;
  int SDIM = -1;
  int INFO = 0;

public:

  //: The constructor takes the symmetric matrix to be composed.
  dv_schur_decomposition(const vnl_matrix<TValue> &A) :
    T(A.transpose()),
    Z(A.rows(),A.cols(),0.f),
    N(A.rows()),
    LWORK(3*N),
    WR(N*N),
    WI(N*N),
    BWORK(SORT == 'N' ? 0 : N)
    {
    itkAssertOrThrowMacro(A.rows() == A.cols(), "");

    itkAssertOrThrowMacro(0 == this->solve(), "");

    T.inplace_transpose();
    Z.inplace_transpose();
    };

  //: An upper triangular (complex) or quasi upper triangular (real) matrix.
  vnl_matrix<TValue> T;

  //: A unitary (complex) or simply invertible (real) matrix.
  vnl_matrix<TValue> Z;

private:

  int N;
  int LWORK;
  std::vector<TValue> WR, WI;
  vnl_vector<bool> BWORK;

  int solve();

};

#endif
