#include "main.h"
#include "apix.h"

#include <fcntl.h>
#include <ports.hpp>
#include <thread>
#include <stdfloat>
#include <subsystems/Intakes.hpp>
#include <subsystems/Lift.hpp>

#include "architecture/AutonomousControlScheduler.hpp"
#include "architecture/DriverControlScheduler.hpp"
#include "subsystems/Drivetrain.hpp"

#include "SerialHandler.hpp"
#include "comms/BrainComm.hpp"
#include "packets/OpticalPacket.hpp"

// Create all subsystems:
Drivetrain& drivetrain = AbstractSubsystem::get_instance<Drivetrain>();
IntakeBottom& intake_bottom = AbstractSubsystem::get_instance<IntakeBottom>();
IntakeTop& intake_top = AbstractSubsystem::get_instance<IntakeTop>();
IntakeEnd& intake_end = AbstractSubsystem::get_instance<IntakeEnd>();
Lift& lift = AbstractSubsystem::get_instance<Lift>();

// Add subsystems to vector for iteration
std::vector<AbstractSubsystem*> subsystems = { &drivetrain, &intake_bottom, &intake_top, &intake_end, &lift };

/** Global serial handler instance used to communicate with auxiliary devices. */
static SerialHandler serial_handler{std::make_unique<BrainComm>()};

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
    // Initialize all subsystems
    for (AbstractSubsystem* subsystem : subsystems) {
        subsystem->initialize();
    }
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {
    // Ensure all subsystems have shutdown/stopped.
    for (AbstractSubsystem* subsystem : subsystems) {
        subsystem->shutdown();
    }

    // Run disabled periodic for all subsystems
    while (true) {
        for (AbstractSubsystem* subsystem: subsystems) {
            subsystem->disabled_periodic();
        }
    }
}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {
}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
    // Initialize the autonomous scheduler
    AutonomousControlScheduler autonomous_scheduler{};

    autonomous_scheduler.initialize();

    // Run forever
    while (true) {
        // Run the autonomous scheduler to do our routine
        autonomous_scheduler.run();

        // Run periodic for all subsystems
        for (AbstractSubsystem* subsystem : subsystems) {
            subsystem->periodic();
        }

        // Delay the loop to prevent the CPU from being overwhelmed
        pros::delay(5);
    }
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
    // Initialize the driver control scheduler
    DriverControlScheduler driver_scheduler{};
    driver_scheduler.initialize();

    while (true) {
        driver_scheduler.run();

        // Run periodic for all subsystems
        for (AbstractSubsystem* subsystem : subsystems) {
            subsystem->periodic();
        }

        // Delay the loop to prevent the CPU from being overwhelmed
        pros::delay(5);
    }
}
