#include "Drivetrain.hpp"
#include <numeric>
#include <set>

#include "ports.hpp"
#include "math/Utils.hpp"

/** Enum to multiply drive powers by to move robot forward. */
enum Direction {
    BACKWARDS = -1,
    FORWARDS = 1
};

Drivetrain::Drivetrain() :
    AbstractSubsystem(),
    odometry({Constants::Initial::Pose::INITIAL_X, Constants::Initial::Pose::INITIAL_Y, Constants::Initial::Pose::INITIAL_HEADING}),

    left_front1(Ports::LEFT_FRONT1_MOTOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorUnits::degrees),
    left_front2(Ports::LEFT_FRONT2_MOTOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorUnits::degrees),

    right_front1(Ports::RIGHT_FRONT1_MOTOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorUnits::degrees),
    right_front2(Ports::RIGHT_FRONT2_MOTOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorUnits::degrees),

    right_back1(Ports::RIGHT_BACK1_MOTOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorUnits::degrees),
    right_back2(Ports::RIGHT_BACK2_MOTOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorUnits::degrees),

    left_back1(Ports::LEFT_BACK1_MOTOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorUnits::degrees),
    left_back2(Ports::LEFT_BACK2_MOTOR_PORT, pros::v5::MotorGears::blue, pros::v5::MotorUnits::degrees),

    left_motors({left_front1.get_port(), left_front2.get_port(), left_back1.get_port(), left_back2.get_port()},
               left_front1.get_gearing(),
               left_front1.get_encoder_units()),

    right_motors({right_front1.get_port(), right_front2.get_port(), right_back1.get_port(), right_back2.get_port()},
                right_front1.get_gearing(),
                right_front1.get_encoder_units())

    // left_rotation_sensor(Ports::LEFT_ROTATION_SENSOR_PORT),
    // right_rotation_sensor(Ports::RIGHT_ROTATION_SENSOR_PORT),

    // gyro(Ports::GYRO_PORT)
{
    this->reversing = false;
    this->braking = false;
    this->direction = Direction::FORWARDS;
    this->drive_type = Constants::DriveType::VOLTAGE;
}

void Drivetrain::initialize() {
    // Set all positions to 0
    left_motors.tare_position_all();
    right_motors.tare_position_all();

    // TODO: Change these when odom pods are added
    // left_rotation_sensor.reset();
    // right_rotation_sensor.reset();
    // left_rotation_sensor.reset_position();
    // right_rotation_sensor.reset_position();

    // TODO: Change when gyro is added
    // Calibrate the gryo
    // gyro.reset(true);  // Takes about 2 seconds - max 3 seconds
    // gyro.tare();
    // gyro.tare_euler();

    reversing = false;
    braking = false;
}

void Drivetrain::periodic() {
    // Calculate the pose of the robot
    // const double heading = -this->gyro.get_rotation() * M_PI / 180;
    // this->odometry.calculate(this->get_position(), heading);

    if (braking) {
        brake_now();
        // return because nothing after this matters if it is braking.
        return;
    } else {
        left_motors.set_brake_mode_all(pros::E_MOTOR_BRAKE_COAST);
        right_motors.set_brake_mode_all(pros::E_MOTOR_BRAKE_COAST);
    }

    switch (drive_type) {
        case Constants::DriveType::POWER: {
            if (reversing) {
                right_motors.move(direction * left_drive_power);
                left_motors.move(direction * right_drive_power);
            } else {
                left_motors.move(direction * left_drive_power);
                right_motors.move(direction * right_drive_power);
            }
            break;
        }
        case Constants::DriveType::VOLTAGE: {
            if (reversing) {
                right_motors.move_voltage(direction * left_drive_voltage);
                left_motors.move_voltage(direction * right_drive_voltage);
            } else {
                left_motors.move_voltage(direction * left_drive_voltage);
                right_motors.move_voltage(direction * right_drive_voltage);
            }
            break;
        }
    }
}

