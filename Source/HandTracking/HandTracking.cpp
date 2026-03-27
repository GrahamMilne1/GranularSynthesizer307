#include "HandTracking.h"
#include <chrono>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <algorithm>

HandTracking::HandTracking() {
    // constructor

    // set defaults
    handY = 0.5f;
    handX = 0.5f;
    pinchDistance = 0.5f;
    currentGesture = Gesture::NONE;
    candidateGesture = Gesture::NONE;
    gestureFrameCount = 0;

    missedFrames = 0;

    // open camera
    cap.open(1);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

    // load hand tracking models
    palmNet = cv::dnn::readNetFromONNX("/Users/grahammilne/GranularSynth/Source/HandTracking/models/palm_detection_mediapipe_2023feb.onnx");
    handPoseNet = cv::dnn::readNetFromONNX("/Users/grahammilne/GranularSynth/Source/HandTracking/models/handpose_estimation_mediapipe_2023feb.onnx");

    // grid 1 (24 x 24)
    for (int p = 0; p < 576; p++) {
        int col = p % 24;
        int row = p / 24;
        anchors.push_back(std::make_pair((col + 0.5) / 24, (row + 0.5) / 24));
        anchors.push_back(std::make_pair((col + 0.5) / 24, (row + 0.5) / 24));
    }

    for (int p = 0; p < 144; p++) {
        int col = p % 12;
        int row = p / 12;
        for (int i = 0; i < 6; i++) {
            anchors.push_back(std::make_pair((col + 0.5) / 12, (row + 0.5) / 12));
        }
    }

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
            std::this_thread::sleep_for(std::chrono::milliseconds(33));
            cv::Mat frame;
            cap.read(frame);

            if (frame.empty())
                continue;
            
            // flip video output so not mirrored
            cv::flip(frame, frame, 1);

            cv::Mat display = frame.clone();

            // local detected gesture
            Gesture detectedGesture = Gesture::NONE;

            // preprocessing for palm tracking network
            cv::resize(frame, frame, cv::Size(192, 192));
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            frame.convertTo(frame, CV_32F, 1.0/255);
            std::vector<int> shape = {1, 192, 192, 3};
            cv::Mat blob(shape, CV_32F, frame.data);

            // feed into palm tracking network
            palmNet.setInput(blob);
            std::vector<cv::Mat> outputs;
            std::vector<std::string> outNames = palmNet.getUnconnectedOutLayersNames();
            palmNet.forward(outputs, outNames);

            cv::Mat regressors = outputs[0].reshape(1, 2016);
            cv::Mat scores = outputs[1].reshape(1, 2016);

            std::vector<cv::Rect> boxes;
            std::vector<float> confidences;

            for (int i = 0; i < 2016; i++) {
                // apply sigmoid function to scores
                float temp_confidence = 1.0 / (1.0 + exp(-(scores.at<float>(i, 0))));
                
                if (temp_confidence > 0.7) {
                    // get and convert box dimensions 
                    int w = regressors.at<float>(i, 2) / 192.0f * 640;
                    int h = regressors.at<float>(i, 3) / 192.0f * 480;

                    int cx = (anchors[i].first + regressors.at<float>(i, 0) / 192.0f) * 640;
                    int cy = (anchors[i].second + regressors.at<float>(i, 1) / 192.0f) * 480;

                    
                    // std::cout << "cx: " << cx << " cy: " << cy << " w: " << w << " h: " << h << std::endl;

                    // push to box vector
                    boxes.push_back(cv::Rect(cx - w / 2, cy - h / 2, w, h));
                    confidences.push_back(temp_confidence);
                }
            }

            // NMS
            std::vector<int> indices;
            cv::dnn::NMSBoxes(boxes, confidences, 0.5, 0.3, indices);

            // draw tracking boxes and crop to track fingers
            for (int i = 0; i < indices.size(); i++) {
                isTracking = true;
                missedFrames = 0;

                cv::rectangle(display, boxes.at(indices[i]), cv::Scalar(0, 0, 255), 2);

                // expand palm box so finger tracking actually sees the fingers

                int centerX = boxes.at(indices[i]).x + boxes.at(indices[i]).width / 2;
                int centerY = boxes.at(indices[i]).y + boxes.at(indices[i]).height / 2;

                int expandedWidth = boxes.at(indices[i]).width * 3;
                int expandedHeight = boxes.at(indices[i]).height * 3;
                int expandedX = centerX - (expandedWidth / 2);
                int expandedY = centerY - (expandedHeight/ 2);

                cv::Rect cropArea(expandedX, expandedY , expandedWidth , expandedHeight);
                cv::Rect frameBounds(0, 0, 640, 480);
                cv::Rect safeArea = cropArea & frameBounds;
                cv::Mat croppedFrame = display(safeArea);

                // preprocessing for hand pose tracking
                cv::resize(croppedFrame, croppedFrame, cv::Size(224, 224));
                cv::cvtColor(croppedFrame, croppedFrame, cv::COLOR_BGR2RGB);
                croppedFrame.convertTo(croppedFrame, CV_32F, 1.0/255);
                std::vector<int> shape = {1, 224, 224, 3};
                cv::Mat blob(shape, CV_32F, croppedFrame.data);

                // run hand pose tracking model
                handPoseNet.setInput(blob);
                std::vector<cv::Mat> poseOutputs;
                std::vector<std::string> poseOutNames = handPoseNet.getUnconnectedOutLayersNames();
                handPoseNet.forward(poseOutputs, poseOutNames);

                std::array<cv::Point2f, 21> lmarkPositions;

                for (int j = 0; j < 21; j++) {
                    int lmarkX = ((poseOutputs[0].at<float>(0, j*3)) / 224.0f) * safeArea.width + safeArea.x;
                    int lmarkY = ((poseOutputs[0].at<float>(0, j*3 + 1)) / 224.0f) * safeArea.height + safeArea.y;

                    lmarkPositions[j].x = lmarkX;
                    lmarkPositions[j].y = lmarkY;
                    // paint circles
                    cv::circle(display, cv::Point(lmarkX, lmarkY), 2, cv::Scalar(0, 0, 255), 2);
                }

                // gesture classification logic
                // this is a dumb way of doing this

                // euclidian distance between thumb and index
                pinchDistance = std::sqrt(std::pow(lmarkPositions[4].x - lmarkPositions[8].x, 2) + std::pow(lmarkPositions[4].y - lmarkPositions[8].y, 2));
                float localPD = pinchDistance;

                // remap to account for hand not reaching edge of screen
                handX = std::clamp((((lmarkPositions[9].x / 640.0) - 0.15) / (0.85 - 0.15)), 0.0, 1.0);
                handY = std::clamp((((lmarkPositions[9].y / 480.0) - 0.15) / (0.75 - 0.15)), 0.0, 1.0);

                // margin to make extended vs curled more distinct
                int margin = std::sqrt(std::pow(lmarkPositions[5].x - lmarkPositions[17].x, 2) + std::pow(lmarkPositions[5].y - lmarkPositions[17].y, 2)) * 0.4;

                // finger extension amounts
                float index = std::sqrt(std::pow(lmarkPositions[8].x - lmarkPositions[0].x, 2) + std::pow(lmarkPositions[8].y - lmarkPositions[0].y, 2));
                float middle = std::sqrt(std::pow(lmarkPositions[12].x - lmarkPositions[0].x, 2) + std::pow(lmarkPositions[12].y - lmarkPositions[0].y, 2));
                float ring = std::sqrt(std::pow(lmarkPositions[16].x - lmarkPositions[0].x, 2) + std::pow(lmarkPositions[16].y - lmarkPositions[0].y, 2));
                float pinky = std::sqrt(std::pow(lmarkPositions[20].x - lmarkPositions[0].x, 2) + std::pow(lmarkPositions[20].y - lmarkPositions[0].y, 2));


                std::cout << "Hand x: " << handX << " Hand y: " << handY << " margin: " << margin << " index - middle - ring - pinky: " << index << " " << middle << " " << ring << " " << pinky << std::endl;

                if (
                    localPD < 20 &&
                    middle > 50.0 &&
                    ring > 50.0)
                {
                    detectedGesture = Gesture::PINCH;
                }
                else if (
                    index > middle * 1.2 &&
                    index > ring * 1.2 &&
                    index > pinky * 1.2 
                )
                {
                    detectedGesture = Gesture::POINT;
                }
                else if (
                    lmarkPositions[8].y < lmarkPositions[5].y - margin &&
                    lmarkPositions[12].y < lmarkPositions[9].y  - margin &&
                    lmarkPositions[16].y < lmarkPositions[13].y - margin &&
                    lmarkPositions[20].y < lmarkPositions[17].y - margin)
                {
                    detectedGesture = Gesture::PALM;
                }
                else if (
                    index < 100 &&
                    middle < 100 &&
                    ring < 100 && 
                    pinky < 100
                ) {
                    detectedGesture = Gesture::FIST;
                }
                else
                {
                    detectedGesture = Gesture::NONE;
                }

                // increment frame count if gesture remains the same
                if (detectedGesture == candidateGesture) {
                    gestureFrameCount += 2;
                } else {
                    gestureFrameCount--;
                }

                if (gestureFrameCount >= 7) {
                    currentGesture = candidateGesture;
                    gestureFrameCount = 7;
                }
                else if (gestureFrameCount <= 0)
                {
                    candidateGesture = detectedGesture;
                    gestureFrameCount = 1;
                }

                std::cout << "detected Gesture: " << static_cast<int>(detectedGesture) << " candidate Gesture: " << static_cast<int>(candidateGesture) << "Local PD: " << localPD << " landmarks 8 - 5 12 - 9 16 - 13 20 - 17" << lmarkPositions[8].y << " " << lmarkPositions[5].y << " " << lmarkPositions[12].y << " " << lmarkPositions[9].y << " " << lmarkPositions[16].y << " " << lmarkPositions[13].y << " " << lmarkPositions[20].y << " " << lmarkPositions[17].y << std::endl;

                // display current gesture
                std::string gestureLabel;
                switch (currentGesture)
                {
                case Gesture::PINCH:
                    gestureLabel = "Gesture: Pinch";
                    break;
                case Gesture::POINT:
                    gestureLabel = "Gesture: Point";
                    break;
                case Gesture::PALM:
                    gestureLabel = "Gesture: Palm";
                    break;
                case Gesture::FIST:
                    gestureLabel = "Gesture: Fist";
                    break;
                default:
                    gestureLabel = "Gesture: None";
                };
                cv::rectangle(display, cv::Rect(5, 5, 320, 40), cv::Scalar(0, 0, 0), cv::FILLED);
                cv::putText(display, gestureLabel, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0));

                // copy frame into lastestImage to be rendered
            }
            {
                std::lock_guard<std::mutex> lock(mtx);
                latestImage = display;
            }
            if ( indices.size() == 0 ) {
                missedFrames++;
            }

            if (missedFrames >= 7) {
                isTracking = false;
            }
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