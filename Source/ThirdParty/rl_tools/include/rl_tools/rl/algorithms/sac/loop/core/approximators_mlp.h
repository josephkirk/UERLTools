#include "../../../../../version.h"
#if (defined(RL_TOOLS_DISABLE_INCLUDE_GUARDS) || !defined(RL_TOOLS_RL_ALGORITHMS_SAC_LOOP_CORE_APPROXIMATORS_MLP_H)) && (RL_TOOLS_USE_THIS_VERSION == 1)
#pragma once
#define RL_TOOLS_RL_ALGORITHMS_SAC_LOOP_CORE_APPROXIMATORS_MLP_H

RL_TOOLS_NAMESPACE_WRAPPER_START
namespace rl_tools::rl::algorithms::sac::loop::core{
    // The approximator config sets up any types that support the usual rl_tools::forward and rl_tools::backward operations (can be custom as well)
    // We provide approximators based on the sequential and mlp models. The latter (mlp) allows for a variable number of layers, but is restricted to a uniform hidden layer size while the former allows for arbitrary layers to be combined in a sequential manner. Both support compile-time autodiff
    template<typename T, typename TI, typename ENVIRONMENT, typename PARAMETERS>
    struct ConfigApproximatorsMLP{
        using SAC_PARAMETERS = typename PARAMETERS::SAC_PARAMETERS;
        template <typename CAPABILITY>
        struct Actor{
            using INPUT_SHAPE = tensor::Shape<TI, SAC_PARAMETERS::SEQUENCE_LENGTH, SAC_PARAMETERS::ACTOR_BATCH_SIZE, ENVIRONMENT::Observation::DIM>;
            using MLP_CONFIG = nn_models::mlp::Configuration<T, TI, 2*ENVIRONMENT::ACTION_DIM, PARAMETERS::ACTOR_NUM_LAYERS, PARAMETERS::ACTOR_HIDDEN_DIM, PARAMETERS::ACTOR_ACTIVATION_FUNCTION,  nn::activation_functions::IDENTITY, typename PARAMETERS::INITIALIZER>;
            using MLP = nn_models::mlp::BindConfiguration<MLP_CONFIG>;
            struct SAMPLE_AND_SQUASH_LAYER_PARAMETERS{
                static constexpr T LOG_STD_LOWER_BOUND = SAC_PARAMETERS::LOG_STD_LOWER_BOUND;
                static constexpr T LOG_STD_UPPER_BOUND = SAC_PARAMETERS::LOG_STD_UPPER_BOUND;
                static constexpr T LOG_PROBABILITY_EPSILON = SAC_PARAMETERS::LOG_PROBABILITY_EPSILON;
                static constexpr bool ADAPTIVE_ALPHA = SAC_PARAMETERS::ADAPTIVE_ALPHA;
                static constexpr bool UPDATE_ALPHA_WITH_ACTOR = false;
                static constexpr T ALPHA = SAC_PARAMETERS::ALPHA;
                static constexpr T TARGET_ENTROPY = SAC_PARAMETERS::TARGET_ENTROPY;
            };
            using SAMPLE_AND_SQUASH_CONFIG = nn::layers::sample_and_squash::Configuration<T, TI, SAMPLE_AND_SQUASH_LAYER_PARAMETERS>;
            using SAMPLE_AND_SQUASH = nn::layers::sample_and_squash::BindConfiguration<SAMPLE_AND_SQUASH_CONFIG>;
            template <typename T_CONTENT, typename T_NEXT_MODULE = nn_models::sequential::OutputModule>
            using Module = typename nn_models::sequential::Module<T_CONTENT, T_NEXT_MODULE>;
            using MODULE_CHAIN = Module<MLP, Module<SAMPLE_AND_SQUASH>>;

            using MODEL = nn_models::sequential::Build<CAPABILITY, MODULE_CHAIN, INPUT_SHAPE>;

        };
        template <typename CAPABILITY>
        struct Critic{
            static constexpr TI INPUT_DIM = ENVIRONMENT::ObservationPrivileged::DIM+ENVIRONMENT::ACTION_DIM;
            using INPUT_SHAPE = tensor::Shape<TI, 1, SAC_PARAMETERS::CRITIC_BATCH_SIZE, INPUT_DIM>;
            using MLP_CONFIG = nn_models::mlp::Configuration<T, TI, 1, PARAMETERS::CRITIC_NUM_LAYERS, PARAMETERS::CRITIC_HIDDEN_DIM, PARAMETERS::CRITIC_ACTIVATION_FUNCTION, nn::activation_functions::IDENTITY, typename PARAMETERS::INITIALIZER>;
            using MLP = nn_models::mlp::BindConfiguration<MLP_CONFIG>;
            template <typename T_CONTENT, typename T_NEXT_MODULE = nn_models::sequential::OutputModule>
            using Module = typename nn_models::sequential::Module<T_CONTENT, T_NEXT_MODULE>;
            using MODULE_CHAIN = Module<MLP>;

            using MODEL = nn_models::sequential::Build<CAPABILITY, MODULE_CHAIN, INPUT_SHAPE>;
        };

        using ACTOR_OPTIMIZER_SPEC = nn::optimizers::adam::Specification<T, TI, typename PARAMETERS::ACTOR_OPTIMIZER_PARAMETERS>;
        using CRITIC_OPTIMIZER_SPEC = nn::optimizers::adam::Specification<T, TI, typename PARAMETERS::CRITIC_OPTIMIZER_PARAMETERS>;
        using ALPHA_OPTIMIZER_SPEC = nn::optimizers::adam::Specification<T, TI, typename PARAMETERS::ALPHA_OPTIMIZER_PARAMETERS>;
        using ACTOR_OPTIMIZER = nn::optimizers::Adam<ACTOR_OPTIMIZER_SPEC>;
        using CRITIC_OPTIMIZER = nn::optimizers::Adam<CRITIC_OPTIMIZER_SPEC>;
        using ALPHA_OPTIMIZER = nn::optimizers::Adam<ALPHA_OPTIMIZER_SPEC>;
        using CAPABILITY_ACTOR = nn::capability::Gradient<nn::parameters::Adam>;
        using CAPABILITY_CRITIC = nn::capability::Gradient<nn::parameters::Adam>;
        using ACTOR_TYPE = typename Actor<CAPABILITY_ACTOR>::MODEL;
        using CRITIC_TYPE = typename Critic<CAPABILITY_CRITIC>::MODEL;
        using CRITIC_TARGET_TYPE = typename Critic<nn::capability::Forward<>>::MODEL;
    };
}
RL_TOOLS_NAMESPACE_WRAPPER_END
#endif

