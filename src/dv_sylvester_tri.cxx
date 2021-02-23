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
// this is dv_sylvester_tri.cxx

#include <dv_lapacke.h>
#include "dv_sylvester_tri.h"

template<>
int
dv_sylvester_tri<float>
::solve(const vnl_matrix<float> &A, const vnl_matrix<float> &B)
{
  return LAPACKE_strsyl(LAPACK_COL_MAJOR,
                        TRANA,
                        TRANB,
                        ISGN,
                        M,
                        N,
                        A.transpose().data_block(),
                        M,
                        B.transpose().data_block(),
                        N,
                        X.data_block(),
                        M,
                        &SCALE);
}

template<>
int
dv_sylvester_tri<double>
::solve(const vnl_matrix<double> &A, const vnl_matrix<double> &B)
{
  return LAPACKE_dtrsyl(LAPACK_COL_MAJOR,
                        TRANA,
                        TRANB,
                        ISGN,
                        M,
                        N,
                        A.transpose().data_block(),
                        M,
                        B.transpose().data_block(),
                        N,
                        X.data_block(),
                        M,
                        &SCALE);
}
