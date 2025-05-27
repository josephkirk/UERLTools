#pragma once

#include "CoreMinimal.h"
// RL Tools includes for Matrix type
#include "rl_tools/containers/matrix/matrix.h" // For rl_tools::Matrix and its operations

#include "RLConfigTypes.h" // For FRLNormalizationParams

// struct FRLNormalizationParams; // No longer needed as it's included from RLConfigTypes.h

namespace RLToolsConversionUtils
{
    /**
     * Converts a TArray<float> from Unreal Engine to an rl_tools::Matrix.
     *
     * @tparam T_MATRIX_SPEC The specification of the rl_tools Matrix.
     * @param UEArray The source TArray<float>.
     * @param RLMatrix The destination rl_tools::Matrix. It must be pre-allocated with correct dimensions.
     * @param bNormalize Whether to apply normalization (requires NormalizationParams).
     * @param NormalizationParams Parameters for normalization (e.g., mean, stddev).
     * @return True if conversion was successful, false otherwise (e.g., dimension mismatch).
     */
    template <typename T_MATRIX_SPEC>
    bool UEArrayToRLMatrix(
        const TArray<float>& UEArray,
        rl_tools::Matrix<T_MATRIX_SPEC>& RLMatrix,
        const FRLNormalizationParams& NormalizationParams
    );

    /**
     * Converts an rl_tools::Matrix to a TArray<float> for Unreal Engine.
     *
     * @tparam T_MATRIX_SPEC The specification of the rl_tools Matrix.
     * @param RLMatrix The source rl_tools::Matrix.
     * @param UEArray The destination TArray<float>. It will be resized accordingly.
     * @param bDenormalize Whether to apply denormalization (requires NormalizationParams).
     * @param NormalizationParams Parameters for denormalization.
     * @return True if conversion was successful, false otherwise.
     */
    template <typename T_MATRIX_SPEC>
    bool RLMatrixToUEArray(
        const rl_tools::Matrix<T_MATRIX_SPEC>& RLMatrix,
        TArray<float>& UEArray,
        const FRLNormalizationParams& DenormalizationParams
    );

    // TODO: Add functions for normalization/denormalization if they are complex
    // and not handled directly within the conversion functions.
    // For example:
    // template <typename T_MATRIX_SPEC>
    // void NormalizeMatrix(rl_tools::Matrix<T_MATRIX_SPEC>& Matrix, const FRLNormalizationParams& Params);
    //
    // template <typename T_MATRIX_SPEC>
    // void DenormalizeMatrix(rl_tools::Matrix<T_MATRIX_SPEC>& Matrix, const FRLNormalizationParams& Params);
}