void Drivetrain::disabled_periodic() {
    // Figure out when odom hardware is usable

    // In case robot is moved when disabled
    // const double heading = -this->gyro.get_rotation()  * M_PI / 180;
    // this->odometry.calculate(this->get_position(), heading);
}

void Drivetrain::shutdown() {
    brake_now();
}

void Drivetrain::set_voltage(int32_t left_mV, int32_t right_mV) {
    left_drive_voltage = std::clamp(left_mV, INT32_C(-12000), INT32_C(12000));
    right_drive_voltage = std::clamp(right_mV, INT32_C(-12000), INT32_C(12000));

    drive_type = Constants::DriveType::VOLTAGE;
}

void Drivetrain::set_drive_power(int32_t left, int32_t right) {
    left_drive_power = std::clamp(left, INT32_C(-127), INT32_C(127));
    right_drive_power = std::clamp(right, INT32_C(-127), INT32_C(127));

    drive_type = Constants::DriveType::POWER;
}

void Drivetrain::set_velocity(double target_left, double target_right) {
    // Get current velocities from the motors
    const std::vector<double> left_vel = left_motors.get_actual_velocity_all();
    const std::vector<double> right_vel = right_motors.get_actual_velocity_all();

    // Add all left/right motors together and divide by count to get average velocity
    const double left_avg_vel = Utils::rpm_to_ips(std::reduce(left_vel.begin(),
    left_vel.end(), 0.0) / left_vel.size());
    const double right_avg_vel = Utils::rpm_to_ips(std::reduce(right_vel.begin(),
    right_vel.end(), 0.0) / right_vel.size());

    // Calculate current error

    const double left_error = target_left - left_avg_vel;
    const double right_error = target_right - right_avg_vel;

    // Calculate new voltages to set
    const auto front_left_voltage = static_cast<int32_t>(left_velocity_pid.calculate(left_error));
    const auto front_right_voltage = static_cast<int32_t>(right_velocity_pid.calculate(right_error));

    // Set the new voltages
    set_voltage(front_left_voltage, front_right_voltage);
}

bool Drivetrain::set_braking(const bool braking) {
    // Don't need to do anything if its already in the right mode
    if (braking == this->braking) return braking;
    bool old = this->braking;

    this->braking = braking;
    return old;
}

void Drivetrain::brake_now() {
    left_motors.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
    right_motors.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);

    // Clear old velocities
    left_drive_power = 0;
    right_drive_power = 0;
    left_drive_voltage = 0;
    right_drive_voltage = 0;

    // set braking
    left_motors.brake();
    right_motors.brake();
}

bool Drivetrain::set_reversing(const bool reversing) {
    // Don't need to do anything if its already in the right mode
    if (reversing == this->reversing) return reversing;
    const bool old = this->reversing;

    this->direction = reversing ? Direction::BACKWARDS : Direction::FORWARDS;

    this->reversing = reversing;
    return old;
}

// TODO: Figure out when odom hardware is available
// std::pair<double, double> Drivetrain::get_position(const bool respect_reverse) const {
//     // Get the positions of the drivetrain in degrees - https://www.vexforum.com/t/get-angle-vs-get-position-pros/115915
//     double left_position  = left_rotation_sensor.get_position() / 100.0;
//     double right_position = right_rotation_sensor.get_position() / 100.0;
//
//     // Reverse values if we are in reverse mode
//     if (respect_reverse && reversing) {
//         left_position = -left_position;
//         right_position = -right_position;
//
//         return {right_position, left_position};
//     }
//
//     return {left_position, right_position};
// }

Pose Drivetrain::get_pose() const {
    Pose pose = this->odometry.get_pose();

    if (reversing) pose.heading += M_PI;

    return pose;
}

void Drivetrain::set_pose(const Pose& new_pose) {
    this->odometry.set_pose(new_pose);
}
