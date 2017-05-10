// THIS file is almost a direct copy of the ofxLiquidEvent.h class
#pragma once

#include <map>
#include <vector>
#include "lambda.h"

namespace ofxCMS {
	template<class ArgType>
	class Middleware {
	public:
		//this used to be
		/*
		 typedef public FUNCTION<void(ArgType&)> Functor;
		 typedef public FUNCTION<void()> VoidFunctor;
		 not sure why... but anyway that doesn't work on Xcode 6
		 */
		typedef FUNCTION<bool(ArgType&)> Functor;
		typedef FUNCTION<bool()> VoidFunctor;
		typedef int32_t IndexType; // use negative index for bottom of stack
		struct Index {
			Index(IndexType order, void* owner) {
				this->order = order;
				this->owner = owner;
			}
			bool operator<(const Index& other) const {
				return this->order < other.order;
			}
			bool operator==(const IndexType& otherIndex) const {
				return this->order == otherIndex;
			}
			IndexType order;
			void* owner;
		};
		typedef std::map<Index, Functor> FunctorMap;
		typedef std::pair<Index, Functor> Pair;

		void operator+=(Functor functor) {
			this->addListener(functor, 0);
		}

		void addListener(Functor functor, void* owner) {
			IndexType order = 0;
			if (!this->listeners.empty()) {
				//loop until we find a free index
				while (listeners.find(Index(order, 0)) != listeners.end()) {
					order++;
				}
			}
			this->listeners.insert(Pair(Index(order, owner), functor));
		}

		void addListener(Functor functor, void* owner, IndexType order) {
			//loop until we find a free index
			while (listeners.find(Index(order, 0)) != listeners.end()) {
				order++;
			}
			this->listeners.insert(Pair(Index(order, owner), functor));
		}

		void removeListeners(void* owner) {
		std::vector<IndexType> toRemove;
			for(auto iterator : this->listeners) {
				if (iterator.first.owner == owner) {
					toRemove.push_back(iterator.first.order);
				}
			}
			for(auto order : toRemove) {
				this->listeners.erase(Index(order, owner));
			}
		}

		bool notifyListeners(ArgType& arguments) {
			for (auto listener : this->listeners) {
				if(listener.second(arguments) == false)
					return false;
			}

	        return true;
		}

		/// Warning : You will not be able to call this if ArgType does not have a default public constructor
		bool notifyListeners() {
			ArgType dummyArguments;
			for (auto listener : this->listeners) {
	            if(listener.second(dummyArguments) == false)
	                return false;
			}

	        return true;
		}

		/// Useful for mouse action stacks where last is top (first)
		bool notifyListenersInReverse(ArgType& arguments) {
			auto it = this->listeners.rbegin();
			for (; it != this->listeners.rend(); it++) {
				if(it->second(arguments) == false)
	                return false;
			}

	        return true;
		}

		bool operator()(ArgType& arguments) {
			return this->notifyListeners(arguments);
		}

		bool empty() const {
			return this->listeners.empty();
		}

		size_t size() const {
			return this->listeners.size();
		}

		const FunctorMap & getListeners() const {
			return this->listeners;
		}

		void clear() {
			this->listeners.clear();
		}

	protected:
		FunctorMap listeners;
	};


	//--
	//Specialization for Middleware<void>
	//--
	//
	//unhappily, we have to write the code out again 1:1 with the different type
	//since void is incompatible with some of the function templates above (e.g. use of ArgType&)
	//
	//
	/// NOTE : you can't call onMyEvent(), you must use onMyEvent.notifyListeners();
	template<>
	class Middleware<void> {
		typedef FUNCTION<bool()> Functor;
		typedef Middleware<int>::IndexType IndexType;
		typedef Middleware<int>::Index Index;
		typedef std::map<Index, Functor> FunctorMap;
		typedef std::pair<Index, Functor> Pair;
	public:
		void operator+=(Functor functor) {
			this->addListener(functor, 0);
		}

		void addListener(Functor functor, void* owner) {
			IndexType order = 0;
			if (!this->listeners.empty()) {
				//loop until we find a free index
				while (listeners.find(Index(order, 0)) != listeners.end()) {
					order++;
				}
			}
			this->listeners.insert(Pair(Index(order, owner), functor));
		}

		void addListener(Functor functor, IndexType order, void* owner) {
			//loop until we find a free index
			while (listeners.find(Index(order, 0)) != listeners.end()) {
				order++;
			}
			this->listeners.insert(Pair(Index(order, owner), functor));
		}

		void removeListeners(void* owner) {
			std::vector<IndexType> toRemove;
			for (auto iterator : this->listeners) {
				if (iterator.first.owner == owner) {
					toRemove.push_back(iterator.first.order);
				}
			}
			for (auto order : toRemove) {
				this->listeners.erase(Index(order, owner));
			}
		}

		bool notifyListeners() {
			for (auto listener : this->listeners) {
				if(listener.second() == false)
	                return false;
			}

	        return true;
		}

		/// Useful for mouse action stacks where last is top (first)
		bool notifyListenersInReverse() {
			auto it = this->listeners.rbegin();
			for (; it != this->listeners.rend(); it++) {
				if(it->second() == false)
	                return false;
			}

	        return true;
		}

		bool empty() const {
			return this->listeners.empty();
		}


		size_t size() const {
			return this->listeners.size();
		}

		const FunctorMap & getListeners() const {
			return this->listeners;
		}

		void clear() {
			this->listeners.clear();
		}
	protected:
		FunctorMap listeners;
	};
}
