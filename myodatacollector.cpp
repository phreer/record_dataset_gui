/*
 * Author: Shifu Shen (GITHUB ID: a8228771)
 * Added Date: Mar.28.2018
 * Version 0.1.0
 * a class to collect data from myo
 */
#define M_PI 3.1415926535
#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <QTime>
#include <myo\myo.hpp>
class DataCollector : public myo::DeviceListener {
public:
    DataCollector()
        : emgSamples(),roll_w(0), pitch_w(0), yaw_w(0),accx(0),accy(0),accz(0),gyrx(0),gyry(0),gyrz(0)
    {
    }
    // onUnpair() is called whenever the Myo is disconnected from Myo Connect by the user.
    void onUnpair(myo::Myo* myo, uint64_t timestamp)
    {
        // We've lost a Myo.
        // Let's clean up some leftover state.
        emgSamples.fill(0);
        roll_w = 0;
        pitch_w = 0;
        yaw_w = 0;
        accx = 0;
        accy = 0;
        accz = 0;
        gyrx = 0;
        gyry = 0;
        gyrz = 0;
    }
    // onEmgData() is called whenever a paired Myo has provided new EMG data, and EMG streaming is enabled.
    void onEmgData(myo::Myo* myo, uint64_t timestamp, const int8_t* emg)
    {
        for (int i = 0; i < 8; i++) {
            emgSamples[i] = emg[i];
        }
    }
    // onAccelerometerData() is called when a paired Myo has provided new accelerometer data in units of g.
    void onAccelerometerData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3< float > &accel)
    {
        accx = static_cast<double>(accel.x());
        accy = static_cast<double>(accel.y());
        accz = static_cast<double>(accel.z());
    }
    // onGyroscopeDat() is called when a paired Myo has provided new gyroscope data in units of deg/s.
    void onGyroscopeData(myo::Myo* myo, uint64_t timestamp, const myo::Vector3<float>& gyro)
    {
        gyrx = static_cast<int>(gyro.x());
        gyry = static_cast<int>(gyro.y());
        gyrz = static_cast<int>(gyro.z());
    }
    // onOrientationData() is called whenever the Myo device provides its current orientation, which is represented
    // as a unit quaternion.
    void onOrientationData(myo::Myo* myo, uint64_t timestamp, const myo::Quaternion<float>& quat)
    {
        using std::atan2;
        using std::asin;
        using std::sqrt;
        using std::max;
        using std::min;
        // Calculate Euler angles (roll, pitch, and yaw) from the unit quaternion.
        float roll = atan2(2.0f * (quat.w() * quat.x() + quat.y() * quat.z()),
            1.0f - 2.0f * (quat.x() * quat.x() + quat.y() * quat.y()));
        float pitch = asin(max(-1.0f, min(1.0f, 2.0f * (quat.w() * quat.y() - quat.z() * quat.x()))));
        float yaw = atan2(2.0f * (quat.w() * quat.z() + quat.x() * quat.y()),
            1.0f - 2.0f * (quat.y() * quat.y() + quat.z() * quat.z()));
        // Convert the floating point angles in radians to a scale from 0 to 18.
        roll_w = static_cast<int>((roll + (float)M_PI) / (M_PI * 2.0f) * 18);
        pitch_w = static_cast<int>((pitch + (float)M_PI / 2.0f) / M_PI * 18);
        yaw_w = static_cast<int>((yaw + (float)M_PI) / (M_PI * 2.0f) * 18);
    }
    // We define this function to print the current values that were updated by the on...() functions above.
    void print()
    {
        // Clear the current line
        std::cout << '\r';
        // Print out the EMG data.
        for (size_t i = 0; i < emgSamples.size(); i++) {
            std::ostringstream oss;
            oss << static_cast<int>(emgSamples[i]);
            std::string emgString = oss.str();
            std::cout << '[' << emgString << std::string(4 - emgString.size(), ' ') << ']';
        }
        std::cout << std::flush;
    }
    // write experiment data into csv file
    void write(std::ofstream &ofile) {
        QTime currTime = QTime::currentTime();
        ofile <<currTime.msecsSinceStartOfDay() << ",";
        ofile << "emgData:" << ",";
        for (size_t i = 0; i < emgSamples.size(); i++){
            std::ostringstream oss;
            oss << static_cast<int>(emgSamples[i]);
            std::string emgString = oss.str();
            ofile << emgString << ",";
        }
        ofile << "OrientationData" << "," << "roll_w:"<< roll_w << ","<< "pitch_w:" << pitch_w << "," << "yaw_w :" << yaw_w << "," ;
        ofile << "AccelerometerData" << "," << "xdir:" << accx << "," << "ydir:" << accy << "," << "zdir:" << accz << ",";
        ofile << "GyroscopeData" << "," << "xdir:" << gyrx << "," << "ydir:" << gyry << "," << "zdir:" << gyrz << std::endl;
    }

    // The values of these variable is set by on...() function above.
    std::array<int8_t, 8> emgSamples;
    int roll_w, pitch_w, yaw_w;
    int accx, accy, accz;
    double gyrx, gyry, gyrz;
};
