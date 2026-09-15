#include "DriverControlScheduler.hpp"

#include "ButtonMappings.hpp"

DriverControlScheduler::DriverControlScheduler() : ParallelCommand({}){
}

bool DriverControlScheduler::is_complete() {
    return false;
}

void DriverControlScheduler::initialize() {
    // TODO: remove when better system is implemented, see issue #29
    drivetrain.set_reversing(false);
}

int32_t scale_power(const int32_t power, double scaling_factor) {
    // https://www.desmos.com/calculator/arejdbcye3
    scaling_factor = std::clamp(scaling_factor, 0.0, 400.0);
    int32_t scaled_power = std::abs(power) * std::exp((scaling_factor / 100 * std::log(127) / 127) * (std::abs(power) - 127));
    // Needed because exponential function cannot produce negative numbers
    return power < 0 ? -scaled_power : scaled_power;
}

int32_t cutoff_power(const int32_t power, int32_t cutoff)
{
    return abs(power) > cutoff ? power : 0;
}

void DriverControlScheduler::periodic() {
#if THINK
    // Normal controls

#endif

#if DO
    // Garret's controls

#endif

    const int32_t left_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_LEFT_Y);
    const int32_t right_power = controller.get_analog(pros::E_CONTROLLER_ANALOG_RIGHT_Y);

    const int32_t left_power_scaled = scale_power(left_power, Constants::Controller::INPUT_SCALING_FACTOR);
    const int32_t right_power_scaled = scale_power(right_power, Constants::Controller::INPUT_SCALING_FACTOR);

    if (abs(left_power_scaled) > 1 || abs(right_power_scaled) > 1) {
        drivetrain.set_drive_power(left_power_scaled, right_power_scaled);
        drivetrain.set_braking(false);
    } else
    {
        drivetrain.set_drive_power(0,0);
    }

    for (auto &[button, command] : BINDS) {
        // controller state for this tick
        const bool new_controller_state = controller.get_digital(button);

        if (command[0].has_value() && controller.get_digital_new_press(button)) { // button just pressed
            this->add_command(command[0].value()());
        } else if (command[1].has_value() && controller.get_digital(button)) { // button down but not just pressed
            this->add_command(command[1].value()());
        } else if (command[2].has_value() && !new_controller_state && controller_state[button]) { // if the button isn't pressed this tick and it was pressed last tick
            this->add_command(command[2].value()());
        }

        // update actual map
        controller_state[button] = new_controller_state;
    }
    ParallelCommand::periodic();
}

void DriverControlScheduler::shutdown() {
    // Does nothing
}