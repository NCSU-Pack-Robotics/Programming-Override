#pragma once

/**
 * This namespace contains all the ports for the robot.
 * Motors, sensors, and other devices should be declared here.
 */
namespace Ports {
    #if THINK

    /** Port of the left first front motor */
    inline constexpr std::int8_t LEFT_FRONT1_MOTOR_PORT = -18; // 1 is closer to the ground, 2 is higher up
    /** Port of the left second front motor */
    inline constexpr std::int8_t LEFT_FRONT2_MOTOR_PORT = 15;
    /** Port of the left second back motor */
    inline constexpr std::int8_t LEFT_BACK2_MOTOR_PORT = 8;
    /** Port of the left first back motor */
    inline constexpr std::int8_t LEFT_BACK1_MOTOR_PORT = -5;

    /** Port of the right first front motor */
    inline constexpr std::int8_t RIGHT_FRONT1_MOTOR_PORT = 12;
    /** Port of the right second front motor */
    inline constexpr std::int8_t RIGHT_FRONT2_MOTOR_PORT = -14;
    /** Port of the right second back motor */
    inline constexpr std::int8_t RIGHT_BACK2_MOTOR_PORT = -11;
    /** Port of the right first back motor */
    inline constexpr std::int8_t RIGHT_BACK1_MOTOR_PORT = 13;


    /** Port of the bottom intake motor */
    inline constexpr std::int8_t INTAKE_BOTTOM_MOTOR_PORT = -16;
    /** Port of the top intake motor */
    inline constexpr std::int8_t INTAKE_TOP_MOTOR_PORT = 10;
    /** Port of the end intake motor */
    inline constexpr std::int8_t INTAKE_END_MOTOR_PORT = -9;

    inline constexpr std::uint8_t LIFT_PISTON_PORT = 'B';

    /** Port for sending data to the auxiliary device. */
    inline constexpr std::uint8_t AUX_TX_PORT = 4;
    /** Port for receiving data from the auxiliary device. */
    inline constexpr std::uint8_t AUX_RX_PORT = 5;

    #elif DO

    /** Port of the left first front motor */
    inline constexpr std::int8_t LEFT_FRONT1_MOTOR_PORT = 10; // 1 is the wheel farther from the center, 2 is closer
    /** Port of the left second front motor */
    inline constexpr std::int8_t LEFT_FRONT2_MOTOR_PORT = -9;
    /** Port of the left second back motor */
    inline constexpr std::int8_t LEFT_BACK2_MOTOR_PORT = 8;
    /** Port of the left first back motor */
    inline constexpr std::int8_t LEFT_BACK1_MOTOR_PORT = -7;

    /** Port of the right first front motor */
    inline constexpr std::int8_t RIGHT_FRONT1_MOTOR_PORT = -13;
    /** Port of the right second front motor */
    inline constexpr std::int8_t RIGHT_FRONT2_MOTOR_PORT = 14;
    /** Port of the right second back motor */
    inline constexpr std::int8_t RIGHT_BACK2_MOTOR_PORT = -15;
    /** Port of the right first back motor */
    inline constexpr std::int8_t RIGHT_BACK1_MOTOR_PORT = 16;


    /** Port of the bottom intake motor */
    inline constexpr std::int8_t INTAKE_BOTTOM_MOTOR_PORT = 16;
    /** Port of the top intake motor */
    inline constexpr std::int8_t INTAKE_TOP_MOTOR_PORT = 10;
    /** Port of the end intake motor */
    inline constexpr std::int8_t INTAKE_END_MOTOR_PORT = 9;

    inline constexpr std::uint8_t LIFT_PISTON_PORT = 'B';

    /** Port for sending data to the auxiliary device. */
    inline constexpr std::uint8_t AUX_TX_PORT = 4;
    /** Port for receiving data from the auxiliary device. */
    inline constexpr std::uint8_t AUX_RX_PORT = 5;

    #endif
}
