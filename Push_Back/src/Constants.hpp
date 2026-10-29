#pragma once

#include <cstdint>

/**
 * Namespace for constants used throughout the codebase.
 * These constants are stagnant for each robot.
 */
namespace Constants {

    /**
     * Enum for the type of drive control to use.
     * VOLTAGE: Set the voltage of the motors directly.
     * POWER: Set the power of the motors from analog sticks.
    */
    enum DriveType {
        /** Set the voltage of the motors directly. */
        VOLTAGE,
        /** Set the power of the motors from analog sticks. */
        POWER
    };

    namespace Hardware {
        /**
         * The diameter of a tracking wheel
         *     • Measured between outer omni rollers
         *     • Measured in inches
        */
        constexpr double TRACKING_DIAMETER = 2.7250;

        /** Ratio of tracking wheel upper gear to lower wheel gear */
        constexpr double TRACKING_RATIO = 1.0;

        /**
         * The diameter of a tracking wheel
         *     • Measured between the centers of the outer wheels
         *     • Measured in inches
        */
        constexpr double ROBOT_DIAMETER = 7.6;

        /**
         * Half the distance between the centers of each tracking wheel
         *      • Measured in inches
        */
        constexpr double ROBOT_RADIUS = ROBOT_DIAMETER / 2;

        /** Maximum motor voltage in milli-volts */
        constexpr int32_t MAX_MOTOR_VOLTAGE = 12'000;

        /**
         * Maximum motor power.
         *     • Value is unless and corresponds to the min/max analog stick values
         *     • Used when setting motor power
        */
        constexpr int32_t MAX_MOTOR_POWER = 127;
    }

    /**
     * Constants used for any sort of path following algorithm.
     * At the time of writing, this only pertains to pure pursuit, but could include others as well.
     */
    namespace PathFollowing {
        /** In pure pursuit, the distance from the robot to the lookahead point (inches) */
        constexpr double LOOKAHEAD_DISTANCE = 5.0;

        /** In pure pursuit, the distance from the target point before the robot stops (inches) */
        constexpr double STOP_DISTANCE = 2;
    }

}
