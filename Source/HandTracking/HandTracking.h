#pragma once
#include <atomic>
#include <thread>
#include <opencv2/opencv.hpp>
#include <cmath>

class HandTracking
{
    public:
        // constructor
        HandTracking();

        // gesture types
        enum class Gesture
        {
            NONE,
            POINT,
            PALM,
            PINCH,
            FIST
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

        // gesture tracking candidate and frame count
        Gesture candidateGesture;
        int gestureFrameCount;

        // total missed frames before dropping tracking
        int missedFrames;

        // image to be rendered
        cv::Mat latestImage;
        std::mutex mtx;

        // methods
        void trackingLoop();

        std::vector<std::pair<float, float>> anchors;

        // hand tracking model loading
        cv::dnn::Net palmNet;
        cv::dnn::Net handPoseNet;
};
