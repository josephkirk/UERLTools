#include "RLToolsConversionUtils.h"
#include "RLConfigTypes.h" // Include for FRLNormalizationParams
#include "Logging/LogMacros.h"
#include "Math/UnrealMathUtility.h" // For FMath::Abs and KINDA_SMALL_NUMBER

// Fallback log category
#ifndef LOG_UERLTOOLS
#define LOG_UERLTOOLS LogTemp
#else
#define LOG_UERLTOOLS LogUERLTools // Assuming you have a LogUERLTools defined elsewhere
#endif

namespace RLToolsConversionUtils
{
    template <typename T_MATRIX_SPEC>
    bool UEArrayToRLMatrix(
        const TArray<float>& UEArray,
        rl_tools::Matrix<T_MATRIX_SPEC>& RLMatrix,
        const FRLNormalizationParams& NormalizationParams)
    {
        constexpr auto ROWS = rl_tools::rows(RLMatrix);
        constexpr auto COLS = rl_tools::cols(RLMatrix);
        const int32 ExpectedNumElements = ROWS * COLS;

        if (UEArray.Num() != ExpectedNumElements)
        {
            UE_LOG(LOG_UERLTOOLS, Error, TEXT("UEArrayToRLMatrix: Dimension mismatch. UEArray has %d elements, RLMatrix expects %d."), UEArray.Num(), ExpectedNumElements);
            return false;
        }

        for (int32 i = 0; i < ExpectedNumElements; ++i)
        {
            float ValueToSet = UEArray[i];

            if (NormalizationParams.bIsEnabled)
            {
                float Mean = 0.0f;
                if (NormalizationParams.Mean.Num() == 1)
                {
                    Mean = NormalizationParams.Mean[0];
                }
                else if (i < NormalizationParams.Mean.Num())
                {
                    Mean = NormalizationParams.Mean[i];
                }
                else if (NormalizationParams.Mean.Num() > 0) // Not empty, not 1, and index is out of bounds
                {
                     UE_LOG(LOG_UERLTOOLS, Warning, TEXT("UEArrayToRLMatrix: Normalization Mean array size (%d) is insufficient for element index %d (total elements %d). Using default mean 0."), NormalizationParams.Mean.Num(), i, ExpectedNumElements);
                }


                float StdDev = 1.0f;
                if (NormalizationParams.StdDev.Num() == 1)
                {
                    StdDev = NormalizationParams.StdDev[0];
                }
                else if (i < NormalizationParams.StdDev.Num())
                {
                    StdDev = NormalizationParams.StdDev[i];
                }
                else if (NormalizationParams.StdDev.Num() > 0) // Not empty, not 1, and index is out of bounds
                {
                     UE_LOG(LOG_UERLTOOLS, Warning, TEXT("UEArrayToRLMatrix: Normalization StdDev array size (%d) is insufficient for element index %d (total elements %d). Using default stddev 1."), NormalizationParams.StdDev.Num(), i, ExpectedNumElements);
                }

                if (FMath::Abs(StdDev) < KINDA_SMALL_NUMBER)
                {
                    UE_LOG(LOG_UERLTOOLS, Warning, TEXT("UEArrayToRLMatrix: Normalization StdDev is near zero for element %d. Skipping normalization for this element."), i);
                    // ValueToSet remains UEArray[i] if we skip normalization due to zero StdDev
                }
                else
                {
                    ValueToSet = (UEArray[i] - Mean) / StdDev;
                }
            }

            if constexpr (ROWS == 1) {
                 rl_tools::set(RLMatrix, 0, i, static_cast<typename T_MATRIX_SPEC::T>(ValueToSet));
            }
            else if constexpr (COLS == 1) {
                 rl_tools::set(RLMatrix, i, 0, static_cast<typename T_MATRIX_SPEC::T>(ValueToSet));
            }
            else {
                int32 Row = i / COLS;
                int32 Col = i % COLS;
                if (Row < ROWS && Col < COLS) {
                     rl_tools::set(RLMatrix, Row, Col, static_cast<typename T_MATRIX_SPEC::T>(ValueToSet));
                } else {
                    UE_LOG(LOG_UERLTOOLS, Error, TEXT("UEArrayToRLMatrix: Index out of bounds during 2D mapping. This should not happen."));
                    return false;
                }
            }
        }
        return true;
    }

