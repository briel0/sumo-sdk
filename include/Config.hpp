#pragma once

namespace Profiles {
    namespace Caipora {
        static constexpr int RIGHT_POS_PIN = 16;
        static constexpr int RIGHT_NEG_PIN = 17;
        static constexpr int LEFT_POS_PIN = 18;
        static constexpr int LEFT_NEG_PIN = 19;

    }

    namespace Smoker {
        static constexpr int RIGHT_POS_PIN = 19;
        static constexpr int RIGHT_NEG_PIN = 18;
        static constexpr int LEFT_POS_PIN = 17;
        static constexpr int LEFT_NEG_PIN = 16;

        static constexpr int MAX_THROTTLE = 90;
        static constexpr int TURN_COEFFICIENT = 83;
        static constexpr int PIVOT_COEFFICIENT = 70;
    }

    namespace Arruela {
        static constexpr int RIGHT_POS_PIN = 18;
        static constexpr int RIGHT_NEG_PIN = 19;
        static constexpr int LEFT_POS_PIN = 16;
        static constexpr int LEFT_NEG_PIN = 17;

        static constexpr int MAX_THROTTLE = 90;
        static constexpr int TURN_COEFFICIENT = 83;
        static constexpr int PIVOT_COEFFICIENT = 70;
    }

    namespace Marola {
        static constexpr int RIGHT_POS_PIN = 18;
        static constexpr int RIGHT_NEG_PIN = 19;
        static constexpr int LEFT_POS_PIN = 16;
        static constexpr int LEFT_NEG_PIN = 17;

        static constexpr int MAX_THROTTLE = 90;
        static constexpr int TURN_COEFFICIENT = 93;
        static constexpr int PIVOT_COEFFICIENT = 75;
    }

}

#if defined(ROBOT_CAIPORA)
using namespace Profiles::Caipora;
#elif defined(ROBOT_SMOKER)
using namespace Profiles::Smoker;
#elif defined(ROBOT_ARRUELA)
using namespace Profiles::Arruela;
#elif defined(ROBOT_MAROLA)
using namespace Profiles::Marola;
#else
#error "No robot profile defined!"
#endif