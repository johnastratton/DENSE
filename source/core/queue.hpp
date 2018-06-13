#ifndef CORE_QUEUE_HPP
#define CORE_QUEUE_HPP

#include "util/Real.hpp"

#include <iostream>
#include <vector>


class Queue {

private:
	std::vector<RATETYPE> contents;
	int start,end,current;
  int size;

public:
//	Queue();

	Queue(unsigned length){
		contents.resize(length + 1);
		size = length + 1;
		start = 0;
		current = -1;
		end = -1;
	}
/*
	void populate(RATETYPE transferArray[]){

		if (isEmpty()){
			for (int i = 0; i < size/2; ++i){
				contents[i] = transferArray[i];
			}
			start = 0;
			midpoint = size/4;
			end = (size/2)-1;
		}
	}

	bool isEmpty(){
		return (midpoint == -1);
	}
*/
	int getSize() { return (size + end - start) % size + 1; }

	void enqueue(RATETYPE entry){
		if (end == (size-1)){
			end = 0;
		}
		else {
			++end;
		}
		contents[end] = entry;

		if (2*getSize() >= size) {
			++current;
			if (current == size) {
				current = 0;
			}
		}
	}

	RATETYPE dequeue(){
		RATETYPE popped = contents[start];
		if (start == (size-1)){
			start = 0;
		}
		else {
			++start;
		}
		if (2*getSize() <= size) {
			++current;
			if (current == size) {
				current = 0;
			}
		}
		return popped;
	}

	RATETYPE getVal(int index){
		return contents[index];
	}

	RATETYPE getCurrent(){
		return current;
	}

	friend std::ostream & operator << (std::ostream & out, Queue & queue) {
		out << '[' << queue.contents[0];
		for (int i = 1; i < queue.size; ++i) {
			out << ',' << queue.contents[i];
		}
		return out << "]\n";
	}
};

#endif
