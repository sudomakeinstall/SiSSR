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
// This is dv_schur_decomposition.cxx

#include "dv_lapacke.h"
#include "dv_schur_decomposition.h"

template<>
int
dv_schur_decomposition<float>
::solve()
{
  std::vector<float> WORK(LWORK);

  return LAPACKE_sgees(LAPACK_COL_MAJOR,
                       JOBVS,
                       SORT,
                       SELECT,
                       N,
                       T.data_block(),
                       N,
                       &SDIM,
                       WR.data(),
                       WI.data(),
                       Z.data_block(),
                       N);
}

template<>
int
dv_schur_decomposition<double>
::solve()
{
  std::vector<double> WORK(LWORK);

  return LAPACKE_dgees(LAPACK_COL_MAJOR,
                       JOBVS,
                       SORT,
                       SELECT,
                       N,
                       T.data_block(),
                       N,
                       &SDIM,
                       WR.data(),
                       WI.data(),
                       Z.data_block(),
                       N);
}
