#include <iostream>
#include <armadillo>

#include "Misc/PubSubSystem/ThreadPool.hpp"
#include "Misc/Utility/BoostLogger.hpp"
#include "Misc/Utility/Systime.hpp"
#include "Misc/Utility/Common.hpp"
#include "Config/Config.hpp"
#include "ProtoGenerated/vFirmware_API.pb.h"

////////////////////////MODULES///////////////////////////
#include "CoreModules/EKF-Module/MotionEkfModule.hpp"
#include "CoreModules/EKF-Module/BallEkfModule.hpp"
#include "CoreModules/ControlModule/ControlModule.hpp"
#include "CoreModules/MotionModule/MotionModule.hpp"
#include "CoreModules/BallCaptureModule/BallCaptureModule.hpp"
#include "PeriphModules/RemoteServers/TcpReceiveModule.hpp"
#include "PeriphModules/RemoteServers/UdpReceiveModule.hpp"
#include "PeriphModules/FirmClientModule/FirmClientModule.hpp"
//////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const arma::vec& v);

int main(int argc, char *argv[]) {
    // Logger Initialization
    B_Log::static_init();
    B_Log::set_shorter_format();
    //B_Log::sink->set_filter(severity >= Debug && tag_attr == "lobal Vision Server Module");
    B_Log::sink->set_filter(severity >= Info);    
    B_Log logger;
    
    // Process Json configurations


    // Process Comandline Arguments
    bool is_virtual = process_args(argc, argv);


    // Preallocate Threads 
    ThreadPool thread_pool(THREAD_POOL_SIZE); // pre-allocate # threads in a pool

    // Construct module instances
    boost::shared_ptr<FirmClientModule> firm_client_module(new VFirmClient());
    boost::shared_ptr<MotionEKF_Module> motion_ekf_module(new VirtualMotionEKF());
    boost::shared_ptr<BallEKF_Module> ball_ekf_module(new VirtualBallEKF());
    boost::shared_ptr<MotionModule> motion_module(new MotionModule());
    boost::shared_ptr<ControlModule> control_module(new PID_System());
    boost::shared_ptr<UdpReceiveModule> udp_receive_module(new CMDServer());
    boost::shared_ptr<TcpReceiveModule> tcp_receive_module(new ConnectionServer());
    boost::shared_ptr<BallCaptureModule> ball_capture_module(new BallCaptureModule());
    
    // Configs
    PID_System::PID_Constants pid_consts;
    pid_consts.RD_Kp = PID_RD_KP;   pid_consts.RD_Ki = PID_RD_KI;   pid_consts.RD_Kd = PID_RD_KD;
    pid_consts.TD_Kp = PID_TD_KP;   pid_consts.TD_Ki = PID_TD_KI;   pid_consts.TD_Kd = PID_TD_KD;
    ITPS::NonBlockingPublisher<PID_System::PID_Constants> pid_const_pub("PID", "Constants", pid_consts);

    // Run the servers
    firm_client_module->run(thread_pool);
    motion_ekf_module->run(thread_pool);    
    ball_ekf_module->run(thread_pool);
    motion_module->run(thread_pool);
    control_module->run(thread_pool);
    udp_receive_module->run(thread_pool);
    tcp_receive_module->run(thread_pool);
    // intern_ekf_server_module->run(thread_pool);
    ball_capture_module->run(thread_pool);
    

    while(1) { // has delay (good for reducing high CPU usage)
        // this program should run forever 
        delay(1000);
    }

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
