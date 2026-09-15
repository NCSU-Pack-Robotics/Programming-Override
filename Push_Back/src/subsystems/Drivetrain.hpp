#pragma once

#include "main.h"
#include "architecture/AbstractSubsystem.hpp"
#include "Config.hpp"
#include "Constants.hpp"
#include "math/odometry/Pose.hpp"
#include "math/PID.hpp"
#include "math/odometry/OdometryGyro.hpp"


/**
 * The Drivetrain subsystem is responsible for controlling and reading from the drive motors.
 */
class Drivetrain : public AbstractSubsystem {
    friend class AbstractSubsystem;

public:
    void initialize() override;

    void periodic() override;

    void disabled_periodic() override;

    void shutdown() override;

    /**
    * Set the voltage (in milli-volts) of the left and right motors in range [-12000,12000].
    * @param left_mV The voltage to set the left motors to.
    * @param right_mV The voltage to set the right motors to.
    */
    void set_voltage(int32_t left_mV, int32_t right_mV);

    /**
    * Set the power of the left and right motors in range [-127, 127].
    * @param left The power to set the left motors to.
    * @param right The power to set the right motors to.
    */
    void set_drive_power(int32_t left, int32_t right);

    /**
    * Set the motors to run at a target velocity in inches per second
    * @param target_left The velocity of the left motors in inches per second
    * @param target_right The velocity of the right motors in inches per second
    */
    void set_velocity(double target_left, double target_right);

    /** Sets the robot to be braking. When it is in this state all methods that would usually move the robot will not work. When this method is called the robot is not
     * guaranteed to be braked, it will become braked when the next periodic loop runs if braking is still true.
     * @returns The previous brake state.
     */
    bool set_braking(bool braking);

    /**
     * Immediately stops the robot, setting all power and voltage power to 0.
     * This is an instantaneous setting of the brake, and should not be used unless you know it won't be reset
     * in the next periodic loop.
     */
    void brake_now();

    /**
     * Sets the robot to be reversing. All motor inputs will be reversed.
     * This is done by setting an internal flag and by setting motors to -1 * the power/voltage.
     * @returns The previous reversal state.
     */
    bool set_reversing(bool reversing);

    /**
     * Get the position of the left and right motors in degrees.
     * @param respect_reverse If true, the returned values will be negated and swapped. If false,
     * the returned value will be absolute to the original orientation of the robot.
     * Ex: This should be true if dealing with position deltas, and you want to pass positive or
     * negative values to some sort of PID.
     * @return A pair of the left and right motor positions in degrees. The first value is the left
     * motor position, and the second value is the right motor position.
     */
    // std::pair<double, double> get_position(bool respect_reverse=false) const;

    /**
    * Get the pose of the robot.
    * This is the result of odometer calculations.
    * @return The pose of the robot.
    */
    Pose get_pose() const;

    /**
     * Set the pose of the robot.
     * This method is intended to be used when the robot hits a wall to realign itself.
     * @param new_pose The new pose of the robot.
     */
    void set_pose(const Pose& new_pose);

private:
    /** Voltage in mV to set motors to. Will be between -12,000 and +12,000. */
    int32_t left_drive_voltage{};
    /** Voltage in mV to set motors to. Will be between -12,000 and +12,000. */
    int32_t right_drive_voltage{};

    /** The current braking state of the robot */
    bool braking;
    /** The current reversal state of the robot */
    bool reversing;
    /** The direction the robot is moving in. 1 for forward, -1 for reverse */
    int direction;

    OdometryGyro odometry;

    /** Power to set motors to from analog sticks. Will be between [-127, 127] */
    int32_t left_drive_power{};
    /** Power to set motors to from analog sticks. Will be between [-127, 127] */
    int32_t right_drive_power{};

    /** The PID used for the left motors velocity */
    PID left_velocity_pid{Constants::PID::Drive::Velocity::Kp,
                                Constants::PID::Drive::Velocity::Kd,
                                Constants::PID::Drive::Velocity::Ki};
    /** The PID used for the right motors velocity */
    PID right_velocity_pid{Constants::PID::Drive::Velocity::Kp,
                                 Constants::PID::Drive::Velocity::Kd,
                                 Constants::PID::Drive::Velocity::Ki};


    /** The PID used for turning */
    PID angular_pid{Constants::PID::Drive::Angular::Kp,
                          Constants::PID::Drive::Angular::Kd,
                          Constants::PID::Drive::Angular::Ki};

    /** Type of drive control to use. */
    Constants::DriveType drive_type;



    // Motors:
    /** The left, front motor farther from the center. */
    pros::Motor left_front1;
    /** The left, front motor closer to the center. */
    pros::Motor left_front2;

    /** The right, front motor farther from the center. */
    pros::Motor right_front1;
    /** The right, front motor closer to the center. */
    pros::Motor right_front2;

    /** The right, back motor farther from the center. */
    pros::Motor right_back1;
    /** The right, back motor closer to the center. */
    pros::Motor right_back2;

    /** The left, back motor farther from the center. */
    pros::Motor left_back1;
    /** The left, back motor closer to the center. */
    pros::Motor left_back2;




    // Motor groups:
    /** Group of all the motors on the left of the robot. */
    pros::MotorGroup left_motors;
    /** Group of all the motors on the right of the robot. */
    pros::MotorGroup right_motors;

    // Rotation sensors:
    /** Smart pointer to the left rotation sensor. */
    // pros::Rotation left_rotation_sensor;
    /** Smart pointer to the right rotation sensor. */
    // pros::Rotation right_rotation_sensor;

    // Gyro:
    /** Smart pointer to the gyro sensor. */
    // pros::Imu gyro;

protected:
    Drivetrain();
};
