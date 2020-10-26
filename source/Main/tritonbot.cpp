
#include <iostream>
#include <armadillo>
#include "PubSubSystem/thread_pool.hpp"
#include "Utility/boost_logger.hpp"
#include "ProtoGenerated/vFirmware_API.pb.h"
#include "Utility/systime.hpp"
#include "Utility/common.hpp"
#include "Config/config.hpp"

////////////////////////MODULES///////////////////////////
#include "FirmClientModule/firmware_client_module.hpp"
#include "FirmClientModule/vfirm_client.hpp"
#include "EKF-Module/motion_ekf_module.hpp"
#include "EKF-Module/virtual_motion_ekf.hpp"
#include "EKF-Module/ball_ekf_module.hpp"
#include "EKF-Module/virtual_ball_ekf.hpp"
#include "ControlModule/control_module.hpp"
#include "ControlModule/pid_system.hpp"
#include "MotionModule/motion_module.hpp"
#include "RemoteServers/ConnectionServer/connection_server_module.hpp"
#include "RemoteServers/RemoteCMDServer/cmd_server_module.hpp"
//////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const arma::vec& v);

int main(int arc, char *argv[]) {
    // Logger Initialization
    B_Log::static_init();
    B_Log::set_shorter_format();
    // B_Log::sink->set_filter(severity >= Debug && tag_attr == "VFirmClient Module");
    B_Log::sink->set_filter(severity >= Info);    
    B_Log logger;

    // Preallocate Threads 
    ThreadPool thread_pool(THREAD_POOL_SIZE); // pre-allocate # threads in a pool

    // Construct module instances
    boost::shared_ptr<FirmClientModule> firm_client_module(new VFirmClient());
    boost::shared_ptr<MotionEKF_Module> motion_ekf_module(new VirtualMotionEKF());
    boost::shared_ptr<BallEKF_Module> ball_ekf_module(new VirtualBallEKF());
    boost::shared_ptr<MotionModule> motion_module(new MotionModule());
    boost::shared_ptr<ControlModule> control_module(new PID_System());
    boost::shared_ptr<CMDServerModule> cmd_server_module(new CMDServer());
    boost::shared_ptr<ConnectionServerModule> connection_server_module(new ConnectionServer());
    
    // Configs
    PID_System::PID_Constants pid_consts;
    pid_consts.RD_Kp = PID_RD_KP;   pid_consts.RD_Ki = PID_RD_KI;   pid_consts.RD_Kd = PID_RD_KD;
    pid_consts.TD_Kp = PID_TD_KP;   pid_consts.TD_Ki = PID_TD_KI;   pid_consts.TD_Kd = PID_TD_KD;
    ITPS::NonBlockingPublisher<PID_System::PID_Constants> pid_const_pub("PID", "Constants", pid_consts);
        // dummy publishers for now
    ITPS::NonBlockingPublisher<bool> dribbler_pub("BallCapture", "Dribbler", false);
    ITPS::NonBlockingPublisher<arma::vec> kicker_pub("Kicker", "KickingSetPoint", zero_vec_2d());

    // Run the servers
    firm_client_module->run(thread_pool);
    motion_ekf_module->run(thread_pool);    
    ball_ekf_module->run(thread_pool);
    motion_module->run(thread_pool);
    control_module->run(thread_pool);
    cmd_server_module->run(thread_pool);
    connection_server_module->run(thread_pool);

    

    while(1); // this program should run forever 

    return 0;
}

std::ostream& operator<<(std::ostream& os, const arma::vec& v)
{
    char fmt_str[30]; // not so important size, greater than printed str size is fine, use magic number here 
    int num_rows = arma::size(v).n_rows;
    os << "<";
    for(int i = 0; i < num_rows; i++) {
        sprintf(fmt_str, "%8.3lf", v(i));
        os << std::string(fmt_str);
        if(i != num_rows - 1) os << ", ";
    }
    os << ">";
    return os;
}
