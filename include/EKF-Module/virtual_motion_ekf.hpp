#pragma once
#include "EKF-Module/motion_ekf_module.hpp"
#include <armadillo>

// Pseudo EKF
class VirtualMotionEKF : public MotionEKF_Module {
    public: 
        void task() {}
        void task(ThreadPool& thread_pool);

};