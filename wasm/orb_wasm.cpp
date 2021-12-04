// Start of emscripten ORB-SLAM experiment.

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

        
extern "C" {
      EMSCRIPTEN_KEEPALIVE bool receiveData(uint8_t *ptr, int width, int height);
      EMSCRIPTEN_KEEPALIVE void cleanup();
      EMSCRIPTEN_KEEPALIVE void captureKeyFrame();
}

vector<double> mapPoints;
vector<double> getMapPoints();

EMSCRIPTEN_BINDINGS(map_points) {
    emscripten::function("getMapPoints", &getMapPoints);
    emscripten::register_vector<double>("vector<double>");
}



ORB_SLAM3::System *slam;


bool isProcessingFrame = false;

int main(int argc, char *argv[]) {
    cout << "initialising orb-slam system..." << endl;

    slam = new ORB_SLAM3::System("assets/ORBvoc.txt", "assets/EuRoC.yaml", ORB_SLAM3::System::MONOCULAR, false);
    
    return 0;
}

extern "C" EMSCRIPTEN_KEEPALIVE bool receiveData(uint8_t *ptr, int width, int height) {
    if(isProcessingFrame) return false;
    isProcessingFrame = true;
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

    // Get map points, same general technique as for PTAM
    // This may well be from a previous frame, as it's thread-based
    mapPoints.clear();
    vector<ORB_SLAM3::MapPoint*> points = slam->GetTrackedMapPoints();
    cout << "Got " << points.size() << " tracked map points." << endl;
    mapPoints.reserve(points.size() * sizeof(double) * 3);
    for(int i=0; i<points.size(); i++) {
        cv::Mat worldPos = points[i]->GetWorldPos();
        for(int j=0; j<3; j++) {
            mapPoints.push_back(worldPos.at<double>(j));
        }
    }
    cout << "Map points to be returned:" << endl;
    for(int i=0; i<mapPoints.size(); i++) {
        cout << mapPoints[i] <<" " ;
    }
    cout << endl;
    isProcessingFrame = false;
    return true;
}

vector<double> getMapPoints() {
    return mapPoints;
}

extern "C"   EMSCRIPTEN_KEEPALIVE void cleanup() {
    slam->Shutdown();
    delete slam;
}

