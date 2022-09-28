#pragma once

#include <algorithm>
#include <vector>
#include <map>

namespace tinykit {
	using namespace std;

	template <class K, class T>
	class OrderMap {
	private:
		vector<K> mKeys;
		map<K, T> mData;
	public:
		OrderMap() {}
		OrderMap(const OrderMap& other) {
			*this = other;
		}
		inline T &at(int index) {
			return mData[mKeys[index]];
		}

		inline int size() {
			return keys().size();
		}

		inline T &front() {
			return mData[mKeys.front()];
		}

		inline T &back() {
			return mData[mKeys.back()];
		}

		inline void insert(const K &key, const T &value) {
			if (!mKeys.contains(key)) {
				mKeys.push_back(key);
			}
			mData[key] = value;
		}
		inline void remove(const K &key) {
			mKeys.removeOne(key);
			mData.remove(key);
		}
		inline bool contains(const K &key)const {
			return mData.find(key) != mData.end();
		}
		inline void clear() {
			mKeys.clear();
			mData.clear();
		}

		inline T &operator[](const K& key) {
			if (std::find(mKeys.begin(), mKeys.end(),key) == mKeys.end()) {
				mKeys.push_back(key);
			}
			return mData[key];
		}
		inline const T operator[](const K& key)const {
			return mData[key];
		}
		inline const T value(const K &key)const {
			return mData[key];
		}
		inline const K &key(int index) const {
			return mKeys[index];
		}

		inline const vector<K> &keys() {
			return mKeys;
		}

	};
}
