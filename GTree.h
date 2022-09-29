#ifndef GTREE_H_
#define GTREE_H_

#include <iostream>
#include <cstdlib>
#include <cstring>
using namespace std;

#define uint uint32_t

// nodes are stored as E*, which is an array of values such that *(p + i) is the min value of the ith subtree, *((E**)p - i - 1) is the ith subtree, and *(uint*)(p + (1 << factor)) is the amount of real values

constexpr uint k = 8; // 2^k is the block size
constexpr uint d = 1; // nodes have a branching factor of 2^(d + k*2^h), where h is the node height. The default value is 1.
constexpr uint e = (3 << k) >> 2; // the minimum sum of real sizes for neighboring blocks

//stores the path, where start is the node and x is the index of the subtree went into

template<class E> struct T {
	E* start;
	uint x;
	T(E* s, uint i) {
		start = s;
		x = i;
	}
};
template<class E> void destruct(E* node, uint height) {
	if (height == 0) {
		free(node);
		return;
	}
	uint l = *(uint*)(node + (1 << ((k << (height - 1)) + d)));
	for (uint x = 1; x <= l; x++) {
		destruct(*((E**)node - x), height - 1);
	}
	free((E**)node - (1 << ((k << (height - 1)) + d)));
}

template<class E> class GTree {
public:
	E* start; // pointer to the root node
	uint length; // number of total elements in the tree
	uint curfactor; // 2^curfactor is the current physical size of the root
	uint levels; // the number of levels
	E* firstleaf;
	T<E>* path;
	GTree() {
		start = (E*)((char*)malloc((sizeof(E) + sizeof(E*) + 2) << 1) + (sizeof(E*) << 1));
		*(uint*)(start + 2) = 1;
		firstleaf = (E*)malloc((sizeof(E) << k) + 4);
		*(uint*)(firstleaf + (1 << k)) = 1;
		*((E**)start - 1) = firstleaf;
		length = 0;
		curfactor = 1;
		levels = 1;
		path = (T<E>*)malloc(sizeof(T<E>));
	}
	~GTree() {
		uint l = *(uint*)(start + (1 << curfactor));
		for (uint x = 1; x <= l; x++) {
			destruct(*((E**)start - x), levels - 1);
		}
		free((E**)start - (1 << curfactor));
		free(path);
	}
    uint size() {
        return length;
    }
    E min() {
        return *firstleaf;
    }
    E max() {
        E* cur = *((E**)start - *(uint*)(start + (1 << curfactor)));
        if (levels > 1) {
            for (uint factor = (k << (levels - 2)) + d; factor >= k + d; factor = (factor + d) >> 1) {
                cur = *((E**)cur - *(uint*)(cur + (1 << factor)));
            }
        }
        return *(cur + *(uint*)(cur + (1 << k)) - 1);
    }
	bool remove(E remove) {
		// same algorithm as contains, just also stores the path in an array
		const int levels = this->levels;
		int level = levels;
		T<E>* path = this->path;
		uint factor = (k << (levels - 1)) + d;
		E* cur = start;
		uint x = 1 << (curfactor - 1);
		uint l = *(uint*)(cur + (x << 1));
		for (;factor >= k + d; factor = (factor + d) >> 1, x = 1 << (factor - 1), l = *(uint*)(cur + (1 << factor))) {
			switch (x) {
			case 1 << 16:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 15) : x + (1 << 15));
			case 1 << 15:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 14) : x + (1 << 14));
			case 1 << 14:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 13) : x + (1 << 13));
			case 1 << 13:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 12) : x + (1 << 12));
			case 1 << 12:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 11) : x + (1 << 11));
			case 1 << 11:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 10) : x + (1 << 10));
			case 1 << 10:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 9) : x + (1 << 9));
			case 1 << 9:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 8) : x + (1 << 8));
			case 1 << 8:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 7) : x + (1 << 7));
			case 1 << 7:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 6) : x + (1 << 6));
			case 1 << 6:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 5) : x + (1 << 5));
			case 1 << 5:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 4) : x + (1 << 4));
			case 1 << 4:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 3) : x + (1 << 3));
			case 1 << 3:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 2) : x + (1 << 2));
			case 1 << 2:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 1) : x + (1 << 1));
			case 1 << 1:
				x = (x >= l || remove < *(cur + x) ? x - (1 << 0) : x + (1 << 0));
			}
			x = (x >= l || remove < *(cur + x) ? x - 1 : x);
			*(path + --level) = T<E>(cur, x);
			cur = *((E**)cur - (x + 1));
		}
		l = *(uint*)(cur + (1 << k));
		x = 1 << (k - 1);
		switch (k) {
		case 9:
			x = (x >= l || remove < *(cur + x) ? x - (1 << 7) : x + (1 << 7));
		case 8:
			x = (x >= l || remove < *(cur + x) ? x - (1 << 6) : x + (1 << 6));
		case 7:
			x = (x >= l || remove < *(cur + x) ? x - (1 << 5) : x + (1 << 5));
		case 6:
			x = (x >= l || remove < *(cur + x) ? x - (1 << 4) : x + (1 << 4));
		case 5:
			x = (x >= l || remove < *(cur + x) ? x - (1 << 3) : x + (1 << 3));
		case 4:
			x = (x >= l || remove < *(cur + x) ? x - (1 << 2) : x + (1 << 2));
		case 3:
			x = (x >= l || remove < *(cur + x) ? x - (1 << 1) : x + (1 << 1));
		case 2:
			x = (x >= l || remove < *(cur + x) ? x - (1 << 0) : x + (1 << 0));
		}
		x = (x >= l || remove < *(cur + x) ? x - 1 : x);
		if (x >= l || (x == 0 && cur == firstleaf) || remove > *(cur + x)) { // if the value isn't in the tree
			return false;
		}
		length--;
		uint ol = l;
		uint y = x;
		E* other;
		factor = k + d; //from here
		for (level = -1; y == 0 && l == 1 && level < levels - 1; y = (path + ++level)->x, l = *(uint*)((path + level)->start + (1 << (level == levels - 1 ? curfactor : factor))),  factor = (factor << 1) - d);
		if (y == 0 && l > 1) {
			E p;
			if (level != -1) {
				p = *((path + level)->start + 1);
			}
			else {
				p = *(cur + 1);
			}
			for (;++level < levels && y == 0;) {
				y = (path + level)->x;
				*((path + level)->start + y) = p;
			}
		} // to here just resets min values that need resetting because the old one got deleted
		l = *(uint*)(cur + (1 << k)); // the real length of the leaf containing the element
		y = path->x; // index of the block containing the element in the node with height 0
		if (l == 1) { // if the block will be empty after the deletion, it should just be removed
			free(cur);
			goto done;
		}
		if (y != 0) { //checking if the block should be merged with the block before it
			other = *((E**)path->start - y); //the block before
			ol = *(uint*)(other + (1 << k)); //  the block before's real size
			if (l + ol <= e + 1) {
				*(uint*)(other + (1 << k)) = l + ol - 1;
				memcpy(other + ol, cur, sizeof(E) * x);
				memcpy(other + (ol + x), cur + (x + 1), sizeof(E) * (l - (x + 1)));
				free(cur);
				goto done;
			}
		}
		if (y != (levels != 1 ? *(uint*)(path->start + (1 << (k + d))) : *(uint*)(start + (1 << curfactor))) - 1) { //checking if the block should be merged with the block after it
			other = *((E**)path->start - (y + 2)); //the block after it
			ol = *(uint*)(other + (1 << k)); //the block after's real size
			if (l + ol <= e + 1) {
				*(uint*)(cur + (1 << k)) = l + ol - 1;
				memmove(cur + x, cur + x + 1, sizeof(E) * (l - (x + 1)));
				memcpy(cur + (l - 1), other, sizeof(E) * ol);
				free(other);
				path->x++; // makes it so that other will be deleted in the 1st level node's array instead of cur
				goto done;
			}
		}
		//this is for if the blocks don't merge
		--*(uint*)(cur + (1 << k));
		memmove(cur + x, cur + (x + 1), sizeof(E) * (l - (x + 1)));
		return true;
		// this is for if 2 blocks did merge
		done:
		E** p;
		for (level = 0, factor = k + d; level < levels - 1; level++, factor = (factor << 1) - d) { //loops up across levels, where 2^factor is the branching factor
			x = (path + level)->x; //the node to be removed's index
			cur = (path + level)->start; //the node to be removed's parent
			y = (path + (level + 1))->x; //the node to be removed's parent's index
			p = (E**)((path + (level + 1))->start) - 1;
			l = *(uint*)(cur + (1 << factor)); //the node to be removed's parent's real size
			if (l == 1) { //if the node to be removed's parent would be empty after removing the node, the parent should be deleted
				free(((E**)cur - (1 << (factor))));
				continue;
			}
			if (y != 0) { //checking if the node's parent should be merged with the node behind it
				other = *(p - y + 1); //the node's parent's node behind
				ol = *(uint*)(other + (1 << factor)); //other's real size
				if (l + ol - 1 <= (uint)1 << (factor - 1)) {
					*(uint*)(other + (1 << factor)) = l + ol - 1;
					memcpy((E**)other - (l + ol - 1), (E**)cur - l, sizeof(E*) * (l - (x + 1)));
					memcpy((E**)other - (ol + x), (E**)cur - x, sizeof(E*) * x);
					memcpy(other + ol, cur, sizeof(E) * x);
					memcpy(other + (ol + x), cur + (x + 1), sizeof(E) * (l - (x + 1)));
					free(((E**)cur - (1 << factor)));
					continue;
				}
			}
			if (y != (level != levels - 2 ? *(uint*)((E*)(p + 1) + (1 << ((factor << 1) - d))) : *(uint*)(start + (1 << curfactor))) - 1) { //checking if the node's parent should be merged with the node after it
				other = *(p - y - 1); //the node's parent's node ahead
				ol = *(uint*)(other + (1 << factor)); //other's real size
				if (l + ol - 1 <= (uint)1 << (factor - 1)) {
					*(uint*)(cur + (1 << factor)) = l + ol - 1;
					memmove((E**)cur - (l - 1), (E**)cur - l, sizeof(E*) * (l - (x + 1)));
					memcpy((E**)cur - (l + ol - 1), (E**)other - ol, sizeof(E*) * ol);
					memmove(cur + x, cur + (x + 1), sizeof(E) * (l - (x + 1)));
					memcpy(cur + (l - 1), other, sizeof(E) * ol);
					free(((E**)other - (1 << factor)));
					(path + level + 1)->x++; //marking it so that other will be deleted rather than cur
					continue;
				}
			}
			//this is when nodes don't merge
			--*(uint*)(cur + (1 << factor));
			memmove((E**)cur - (l - 1), (E**)cur - l, sizeof(E*) * (l - x - 1));
			memmove(cur + x, cur + x + 1, sizeof(E) * (l - x - 1));
			return true;
		}
		//this is when the node to be deleted's parent is the root
		x = (path + levels - 1)->x; //the node's index from the root
		l = *(uint*)(start + (1 << curfactor)); //the root's real size
		cur = start;
		if (curfactor > 1) { //this is seeing if the roots current array size is small enough that the root should should be removed
			if (l - 1 > (uint)1 << (curfactor - 2)) { //this is if the root's array doesn't need resizing
				--*(uint*)(cur + (1 << curfactor));
				memmove((E**)cur - (l - 1), (E**)cur - l, sizeof(E*) * (l - x - 1));
				memmove(cur + x, cur + x + 1, sizeof(E) * (l - x - 1));
				return true;
			}
			//this is for when the root's array needs resizing to half its previous size
			curfactor--;
			start = (E*)((char*)malloc(((sizeof(E*) + sizeof(E)) << curfactor) + 4) + (sizeof(E*) << curfactor)); //allocating the new root
			*(uint*)(start + (1 << curfactor)) = l - 1;
			memcpy((E**)start - (l - 1), (E**)cur - l, sizeof(E*) * (l - (x - 1)));
			memcpy((E**)start - x, (E**)cur - x, (sizeof(E) + sizeof(E*)) * x);
			memcpy(start + x, cur + (x + 1), sizeof(E) * (l - (x - 1)));
			free(((E**)cur - (2 << curfactor)));
			if (curfactor > 1) { //if the root's maximum size is less than 2, the root should be deleted, and this is checking that
				return true;
			}
			cur = start;
		}
		//this is for if the root is being deleted
		if (levels != 1) { //if there is only 1 level, special things need to happen
			if (x == 0) { //sets the root equal to the node that wasn't deleted
				start = *((E**)cur - 2);
			}
			else {
				start = *((E**)cur - 1);
			}
			curfactor = (k << (--(this->levels) - 1)) + d;
			free(((E**)cur - 2));
			free(path);
			this->path = (T<E>*)malloc(this->levels * sizeof(T<E>));
			return true;
		}
		if (x == 0) {
			*cur = *(cur + 1);
			*((E**)cur - 1) = *((E**)cur - 2);
		}
		*(uint*)(cur + 2) = 1;
		return true;
	}
	bool insert(E insert) {
		const uint levels = this->levels;
		uint level = levels;
		T<E>* path = this->path;
		uint factor = (k << ((levels - 1))) + d;
		E* cur = start;
		uint x = 1 << (curfactor - 1);
		uint l = *(uint*)(cur + (x << 1));
		for (;factor >= k + d; factor = (factor + d) >> 1, x = 1 << (factor - 1), l = *(uint*)(cur + (1 << factor))) {
			switch (x) {
			case 1 << 16:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 15) : x + (1 << 15));
			case 1 << 15:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 14) : x + (1 << 14));
			case 1 << 14:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 13) : x + (1 << 13));
			case 1 << 13:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 12) : x + (1 << 12));
			case 1 << 12:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 11) : x + (1 << 11));
			case 1 << 11:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 10) : x + (1 << 10));
			case 1 << 10:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 9) : x + (1 << 9));
			case 1 << 9:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 8) : x + (1 << 8));
			case 1 << 8:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 7) : x + (1 << 7));
			case 1 << 7:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 6) : x + (1 << 6));
			case 1 << 6:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 5) : x + (1 << 5));
			case 1 << 5:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 4) : x + (1 << 4));
			case 1 << 4:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 3) : x + (1 << 3));
			case 1 << 3:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 2) : x + (1 << 2));
			case 1 << 2:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 1) : x + (1 << 1));
			case 1 << 1:
				x = (x >= l || insert < *(cur + x) ? x - (1 << 0) : x + (1 << 0));
			}
			x = (x >= l || insert < *(cur + x) ? x - 1 : x);
			*(path + --level) = T<E>(cur, x);
			cur = *((E**)cur - (x + 1));
		}
		l = *(uint*)(cur + (1 << k));
		x = 1 << (k - 1);
		switch (k) {
		case 9:
			x = (x >= l || insert < *(cur + x) ? x - (1 << 7) : x + (1 << 7));
		case 8:
			x = (x >= l || insert < *(cur + x) ? x - (1 << 6) : x + (1 << 6));
		case 7:
			x = (x >= l || insert < *(cur + x) ? x - (1 << 5) : x + (1 << 5));
		case 6:
			x = (x >= l || insert < *(cur + x) ? x - (1 << 4) : x + (1 << 4));
		case 5:
			x = (x >= l || insert < *(cur + x) ? x - (1 << 3) : x + (1 << 3));
		case 4:
			x = (x >= l || insert < *(cur + x) ? x - (1 << 2) : x + (1 << 2));
		case 3:
			x = (x >= l || insert < *(cur + x) ? x - (1 << 1) : x + (1 << 1));
		case 2:
			x = (x >= l || insert < *(cur + x) ? x - (1 << 0) : x + (1 << 0));
		}
		x = (x >= l || insert < *(cur + x) ? x - 1 : x);
		if (x < l && (x != 0 || cur != firstleaf) && !(insert > *(cur + x))) { //if the value is already in the tree
			return false;
		}
		length++;
		x++; //x is now the index of where the inserted element should go
		if (l < 1 << k) { //if the block isn't full, inserts it into the block
			++*(uint*)(cur + (1 << k));
			memmove(cur + x + 1, cur + x, sizeof(E) * (l - x));
			*(cur + x) = insert;
			return true;
		}
		E* next = (E*)malloc((sizeof(E) << k) + 4); //makes the 2nd block the 2nd half of cur splits into
		if (x < 1 << (k - 1)) { // if the inserted element will be cur after the split
			*(uint*)(cur + (1 << k)) = (1 << (k - 1)) + 1;
			*(uint*)(next + (1 << k)) = 1 << (k - 1);
			memcpy(next, cur + (1 << (k - 1)), sizeof(E) << (k - 1));
			memmove(cur + (x + 1), cur + x, sizeof(E) * ((1 << (k - 1)) - x));
			*(cur + x) = insert;
		}
		else { //if the inserted element will be in next after the split
			*(uint*)(cur + (1 << k)) = 1 << (k - 1);
			*(uint*)(next + (1 << k)) = (1 << (k - 1)) + 1;
			x -= 1 << (k - 1);
			memcpy(next, cur + (1 << (k - 1)), sizeof(E) * x);
			*(next + x) = insert;
			memcpy(next + (x + 1), cur + (x + (1 << (k - 1))), sizeof(E) * ((1 << (k - 1)) - x));
		}
		level = 0;
		factor = k + d;
		for (;level < levels - 1; level++, factor = (factor << 1) - d) { //iterates up the tree to insert nodes
			cur = (path + level)->start; //the node that something will be inserted into
			x = (path + level)->x + 1; //the index into where the inserted value should be
			l = *(uint*)(cur + (1 << factor)); //the length of cur
			if (l < (uint)1 << factor) { //if cur isn't full, just insert the value
				++*(uint*)(cur + (1 << factor));
				memmove((E**)cur - (l + 1), (E**)cur - l, sizeof(E*) * (l - x));
				*((E**)cur - (x + 1)) = next;
				memmove(cur + x + 1, cur + x, sizeof(E) * (l - x));
				*(cur + x) = *next;
				return true;
			}
			// if cur is full
			E* old = next; //the thing that will be inserted
			next = (E*)((char*)malloc(((sizeof(E) + sizeof(E*)) << factor) + 4) + (sizeof(E*) << factor)); //makes the node into which the 2nd half of cur will split into
			if (x < l >> 1) { //if the value should be in cur
				*(uint*)(cur + l) = (l >> 1) + 1;
				*(uint*)(next + l) = l >> 1;
				l >>= 1;
				memcpy((E**)next - l, (E**)cur - (l << 1), sizeof(E*) << (factor - 1));
				memcpy(next, cur + l, sizeof(E) << (factor - 1));
				memmove((E**)cur - (l + 1), (E**)cur - l, sizeof(E*) * (l - x));
				*((E**)cur - (x + 1)) = old;
				memmove(cur + x + 1, cur + x, sizeof(E) * (l - x));
				*(cur + x) = *old;
			}
			else { // if the value should be in next
				*(uint*)(cur + l) = l >> 1;
				*(uint*)(next + l) = (l >> 1) + 1;
				x -= l >>= 1;
				memcpy((E**)next - (l + 1), (E**)cur - (l << 1), sizeof(E*) * (l - x));
				*((E**)next - (x + 1)) = old;
				memcpy((E**)next - x, (E**)cur - (l + x), sizeof(E*) * x);
				memcpy(next, cur + l, sizeof(E) * x);
				*(next + x) = *old;
				memcpy(next + (x + 1), cur + (l + x), sizeof(E) * (l - x));
			}
		}
		// if something's being inserted into the root
		cur = start;
		x = (path + level)->x + 1; // the index where the inserted thing should be
		l = *(uint*)(start + (1 << curfactor));
		if (l < (uint)1 << curfactor) { //if the root isn't full, it should just be inserted
			++*(uint*)(cur + (1 << curfactor));
			memmove((E**)cur - (l + 1), (E**)cur - l, sizeof(E*) * (l - x));
			*((E**)cur - (x + 1)) = next;
			memmove(cur + x + 1, cur + x, sizeof(E) * (l - x));
			*(cur + x) = *next;
			return true;
		}
		if (curfactor != (k << ((levels - 1))) + d) { //if the root is full, but it's current branching factor isn't the maximum branching factor it can have, it should double in size
			curfactor++;
			E* old = next;
			E* next = (E*)((char*)malloc(((sizeof(E) + sizeof(E*)) << curfactor) + 4) + (sizeof(E*) << curfactor)); //creates the new root with double the size
			memcpy((E**)next - (l + 1), (E**)cur - l, sizeof(E*) * (l - x));
			*((E**)next - (x + 1)) = old;
			memcpy((E**)next - x, (E**)cur - x, (sizeof(E*) + sizeof(E)) * x);
			*(next + x) = *old;
			memcpy(next + (x + 1), cur + x, sizeof(E) * (l - x));
			free(((E**)start - (1 << (curfactor - 1))));
			start = next;
			*(uint*)(start + (1 << curfactor)) = (1 << (curfactor - 1)) + 1;
			return true;
		}
		//if the root is full and at it's maximum allowed branching factor, the trees height needs to increase and a new root made
		E* old = next;
		next = (E*)((char*)malloc(((sizeof(E) + sizeof(E*)) << curfactor) + 4) + (sizeof(E*) << curfactor)); //makes the node into which the 2nd half of the old root will split into
		if (x < l >> 1) { //if the value should be in the old root
			*(uint*)(cur + l) = (l >> 1) + 1;
			*(uint*)(next + l) = l >> 1;
			l >>= 1;
			memcpy((E**)next - l, (E**)cur - (l << 1), sizeof(E*) << (curfactor - 1));
			memcpy(next, cur + l, sizeof(E) << (curfactor - 1));
			memmove((E**)cur - (l + 1), (E**)cur - l, sizeof(E*) * (l - x));
			*((E**)cur - (x + 1)) = old;
			memmove(cur + x + 1, cur + x, sizeof(E) * (l - x));
			*(cur + x) = *old;
		}
		else { //if the value should be in next
			*(uint*)(cur + l) = l >> 1;
			*(uint*)(next + l) = (l >> 1) + 1;
			x -= l >>= 1;
			memcpy((E**)next - (l + 1), (E**)cur - (l << 1), sizeof(E*) * (l - x));
			*((E**)next - (x + 1)) = old;
			memcpy((E**)next - x, (E**)cur - (l + x), sizeof(E*) * x);
			memcpy(next, cur + l, sizeof(E) * x);
			*(next + x) = *old;
			memcpy(next + (x + 1), cur + (l + x), sizeof(E) * (l - x));
		}
		cur = (E*)((char*)malloc((sizeof(E) + sizeof(E*) + 2) << 1) + (sizeof(E*) << 1)); //creates the new root and makes it point to the old root and next
		*(uint*)(cur + 2) = 2;
		*cur = *start;
		*((E**)cur - 1) = start;
		*(cur + 1) = *next;
		*((E**)cur - 2) = next;
		start = cur;
		curfactor = 1;
		this->levels++;
		free(path);
		this->path = (T<E>*)malloc(this->levels * sizeof(T<E>));
		return true;
	}
	E const contains(E search) {
		uint factor = (k << ((levels - 1))) + d; //2^factor is the current nodes branching factor
		E* cur = start; //the current node
		uint x = 1 << (curfactor - 1); //the with which a comparison will happen
		uint l = *(uint*)(cur + (x << 1)); //the size of cur
		for (;factor >= k + d; factor = (factor + d) >> 1, x = 1 << (factor - 1), l = *(uint*)(cur + (1 << factor))) { //iterates across all levels adjusting values
			switch (x) {
			case 1 << 16:
				x = (x >= l || search < *(cur + x) ? x - (1 << 15) : x + (1 << 15));
			case 1 << 15:
				x = (x >= l || search < *(cur + x) ? x - (1 << 14) : x + (1 << 14));
			case 1 << 14:
				x = (x >= l || search < *(cur + x) ? x - (1 << 13) : x + (1 << 13));
			case 1 << 13:
				x = (x >= l || search < *(cur + x) ? x - (1 << 12) : x + (1 << 12));
			case 1 << 12:
				x = (x >= l || search < *(cur + x) ? x - (1 << 11) : x + (1 << 11));
			case 1 << 11:
				x = (x >= l || search < *(cur + x) ? x - (1 << 10) : x + (1 << 10));
			case 1 << 10:
				x = (x >= l || search < *(cur + x) ? x - (1 << 9) : x + (1 << 9));
			case 1 << 9:
				x = (x >= l || search < *(cur + x) ? x - (1 << 8) : x + (1 << 8));
			case 1 << 8:
				x = (x >= l || search < *(cur + x) ? x - (1 << 7) : x + (1 << 7));
			case 1 << 7:
				x = (x >= l || search < *(cur + x) ? x - (1 << 6) : x + (1 << 6));
			case 1 << 6:
				x = (x >= l || search < *(cur + x) ? x - (1 << 5) : x + (1 << 5));
			case 1 << 5:
				x = (x >= l || search < *(cur + x) ? x - (1 << 4) : x + (1 << 4));
			case 1 << 4:
				x = (x >= l || search < *(cur + x) ? x - (1 << 3) : x + (1 << 3));
			case 1 << 3:
				x = (x >= l || search < *(cur + x) ? x - (1 << 2) : x + (1 << 2));
			case 1 << 2:
				x = (x >= l || search < *(cur + x) ? x - (1 << 1) : x + (1 << 1));
			case 1 << 1:
				x = (x >= l || search < *(cur + x) ? x - (1 << 0) : x + (1 << 0));
			}
			cur = (x >= l || search < *(cur + x) ? *((E**)cur - x) : *((E**)cur - (x + 1))); // after the loop, search >= x - 1 and < x + 1, so this chooses between x and x- 1
		}
		l = *(uint*)(cur + (1 << k));
		x = 1 << (k - 1);
		switch (k) {
		case 9:
			x = (x >= l || search < *(cur + x) ? x - (1 << 7) : x + (1 << 7));
		case 8:
			x = (x >= l || search < *(cur + x) ? x - (1 << 6) : x + (1 << 6));
		case 7:
			x = (x >= l || search < *(cur + x) ? x - (1 << 5) : x + (1 << 5));
		case 6:
			x = (x >= l || search < *(cur + x) ? x - (1 << 4) : x + (1 << 4));
		case 5:
			x = (x >= l || search < *(cur + x) ? x - (1 << 3) : x + (1 << 3));
		case 4:
			x = (x >= l || search < *(cur + x) ? x - (1 << 2) : x + (1 << 2));
		case 3:
			x = (x >= l || search < *(cur + x) ? x - (1 << 1) : x + (1 << 1));
		case 2:
			x = (x >= l || search < *(cur + x) ? x - (1 << 0) : x + (1 << 0));
		}
		x = (x >= l || search < *(cur + x) ? x - 1 : x); //now search >= x and < x + 1
		if (x >= l || (x == 0 && cur == firstleaf) || search > *(cur + x)) { // if the value isn't found
			return false;
		}
		return *(cur + x);
	}
	E* const sort() {
		if (length == 0) {
			return 0;
		}
		const uint levels = this->levels;
		uint level = levels;
		T<E>* path = this->path;
		E* cur = start;
		for (;level;) {
			(path + --level)->start = cur;
			(path + level)->x = 0;
			cur = *((E**)cur - 1);
		}
		E* ans = (E*)malloc(sizeof(E) * length);
		E* firstleaf = this->firstleaf;
		uint l = *(uint*)(firstleaf + (1 << k));
		memcpy(ans, firstleaf + 1, sizeof(E) * (l - 1));
		path->x = 1;
		uint x = l - 1;
		for (;;) {
			cur = *((E**)path->start - path->x - 1);
			l = *(uint*)(cur + (1 << k));
			memcpy(ans + x, cur, sizeof(E) * l);
			if ((x += l) == length) {
				return ans;
			}
			uint s = 0;
			for (;;s++) {
				if ((path + s)->x + 1 < *(uint*)((path + s)->start + (1 << (s == levels - 1 ? curfactor : d + (k << s))))) {
					break;
				}
			}
			(path + s)->x++;
			for (; s != 0;) {
				(path + (s - 1))->start = *((E**)((path + s)->start) - (path + s)->x - 1);
				(path + --s)->x = 0;
			}
		}
	}
};

#undef uint

#endif /* GTREE_H_ */