    template <typename T_MATRIX_SPEC>
    bool RLMatrixToUEArray(
        const rl_tools::Matrix<T_MATRIX_SPEC>& RLMatrix,
        TArray<float>& UEArray,
        const FRLNormalizationParams& DenormalizationParams)
    {
        constexpr auto ROWS = rl_tools::rows(RLMatrix);
        constexpr auto COLS = rl_tools::cols(RLMatrix);
        const int32 NumElements = ROWS * COLS;

        UEArray.SetNumUninitialized(NumElements);

        for (int32 i = 0; i < NumElements; ++i)
        {
            float ValueFromMatrix;
            if constexpr (ROWS == 1) {
                ValueFromMatrix = static_cast<float>(rl_tools::get(RLMatrix, 0, i));
            }
            else if constexpr (COLS == 1) {
                ValueFromMatrix = static_cast<float>(rl_tools::get(RLMatrix, i, 0));
            }
            else {
                int32 Row = i / COLS;
                int32 Col = i % COLS;
                ValueFromMatrix = static_cast<float>(rl_tools::get(RLMatrix, Row, Col));
            }

            if (DenormalizationParams.bIsEnabled)
            {
                float Mean = 0.0f;
                 if (DenormalizationParams.Mean.Num() == 1)
                {
                    Mean = DenormalizationParams.Mean[0];
                }
                else if (i < DenormalizationParams.Mean.Num())
                {
                    Mean = DenormalizationParams.Mean[i];
                }
                else if (DenormalizationParams.Mean.Num() > 0)
                {
                     UE_LOG(LOG_UERLTOOLS, Warning, TEXT("RLMatrixToUEArray: Denormalization Mean array size (%d) is insufficient for element index %d (total elements %d). Using default mean 0."), DenormalizationParams.Mean.Num(), i, NumElements);
                }

                float StdDev = 1.0f;
                if (DenormalizationParams.StdDev.Num() == 1)
                {
                    StdDev = DenormalizationParams.StdDev[0];
                }
                else if (i < DenormalizationParams.StdDev.Num())
                {
                    StdDev = DenormalizationParams.StdDev[i];
                }
                else if (DenormalizationParams.StdDev.Num() > 0)
                {
                     UE_LOG(LOG_UERLTOOLS, Warning, TEXT("RLMatrixToUEArray: Denormalization StdDev array size (%d) is insufficient for element index %d (total elements %d). Using default stddev 1."), DenormalizationParams.StdDev.Num(), i, NumElements);
                }
                
                UEArray[i] = (ValueFromMatrix * StdDev) + Mean;
            }
            else
            {
                UEArray[i] = ValueFromMatrix;
            }
        }
        return true;
    }

    // Explicit template instantiations should be carefully managed or placed in a separate .cpp file
    // if they are truly needed and this header is widely included. For now, commenting out.
    // Example:
    // using DEVICE = rl_tools::devices::DefaultCPU;
    // using SPEC_1_10 = rl_tools::matrix::Specification<float, DEVICE::index_t, 1, 10>;
    // template bool UEArrayToRLMatrix<SPEC_1_10>(const TArray<float>&, rl_tools::Matrix<SPEC_1_10>&, const FRLNormalizationParams&);
    // template bool RLMatrixToUEArray<SPEC_1_10>(const rl_tools::Matrix<SPEC_1_10>&, TArray<float>&, const FRLNormalizationParams&);

} // namespace RLToolsConversionUtils

#include "Logging/LogMacros.h"

// Fallback log category
#ifndef LOG_UERLTOOLS
#define LOG_UERLTOOLS LogTemp
#else
#define LOG_UERLTOOLS LogUERLTools
#endif

// If FRLNormalizationParams is used, include its definition
// #include "RLTypes.h" 

