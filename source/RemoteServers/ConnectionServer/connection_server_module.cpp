#include "RemoteServers/ConnectionServer/connection_server_module.hpp"

#include <string>
#include <thread>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include "Utility/systime.hpp"
#include "PubSubSystem/thread_pool.hpp"
#include "ProtoGenerated/RemoteAPI.pb.h"
#include "Utility/boost_logger.hpp"
#include "Config/config.hpp"
#include "Utility/common.hpp"

using namespace boost;

// Reference: https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
static bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), 
        s.end(), [](unsigned char c) { return !std::isdigit(c); }) == s.end();
}

// Implementation of task to be run on this thread
void ConnectionServer::task(ThreadPool& thread_pool) {
    UNUSED(thread_pool); // no child thread is needed in a async scheme

    B_Log logger;
    logger.add_tag("Connection Server Module");
    logger(Info) << "\033[0;32m Thread Started \033[0m";


    int field_length = 0;
    int field_width = 0;
    int goal_depth = 0;
    int goal_width = 0;

    asio::io_service io_service;
    asio::ip::tcp::endpoint endpoint_to_listen(asio::ip::tcp::v4(), CONN_SERVER_PORT); 
    asio::ip::tcp::acceptor acceptor(io_service, endpoint_to_listen);
    asio::ip::tcp::socket socket(io_service);
    asio::streambuf read_buf;
    std::string write_buf;

    ITPS::NonBlockingPublisher<bool> safety_enable_pub("AI Connection", "SafetyEnable", false);
    ITPS::NonBlockingPublisher< arma::vec > robot_origin_w_pub("ConnectionInit", "RobotOrigin(WorldFrame)", zero_vec_2d());
    ITPS::NonBlockingPublisher<bool> init_sensors_pub("vfirm-client", "re/init sensors", false);
    // ITPS::BlockingSubscriber<bool> precise_kick_sub("ConnectionServer", "KickerStatusRtn");
    // ITPS::BlockingSubscriber<bool> ball_capture_sub("ConnectionServer", "BallCaptureStatusRtn");

    // TODO: since the following modules are not here yet... Uncomment in the future
    // while(!precise_kick_sub.subscribe());
    // while(!ball_capture_sub.subscribe());

    logger.log(Info, "Server Started on Port Number:" + repr(CONN_SERVER_PORT) 
                    + ", Awaiting Remote AI Connection...");

    try 
    {
        acceptor.accept(socket); // blocks until getting a connection request and accept the connection
    }
    catch(std::exception& e)
    {
        B_Log logger;
        logger.add_tag("[connection_server_module.cpp]");
        logger.log(Error, e.what());
        safety_enable_pub.publish(false);
        while(1);
    }

    logger.log(Info, "Connection Established");
    asio::write(socket, asio::buffer("CONNECTION ESTABLISHED\n"));


    while(1) {
        // get first line seperated string from the receiving buffer
        std::istream input_stream(&read_buf);

        try {
            asio::read_until(socket, read_buf, "\n"); 
        }
        catch(std::exception& e) {
            B_Log logger;
            logger.add_tag("[connection_server_module.cpp]");
            logger.log(Error, e.what());
            safety_enable_pub.publish(false);

            // To-do: handle disconnect
            while(1);
        }
        

        // Tokenize the received input
        std::vector<std::string> tokens;
        std::string tmp_str;
        while(input_stream >> tmp_str) 
        { 
            tokens.push_back(tmp_str); 
        }   

        std::string rtn_str;

        // Processing CommandLines
        if(tokens.size() > 0) {


            // Format: init [x] [y]     where (x,y) is the origin of the robot in the world coordinates
            if(tokens[0] == "init") {
                if(tokens.size() != 3 || !is_number(tokens[1]) || !is_number(tokens[2])) {
                    rtn_str = "Invalid Arguments";
                }
                else {
                    arma::vec origin = {std::stod(tokens[1]), std::stod(tokens[2])};
                    init_sensors_pub.publish(true); // Initialize Sensors
                    robot_origin_w_pub.publish(origin); // Update Robot's origin point represented in the world frame of reference
                    rtn_str = "Initialized";
                    safety_enable_pub.publish(true);
                }
            }

            else if(tokens[0] == "...") {
                // ...
            }

            else { // invalid command 
                rtn_str = "Invalid Command Received From Remote Side";
            }
        }

        asio::write(socket, asio::buffer(rtn_str + "\n"));
    }




    
}
