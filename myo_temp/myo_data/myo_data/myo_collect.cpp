// Distributed under the Myo SDK license agreement. See LICENSE.txt for details.
// This sample illustrates how to use EMG data. EMG streaming is only supported for one Myo at a time.
#define _USE_MATH_DEFINES
#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <fstream>
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
		accx = static_cast<int>(accel.x());
		accy = static_cast<int>(accel.y());
		accz = static_cast<int>(accel.z());
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
	void write()
	{
		std::ofstream ofile;
		ofile.open("f:\\Data.csv", std::ios::out | std::ios::app);
		ofile << "emgData:" << ",";
		for (size_t i = 0; i < emgSamples.size(); i++)
		{
			std::ostringstream oss;
			oss << static_cast<int>(emgSamples[i]);
			std::string emgString = oss.str();
			ofile << emgString << ",";
		}
		ofile << "OrientationData" << "," << "roll_w:"<< roll_w << ","<< "pitch_w:" << pitch_w << "," << "yaw_w :" << yaw_w << "," ;
		ofile << "AccelerometerData" << "," << "xdir:" << accx << "," << "ydir:" << accy << "," << "zdir:" << accz << ",";
		ofile << "GyroscopeData" << "," << "xdir:" << gyrx << "," << "ydir:" << gyry << "," << "zdir:" << gyrz << std::endl;
		ofile.close();
	}
	// The values of these variable is set by on...() function above.
	std::array<int8_t, 8> emgSamples;
	int roll_w, pitch_w, yaw_w;
	int accx, accy, accz;
	int gyrx, gyry, gyrz;
};
int main(int argc, char** argv)
{
	// We catch any exceptions that might occur below -- see the catch statement for more details.
	try {
		// First, we create a Hub with our application identifier. Be sure not to use the com.example namespace when
		// publishing your application. The Hub provides access to one or more Myos.
		myo::Hub hub("com.example.emg-data-sample");
		std::cout << "Attempting to find a Myo..." << std::endl;
		// Next, we attempt to find a Myo to use. If a Myo is already paired in Myo Connect, this will return that Myo
		// immediately.
		// waitForMyo() takes a timeout value in milliseconds. In this case we will try to find a Myo for 10 seconds, and
		// if that fails, the function will return a null pointer.
		myo::Myo* myo = hub.waitForMyo(10000);
		// If waitForMyo() returned a null pointer, we failed to find a Myo, so exit with an error message.
		if (!myo) {
			throw std::runtime_error("Unable to find a Myo!");
		}
		// We've found a Myo.
		std::cout << "Connected to a Myo armband!" << std::endl << std::endl;
		// Next we enable EMG streaming on the found Myo.
		myo->setStreamEmg(myo::Myo::streamEmgEnabled);
		// Next we construct an instance of our DeviceListener, so that we can register it with the Hub.
		DataCollector collector;
		// Hub::addListener() takes the address of any object whose class inherits from DeviceListener, and will cause
		// Hub::run() to send events to all registered device listeners.
		hub.addListener(&collector);
		// Finally we enter our main loop.
		while (1) {
			// In each iteration of our main loop, we run the Myo event loop for a set number of milliseconds.
			// In this case, we wish to update our display 100 times a second, so we run for 1000/10 milliseconds.
			hub.run(1000 / 10);
			// After processing events, we call the print() member function we defined above to print out the values we've
			// obtained from any events that have occurred.
			collector.print();
			//collector.write();
			collector.write();
		}
		// If a standard exception occurred, we print out its message and exit.
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Press enter to continue.";
		std::cin.ignore();
		return 1;
	}
}