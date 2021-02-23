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

#ifndef dv_sylvester_h_
#define dv_sylvester_h_

#include "dv_schur_decomposition.h"
#include "dv_sylvester_tri.h"

template<typename TValue>
class
dv_sylvester
{

typedef dv_schur_decomposition<TValue> TSchur;
typedef dv_sylvester_tri<TValue> TSylvester;

public:
  vnl_matrix<TValue> X;
  dv_sylvester(const vnl_matrix<TValue> &A,
                const vnl_matrix<TValue> &B,
                const vnl_matrix<TValue> &Q)
    {
    itkAssertOrThrowMacro(A.rows() == A.cols(), "");
    itkAssertOrThrowMacro(B.rows() == B.cols(), "");
    itkAssertOrThrowMacro(Q.rows() == A.rows(), "");
    itkAssertOrThrowMacro(Q.cols() == B.cols(), "");

    const TSchur schur_A(A);
    const TSchur schur_B(B.transpose());

    const auto F = schur_A.Z.transpose() * Q * schur_B.Z;

    const TSylvester sylv(schur_A.T, schur_B.T, F, 'N', 'C');

    X = schur_A.Z * sylv.X * schur_B.Z.transpose();
    };
};

#endif
