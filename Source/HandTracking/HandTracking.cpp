#include "HandTracking.h"
#include <chrono>
#include <opencv2/opencv.hpp>
#include <iostream>

HandTracking::HandTracking() {
    // constructor

    // set defaults
    handY = 0.5f;
    handX = 0.5f;
    pinchDistance = 0.5f;
    currentGesture = Gesture::NONE;

    // open camera
    cap.open(1);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);


    // start thread
    isRunning = true;
    HTThread = std::thread(&HandTracking::trackingLoop, this);

}

HandTracking::~HandTracking() {
    // destructor
    isRunning = false;
    HTThread.join();
    cap.release();
}

void HandTracking::trackingLoop() {
    if (cap.isOpened()) {
        while (isRunning == true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            cv::Mat frame;
            cap.read(frame);

            // copy frame into lastestImage to be rendered
            std::lock_guard<std::mutex> lock(mtx);
            latestImage = frame;

            std::cout << "Captured Frame, Width: " << frame.cols << "Height: " << frame.rows << std::endl;
        }
    }
    else
    {
        std::cout << "Failed to open camera" << std::endl;
    }
}

cv::Mat HandTracking::getImage() {
    std::lock_guard<std::mutex> lock(mtx);
    cv::Mat localFrame = latestImage;
    return localFrame;
}