// QR code reader.
//
// Code copied from "https://rest-term.com/archives/3491/"
//
#include "pch.h"

#include <iostream>
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

#define CLASS extern
#include "usbCamera.h"
#include "DDconfig.h"

int QRreader(char* qr_file)
{
    using namespace std;

    const string file_name = qr_file;
    const cv::Mat input_image = cv::imread(file_name, cv::IMREAD_COLOR);
    cv::Mat output_image = input_image.clone();
    vector<cv::Point> points;
    cv::Mat straight_qrcode;
    // QR code detector.
    cv::QRCodeDetector detector;
    // Detect QR code and decode.
    const string data = detector.detectAndDecode(input_image, points, straight_qrcode);
    if (data.length() > 0) {
        // Output docoded data.
        cout << "decoded data: " << data << endl;
        // 検出結果の矩形描画
        for (size_t i = 0; i < points.size(); ++i) {
            cv::line(output_image, points[i], points[(i + 1) % points.size()], cv::Scalar(0, 0, 255), 4);
        }
        cv::imwrite("output.png", output_image);
        // Output of version.
        cout << "QR code version: " << ((straight_qrcode.rows - 21) / 4) + 1 << endl;
    }
    else {
        cout << "QR code not detected" << endl;
    }
    return 0;
}

int QRreaderFromMat(cv::Mat input_image, char *attrText)
{
    using namespace std;

    cv::Mat output_image = input_image.clone();
    vector<cv::Point> points;
    cv::Mat straight_qrcode;
    // QR code detector.
    cv::QRCodeDetector detector;
    // Detect QR code and decode.
    const string data = detector.detectAndDecode(input_image, points, straight_qrcode);
    if (data.length() > 0) {
        // Output docoded data.
        cout << "decoded data: " << data << endl;
        // 検出結果の矩形描画
        for (size_t i = 0; i < points.size(); ++i) {
            cv::line(output_image, points[i], points[(i + 1) % points.size()], cv::Scalar(0, 0, 255), 4);
        }
        // Output of version.
        cout << "QR code version: " << ((straight_qrcode.rows - 21) / 4) + 1 << endl;
    }
    else {
        cout << "QR code not detected" << endl;
    }
    strcpy_s(attrText, 100, data.c_str());      // Copy QR string to attribute text buffer.

    // Anyway write output image.
    cv::imwrite("c:\\temp\\logs\\QRoutput.png", output_image);

    return 0;
}

int IMP_readQR(IMG0* src, char *attr)
{
//    cv::Mat orgImage(src->bih.biWidth, src->bih.biHeight, CV_8U, src->lpBmpData);
    cv::Mat orgImage(src->bih.biHeight, src->bih.biWidth, CV_8U, src->lpBmpData);
    QRreaderFromMat(orgImage, attr);

    return 0;
}
