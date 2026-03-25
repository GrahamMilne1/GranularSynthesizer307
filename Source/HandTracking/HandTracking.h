#pragma once
#include <atomic>
#include <thread>
#include <opencv2/opencv.hpp>

class HandTracking
{
    public:
        // constructor
        HandTracking();

        // gesture types
        enum class Gesture
        {
            NONE,
            FIST,
            PALM,
            PINCH
        };

        std::atomic<float> handY;
        std::atomic<float> handX;
        std::atomic<float> pinchDistance;
        std::atomic<bool> isTracking;
        std::atomic<Gesture> currentGesture;

        // getters
        cv::Mat getImage();

        // destructor
        ~HandTracking();

    private:
        // thread stuff
        std::thread HTThread;
        std::atomic<bool> isRunning;

        cv::VideoCapture cap;

        // image to be rendered
        cv::Mat latestImage;
        std::mutex mtx;

        // methods
        void trackingLoop();
};
