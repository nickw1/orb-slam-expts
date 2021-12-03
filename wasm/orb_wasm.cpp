// Start of emscripten ORB-SLAM experiment.
// Skeleton code only. No functionality yet. 

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <emscripten.h>
#include <iostream>
#include <emscripten/bind.h>
#include <vector>
#include <chrono>
#include <System.h>



//using namespace emscripten;
using std::cout;
using std::endl;
using std::vector;

vector<double> mapPoints;
        
extern "C" {
      EMSCRIPTEN_KEEPALIVE bool receiveData(uint8_t *ptr, int width, int height);
      EMSCRIPTEN_KEEPALIVE void cleanup();
      EMSCRIPTEN_KEEPALIVE void captureKeyFrame();
}

ORB_SLAM3::System *slam;

int main(int argc, char *argv[]) {
    cout << "initialising orb-slam system..." << endl;

    slam = new ORB_SLAM3::System("", "", ORB_SLAM3::System::MONOCULAR, false);
    
    return 0;
}

extern "C" EMSCRIPTEN_KEEPALIVE bool receiveData(uint8_t *ptr, int width, int height) {

    cout << "receiveData()" << endl;
    auto cv_image = cv::Mat(width, height, CV_8UC4, ptr);
    
    cv::Mat gray_image;
    
    // From PTAM code (slam_system.cc)
    // TODO - Not sure if this is needed here
//    cv::cvtColor(cv_image, cv_image, cv::COLOR_BGR2RGB);

    cv::cvtColor(cv_image, gray_image, cv::COLOR_RGB2GRAY);
    
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> dur = t1.time_since_epoch();
    slam->TrackMonocular(gray_image, dur.count());
}

// simulate the 'space press'...
extern "C" EMSCRIPTEN_KEEPALIVE void captureKeyFrame() {
    cout << "### orb_wasm.cpp: simulating space press" << endl;
}

extern "C"   EMSCRIPTEN_KEEPALIVE void cleanup() {
    slam->Shutdown();
    delete slam;
}

