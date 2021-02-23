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
// This is core/vnl/algo/dv_sylvester_tri.h
#ifndef dv_sylvester_tri_h_
#define dv_sylvester_tri_h_

//:
// \file
// \brief Solve the Sylvester equation using LAPACK for a constrained set of input matrices.
// \author Davis Vigneault
//

#include <vnl/vnl_matrix.h>
//: Solve the Sylvester equation using LAPACK for a constrained set of input matrices.
//  The Sylvester equation is as follows:
//
//    A*X + X*B = C
//
//    A : (M, M)
//    B : (N, N)
//    C : (M, N)
//    X : (M, N)
//
//  LAPACK provides a series of subroutines (*trsyl_) which solve this equation
//  given the constraint that A and B be in Schur canonical form.  This class
//  may be used directly if the input matrices are already in this form; otherwise,
//  for a general matrix, please consider see dv_sylvester.h.
//

//// http://physics.oregonstate.edu/~landaur/nacphy/lapack/routines/strsyl.html
//extern "C" void strsyl_(const char* TRANA,
//                        const char* TRANB,
//                        const int* ISGN,
//                        const int* M,
//                        const int* N,
//                        const float* A,
//                        const int* LDA,
//                        const float* B,
//                        const int* LDB,
//                        float* C,
//                        const int* LDC,
//                        const float* SCALE,
//                        int* INFO);
//
//// http://physics.oregonstate.edu/~landaur/nacphy/lapack/routines/dtrsyl.html
//extern "C" void dtrsyl_(const char* TRANA,
//                        const char* TRANB,
//                        const int* ISGN,
//                        const int* M,
//                        const int* N,
//                        const double* A,
//                        const int* LDA,
//                        const double* B,
//                        const int* LDB,
//                        double* C,
//                        const int* LDC,
//                        const double* SCALE,
//                        int* INFO);

template<typename TReal>
class dv_sylvester_tri
{
public:
  dv_sylvester_tri(const vnl_matrix<TReal> &A,
                const vnl_matrix<TReal> &B,
                const vnl_matrix<TReal> &Q,
                char _TRANA = 'N', char _TRANB = 'N') :
    M(A.rows()),
    N(B.rows()),
    X(Q),
    TRANA(_TRANA),
    TRANB(_TRANB)
    {
    itkAssertOrThrowMacro(A.rows() == A.cols(), "");
    itkAssertOrThrowMacro(B.rows() == B.cols(), "");
    itkAssertOrThrowMacro(Q.rows() == A.rows(), "");
    itkAssertOrThrowMacro(Q.cols() == B.cols(), "");

    X.inplace_transpose();

    itkAssertOrThrowMacro(this->solve(A, B) >= 0, "");

    X.inplace_transpose();
    };

  int M;
  int N;
  vnl_matrix<TReal> X;

  char TRANA;
  char TRANB;
  int ISGN = +1;
  TReal SCALE = 1.f;
  int INFO = 0;

private:

  int solve(const vnl_matrix<TReal> &A, const vnl_matrix<TReal> &B);

};

#endif
