#include "ControlModule/virtual_pid_system.hpp"
#include "ControlModule/pid_system.hpp"
#include "PubSubSystem/thread_pool.hpp"
#include "ProtoGenerated/vFirmware_API.pb.h"
#include "Utility/systime.hpp"
#include "Utility/boost_logger.hpp"
#include "Config/config.hpp"
#include "ControlModule/pid.hpp"
#include "EKF-Module/motion_ekf_module.hpp"
#include <armadillo>

Virtual_PID_System::Virtual_PID_System() : PID_System(),
                                           cmd_eavesdrop_sub("vfirm-client", "commands")
{}


void Virtual_PID_System::init_subscribers(void) {
    PID_System::init_subscribers();
    while(!cmd_eavesdrop_sub.subscribe());
    VF_Commands cmd;
    Vec_2D zero_vec;
    std::string write;
    
    zero_vec.set_x(0.00);
    zero_vec.set_y(0.00);

    cmd.set_init(true);
    cmd.set_allocated_translational_output(&zero_vec);
    cmd.set_rotational_output(0.00);
    cmd.set_allocated_kicker(&zero_vec);
    cmd.set_dribbler(false);

    cmd_eavesdrop_sub.set_default_latest_msg(cmd);

    cmd.release_kicker();
    cmd.release_translational_output();
}

MotionEKF::MotionData Virtual_PID_System::get_sensor_feedbacks(void) {
    MotionEKF::MotionData feedback_data = PID_System::get_sensor_feedbacks();
    /* because the velocity data calc from the simulator is not stable
     * here we sneaky-mask the velocity data by making them zero(s),
     * so the pid controller will forward the setpoint value directly, 
     * which delivers good enough performance on the simulator
     */

    feedback_data.rotat_vel = 0.00;
    feedback_data.trans_vel = {0.00, 0.00};
    return feedback_data;

}
