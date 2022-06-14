//==============================================================================
//
//  @@-COPYRIGHT-START-@@
//
//  Copyright (c) 2019-2022, Qualcomm Innovation Center, Inc. All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//  3. Neither the name of the copyright holder nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  SPDX-License-Identifier: BSD-3-Clause
//
//  @@-COPYRIGHT-END-@@
//
//==============================================================================

#include <cstddef>
#include <cassert>
#include <vector>

#include "DlQuantization/Quantization.hpp"
#include "math_functions.hpp"
#include "quantization_utils.hpp"

#include "TfEncodingAnalyzer.h"

namespace DlQuantization
{

template <typename DTYPE>
std::vector<std::tuple<double, double>> TfEncodingAnalyzer<DTYPE>::getStatsHistogram() const
{
    // No real histogram data is kept for TF Encoding analyzer
    assert(0);
}

template <typename DTYPE>
void TfEncodingAnalyzer<DTYPE>::updateStats(const DTYPE* tensor, const size_t tensorSize,
                                            ComputationMode tensorCpuGpuMode)
{
    this->_statsUpdated = true;
    // Compute stats for the tensor being passed in
    auto currentMin  = (double) GetMin(tensor, tensorSize, tensorCpuGpuMode);
    auto currentMax  = (double) GetMax(tensor, tensorSize, tensorCpuGpuMode);

    // Update accumulated stats
    _accumulatedStats.min = std::min(_accumulatedStats.min, currentMin);
    _accumulatedStats.max = std::max(_accumulatedStats.max, currentMax);
}

template <typename DTYPE>
TfEncoding TfEncodingAnalyzer<DTYPE>::computeEncoding(uint8_t bw, bool useSymmetricEncodings,
                                                      bool useStrictSymmetric, bool useUnsignedSymmetric) const
{
    // If symmetric encodings are requested then strictSymmetric and unsignedSymmetric are exclusive modes
    if (useSymmetricEncodings)
        assert(!(useStrictSymmetric && useUnsignedSymmetric));

    TfEncoding encoding;

    // Make sure zero value is within the range
    double newMin  = std::min(0.0, _accumulatedStats.min);
    double newMax  = std::max(0.0, _accumulatedStats.max);

    // When the min and max are too close together, nudge the maximum to meet the
    // minimum range requirement
    // This also handles the case where min==max==0 to avoid division by zero
    newMax       = std::max(newMax, newMin + MIN_RANGE);
    encoding.bw  = bw;

    return getComputedEncodings(bw, newMin, newMax, useSymmetricEncodings, useStrictSymmetric, useUnsignedSymmetric);
}


// Explicit instantiations
template class TfEncodingAnalyzer<double>;

template class TfEncodingAnalyzer<float>;

}