namespace RLToolsConversionUtils
{
    template <typename T_MATRIX_SPEC>
    bool UEArrayToRLMatrix(
        const TArray<float>& UEArray,
        rl_tools::Matrix<T_MATRIX_SPEC>& RLMatrix
        // bool bNormalize, // Uncomment when normalization is added
        // const FRLNormalizationParams& NormalizationParams // Uncomment when normalization is added
    )
    {
        // using DEVICE = rl_tools::devices::DefaultCPU; // Assuming CPU for now
        // using TI = typename DEVICE::index_t;
        // rl_tools::devices::DefaultCPU device; // Temporary device instance

        constexpr auto ROWS = rl_tools::rows(RLMatrix);
        constexpr auto COLS = rl_tools::cols(RLMatrix);
        const int32 ExpectedNumElements = ROWS * COLS;

        if (UEArray.Num() != ExpectedNumElements)
        {
            UE_LOG(LOG_UERLTOOLS, Error, TEXT("UEArrayToRLMatrix: Dimension mismatch. UEArray has %d elements, RLMatrix expects %d."), UEArray.Num(), ExpectedNumElements);
            return false;
        }

        // Direct copy for now. Normalization would happen here.
        for (int32 i = 0; i < ExpectedNumElements; ++i)
        {
            // Assuming RLMatrix is a 1D view or direct access to data is simple.
            // For a 2D matrix, access would be rl_tools::get(RLMatrix, row_idx, col_idx) = ...
            // If T_MATRIX_SPEC::ROWS == 1 (row vector)
            if constexpr (ROWS == 1) {
                 rl_tools::set(RLMatrix, 0, i, static_cast<typename T_MATRIX_SPEC::T>(UEArray[i]));
            }
            // If T_MATRIX_SPEC::COLS == 1 (column vector)
            else if constexpr (COLS == 1) {
                 rl_tools::set(RLMatrix, i, 0, static_cast<typename T_MATRIX_SPEC::T>(UEArray[i]));
            }
            // Else, it's a general matrix, need to map 1D index to 2D
            else {
                // This simple loop assumes row-major storage for the 1D UEArray mapping to RLMatrix
                // RLMatrix internal storage might be column-major or row-major depending on rl_tools
                // For simplicity, assuming direct indexed access to underlying buffer if possible,
                // or using set(matrix, r, c, value)
                int32 Row = i / COLS;
                int32 Col = i % COLS;
                if (Row < ROWS && Col < COLS) { // Boundary check
                     rl_tools::set(RLMatrix, Row, Col, static_cast<typename T_MATRIX_SPEC::T>(UEArray[i]));
                } else {
                    UE_LOG(LOG_UERLTOOLS, Error, TEXT("UEArrayToRLMatrix: Index out of bounds during 2D mapping. This should not happen if ExpectedNumElements is correct."));
                    return false;
                }
            }
        }

        // TODO: Implement normalization if bNormalize is true using NormalizationParams
        // if (bNormalize) { NormalizeMatrix(RLMatrix, NormalizationParams); }

        return true;
    }

    template <typename T_MATRIX_SPEC>
    bool RLMatrixToUEArray(
        const rl_tools::Matrix<T_MATRIX_SPEC>& RLMatrix,
        TArray<float>& UEArray
        // bool bDenormalize, // Uncomment when normalization is added
        // const FRLNormalizationParams& NormalizationParams // Uncomment when normalization is added
    )
    {
        // using DEVICE = rl_tools::devices::DefaultCPU;
        // using TI = typename DEVICE::index_t;
        // rl_tools::devices::DefaultCPU device;

        constexpr auto ROWS = rl_tools::rows(RLMatrix);
        constexpr auto COLS = rl_tools::cols(RLMatrix);
        const int32 NumElements = ROWS * COLS;

        UEArray.SetNumUninitialized(NumElements); // Resize array

        // Direct copy for now. Denormalization would happen here.
        // Create a temporary mutable copy if denormalization modifies the matrix in-place
        // rl_tools::Matrix<T_MATRIX_SPEC> TempMatrix = RLMatrix; // If needed for denormalization
        // if (bDenormalize) { DenormalizeMatrix(TempMatrix, NormalizationParams); }

        for (int32 i = 0; i < NumElements; ++i)
        {
            if constexpr (ROWS == 1) {
                UEArray[i] = static_cast<float>(rl_tools::get(RLMatrix, 0, i));
            }
            else if constexpr (COLS == 1) {
                UEArray[i] = static_cast<float>(rl_tools::get(RLMatrix, i, 0));
            }
            else {
                int32 Row = i / COLS;
                int32 Col = i % COLS;
                 UEArray[i] = static_cast<float>(rl_tools::get(RLMatrix, Row, Col));
            }
        }
        return true;
    }

    // Explicitly instantiate common types to avoid linker errors if these are not inlined
    // This depends on what T_MATRIX_SPEC will commonly be.
    // Example:
    // template bool UEArrayToRLMatrix<rl_tools::matrix::Specification<float, rl_tools::devices::DefaultCPU::index_t, 1, 10>>(const TArray<float>&, rl_tools::Matrix<rl_tools::matrix::Specification<float, rl_tools::devices::DefaultCPU::index_t, 1, 10>>&);
    // template bool RLMatrixToUEArray<rl_tools::matrix_specification<float, rl_tools::devices::DefaultCPU::index_t, 1, 10>>(const rl_tools::Matrix<rl_tools::matrix::Specification<float, rl_tools::devices::DefaultCPU::index_t, 1, 10>>&, TArray<float>&);

} // namespace RLToolsConversionUtils
