#pragma once

#include <chrono>
#include <list>
#include <string>
#include "OrderMap.h"

using namespace std;

namespace tinykit{
    class Clock
    {
    public:
        Clock(const string& name);
        virtual ~Clock();
		void start();
		void end();
		void split(const string& tip);
		void reset();
		list<int> durationMilliseconds();
		list<double> durationSeconds();
		string summary();
	private:
		string name_;
		OrderMap<string , std::chrono::steady_clock::time_point> points_;
    };
};

