// THIS FILE IS TAKEN DIRECTLY FROM:
// https://github.com/elliotwoods/ofxLiquidEvent

#pragma once

#include "function.h"
#include <map>
#include <vector>

template<class ArgType>
class ofxLiquidEvent {
public: // types & constants
	//this used to be
	/*
	 typedef public FUNCTION<void(ArgType&)> Functor;
	 typedef public FUNCTION<void()> VoidFunctor;
	 not sure why... but anyway that doesn't work on Xcode 6
	 */
	typedef FUNCTION<void(ArgType&)> Functor;
	typedef FUNCTION<void()> VoidFunctor;
	typedef int32_t IndexType; // use negative index for bottom of stack

	class Modification {
	public:
		Functor func;
		void* owner;
		IndexType order;
		bool add;

		Modification(void* ownerToRemove) : owner(ownerToRemove), add(false){}
		Modification(Functor _func, void* _owner, IndexType _order) : func(_func), owner(_owner), order(_order), add(true){}
	};

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


public: // methods

	ofxLiquidEvent() : lockCounter(0){}

	void operator+=(Functor functor) {
		this->addListener(functor, 0, 0);
	}

	void addListener(Functor functor, void* owner) {
		addListener(functor, owner, 0);
	}

	void addListener(Functor functor, void* owner, IndexType order) {
		if(isLocked()){
			// queue modification until we unlock
			modificationRefs.push_back(make_shared<Modification>(functor, owner, order));
			return;
		}

		//loop until we find a free index
		while (listeners.find(Index(order, 0)) != listeners.end()) {
			order++;
		}

		this->listeners.insert(Pair(Index(order, owner), functor));
	}

	void removeListeners(void* owner) {
		if(isLocked()){
			// queue modification until we unlock
			modificationRefs.push_back(make_shared<Modification>(owner));
			return;
		}

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

	void notifyListeners(ArgType& arguments) {
		lock([&](){
			for (auto listener : this->listeners) {
				listener.second(arguments);
			}
		});
	}

	/// Warning : You will not be able to call this if ArgType does not have a default public constructor
	void notifyListeners() {
		ArgType dummyArguments;
		notifyListeners(dummyArguments);
	}

	/// Useful for mouse action stacks where last is top (first)
	void notifyListenersInReverse(ArgType& arguments) {
		lock([&](){
			auto it = this->listeners.rbegin();
			for (; it != this->listeners.rend(); it++) {
				it->second(arguments);
			}
		});
	}

	void operator()(ArgType& arguments) {
		this->notifyListeners(arguments);
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

	bool isLocked(){ return lockCounter > 0; }
	void lock(FUNCTION<void(void)> func){
		lockCounter++;
		func();
		lockCounter--;

		// locks can be nested!
		if(isLocked())
			return;

		// process queue
		for(auto modRef : modificationRefs){
			if(modRef->add){
				addListener(modRef->func, modRef->owner, modRef->order);
			} else {
				removeListeners(modRef->owner);
			}
		}

		// clear queue
		modificationRefs.clear();
	}

protected:

	FunctorMap listeners;

private:
	int lockCounter;
	std::vector<shared_ptr<Modification>> modificationRefs;
};


//--
//Specialization for ofxLiquidEvent<void>
//--
//
//unhappily, we have to write the code out again 1:1 with the different type
//since void is incompatible with some of the function templates above (e.g. use of ArgType&)
//
//
/// NOTE : you can't call onMyEvent(), you must use onMyEvent.notifyListeners();
template<>
class ofxLiquidEvent<void> {
	typedef FUNCTION<void()> Functor;
	typedef ofxLiquidEvent<int>::IndexType IndexType;
	typedef ofxLiquidEvent<int>::Index Index;
	typedef std::map<Index, Functor> FunctorMap;
	typedef std::pair<Index, Functor> Pair;
public:
	void operator+=(Functor functor) {
		this->addListener(functor, 0, 0);
	}

	void addListener(Functor functor, void* owner) {
		addListener(functor, owner, 0);
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
		for (auto iterator : this->listeners) {
			if (iterator.first.owner == owner) {
				toRemove.push_back(iterator.first.order);
			}
		}
		for (auto order : toRemove) {
			this->listeners.erase(Index(order, owner));
		}
	}

	void notifyListeners() {
		for (auto listener : this->listeners) {
			listener.second();
		}
	}

	/// Useful for mouse action stacks where last is top (first)
	void notifyListenersInReverse() {
		auto it = this->listeners.rbegin();
		for (; it != this->listeners.rend(); it++) {
			it->second();
		}
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
//
//--
