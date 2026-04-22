#pragma once

namespace Profiles{
    namespace Caipora{
        static constexpr int RIGHT_POS_PIN = 16;
        static constexpr int RIGHT_NEG_PIN = 17;
        static constexpr int LEFT_POS_PIN = 18;
        static constexpr int LEFT_NEG_PIN = 19;
    }

    namespace Smoker{
        static constexpr int RIGHT_POS_PIN = 19;
        static constexpr int RIGHT_NEG_PIN = 18;
        static constexpr int LEFT_POS_PIN = 17;
        static constexpr int LEFT_NEG_PIN = 16;
    }

}

#if defined(ROBOT_CAIPORA)
    using namespace Profiles::Caipora;
#elif defined(ROBOT_SMOKER)
    using namespace Profiles::Smoker;
#else
    #error "Nenhum robô definido!"
#endif