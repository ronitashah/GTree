#ifndef GTREE_H_
#define GTREE_H_

#include <iostream>
#include <cstdlib>
using namespace std;

// nodes are stored as E*, which is an array of values such that *(p + i) is the min value of the ith subtree, *((E**)p - i - 1) is the ith subtree, and *(uint*)(p + (1 << factor)) is the amount of real values

constexpr unsigned char k = 6; // 2^k is the block size
constexpr unsigned char d = 1; // nodes have a branching factor of 2^(d + k*2^h), where h is the node height
constexpr unsigned char e = (3 << k) >> 2; // the minimum sum of real sizes for neighboring blocks

//stores the path, where start is the node and x is the index of the subtree went into

template<class E> struct T {
	E* start;
	uint x;
	T(E* s, uint i) {
		start = s;
		x = i;
	}
};

template<class E> class GTree {
public:
	E* start; // pointer to the root node
	uint length; // number of total elements in the tree
	unsigned char curfactor; // 2^curfactor is the current physical size of the root
	unsigned short goodfactor; // 2^goodfactor is the size the root should be
	unsigned char levels; // the number of levels
	E min; // the minimum value in the tree
	GTree() {
		start = (E*)(malloc((sizeof(E) + sizeof(E*) + 2) << 1) + (sizeof(E*) << 1));
		*(uint*)(start + 2) = 1;
		*((E**)start - 1) = (E*)malloc((sizeof(E) << k) + 4);
		*(uint*)(*((E**)start - 1) + (1 << k)) = 0;
		length = 0;
		curfactor = 1;
		goodfactor = k + d;
		levels = 1;
	}
	bool remove(E remove) {
		if (length == 0 || remove < min) {
			return false;
		}
		// same algorithm as contains, just also stores the path in an array
		T<E>* path = (T<E>*)malloc(sizeof(T<E>) * levels);
		signed char level = levels;
		unsigned char factor = goodfactor;
		uint l = *(uint*)(start + (1 << curfactor));
		E* cur = start;
		unsigned char z = curfactor - 1;
		uint x = 1 << z;
		for (;factor >= k + d; factor = (factor + d) >> 1, z = factor - 1, x = 1 << z, l = *(uint*)(cur + (1 << factor))) {
			for (;z;) {
				x >= l || remove < *(cur + x) ? x -= 1 << --z : x += 1 << --z;
			}
			x >= l || remove < *(cur + x) ? x-- : x;
			*(path + --level) = T<E>(cur, x);
			cur = *((E**)cur - (x + 1));
		}
		l = *(uint*)(cur + (1 << k));
		z = k - 1;
		x = 1 << z;
		for (;z;) {
			x >= l || remove < *(cur + x) ? x -= 1 << --z : x += 1 << --z;
		}
		x >= l || remove < *(cur + x) ? x-- : x;
		if (x >= l || remove > *(cur + x)) { // if the value isn't in the tree
			delete path;
			return false;
		}
		length--;
		uint ol = l; // from here
		E* to;
		uint y = x;
		E* other;
		factor = k + d;
		for (level = -1; y == 0 && l == 1 && level < levels; y = (path + ++level)->x, l = *(uint*)((path + level)->start + (1 << factor)),  factor = (factor << 1) - d);
		if (y == 0 && level < levels && l > 1) {
			E p;
			if (level != -1) {
				p = *((path + level)->start + 1);
			}
			else {
				p = *(cur + 1);
			}
			for (;++level < levels && y == 0; y = (path + level)->x) {
				*((path + level)->start + (path + level)->x) = p;
			}
			if (y == 0 && level == levels) {
				min = p;
			}
		} // to here just resets min values that need resetting because the old one got deleted
		l = *(uint*)(cur + (1 << k)); // the real length of the blokc containing the element
		y = path->x; // index of the block containing the element in the node with height 0
		if (l == 1) { // if the block will be empty after the deletion, it should just be removed
			delete cur;
			goto done;
		}
		if (y != 0) { //checking if the block should be merged with the block before it
			other = *((E**)path->start - y); //the block before
			ol = *(uint*)(other + (1 << k)); //  the block before's real size
			if (l + ol <= e + 1) {
				*(uint*)(other + (1 << k)) = l + ol - 1;
				other += ol;
				y = ~(uint)0;
				for (;++y != x;) { //copying cur into other
					*(other + y) = *(cur + y);
				}
				other--; //skips copying the element that should be deleted
				for (;++y != l;) {
					*(other + y) = *(cur + y);
				}
				delete cur;
				goto done;
			}
		}
		if (y != (levels != 1 ? *(uint*)(path->start + (1 << (k + d))) : *(uint*)(start + (1 << curfactor))) - 1) { //checking if the block should be merged with the block after it
			other = *((E**)path->start - (y + 2)); //the block after it
			ol = *(uint*)(other + (1 << k)); //the block after's real size
			if (l + ol <= e + 1) {
				*(uint*)(cur + (1 << k)) = l + ol - 1;
				to = cur + l;
				cur += x;
				for (;++cur != to;) { //deletes the element from cur
					*(cur - 1) = *cur;
				}
				cur--;
				y = ~(uint)0;
				for (; ++y != ol;) { // copies other into cur
		 			*(cur + y) = *(other + y);
				}
				delete other;
				path->x++; // makes it so that other will be deleted in the 1st level node's array instead of cur
				goto done;
			}
		}
		//this is for if the blocks don't merge
		--*(uint*)(cur + (1 << k));
		to = cur + l;
		cur += x;
		for (;++cur != to;) { // removes the element that should be removed
			*(cur - 1) = *cur;
		}
		delete path;
		return true;
		// this is for if 2 blocks did merge
		done:
		E** t;
		E** c;
		for (level = 0, factor = k + d; level < levels - 1; level++, factor = (factor << 1) - d) { //loops up across levels, where 2^factor is the branching factor
			x = (path + level)->x; //the node to be removed's index
			cur = (path + level)->start; //the node to be removed's parent
			c = (E**)cur - 1;
			y = (path + (level + 1))->x; //the node to be removed's parent's index
			to = (path + (level + 1))->start; //the node to be removed's parent's parent
			t = (E**)to - 1;
			l = *(uint*)(cur + (1 << factor)); //the node to be removed's parent's real size
			if (l == 1) { //if the node to be removed's parent would be empty after removing the node, the parent should be deleted
				delete (c - (1 << (factor)) + 1);
				continue;
			}
			if (y != 0) { //checking if the node's parent should be merged with the node behind it
				other = *(t - y + 1); //the node's parent's node behind
				ol = *(uint*)(other + (1 << factor)); //other's real size
				if (l + ol - 1 <= (uint)1 << (factor - 1)) {
					*(uint*)(other + (1 << factor)) = l + ol - 1;
					t = (E**)other - 1 - ol;
					other += ol;
					y = ~(uint)0;
					for (;++y != x;) { //copying cur into other
						*(other + y) = *(cur + y);
						*(t - y) = *(c - y);
					}
					other--; //skipping copying the node to be removed
					t++;
					for (;++y != l;) {
						*(other + y) = *(cur + y);
						*(t - y) = *(c - y);
					}
					delete (c - (1 << factor) + 1);
					continue;
				}
			}
			if (y != (level != levels - 2 ? *(uint*)(to + (1 << ((factor << 1) - d))) : *(uint*)(start + (1 << curfactor))) - 1) { //checking if the node's parent should be merged with the node after it
				other = *(t - y - 1); //the node's parent's node ahead
				ol = *(uint*)(other + (1 << factor)); //other's real size
				if (l + ol - 1 <= (uint)1 << (factor - 1)) {
					*(uint*)(cur + (1 << factor)) = l + ol - 1;
					to = cur + l;
					cur += x;
					for (;++cur != to;) { //deleting the node to be removed from cur
						*(cur - 1) = *cur;
					}
					t = c - l;
					c -= x;
					for (;--c != t;) { //also deleting the node
						*(c + 1) = *c;
					}
					cur--;
					c++;
					t = (E**)other - 1;
					y = ~(uint)0;
					for (;++y != ol;) { //copying other into cur
						*(cur + y) = *(other + y);
						*(c - y) = *(t - y);
					}
					delete (t - (1 << factor) + 1);
					(path + level + 1)->x++; //marking it so that other will be deleted rather than cur
					continue;
				}
			}
			//this is when nodes don't merge
			--*(uint*)(cur + (1 << factor));
			to = cur + l;
			cur += x;
			for (;++cur != to;) { //deleting the node to be deleted from cur
				*(cur - 1) = *cur;
			}
			t = c - l;
			c -= x;
			for (;--c != t;) { //also deleting the node
				*(c + 1) = *c;
			}
			delete path;
			return true;
		}
		//this is when the node to be deleted's parent is the root
		x = (path + levels - 1)->x; //the node's index from the root
		l = *(uint*)(start + (1 << curfactor)); //the root's real size
		cur = start;
		c = (E**)start - 1;
		if (curfactor > 1) { //this is seeing if the roots current array size is small enough that the root should should be removed
			if (l - 1 > (uint)1 << (curfactor - 2)) { //this is if the root's array doesn't need resizing
				--*(uint*)(cur + (1 << curfactor));
				to = cur + l;
				cur += x;
				for (;++cur != to;) { //removing the node to be removed from the root
					*(cur - 1) = *cur;
				}
				t = c - l;
				c -= x;
				for (;--c != t;) { //also removing the node
					*(c + 1) = *c;
				}
				delete path;
				return true;
			}
			//this is for when the root's array needs resizing to half its previous size
			curfactor--;
			start = (E*)(malloc(((sizeof(E*) + sizeof(E)) << curfactor) + 4) + (sizeof(E*) << curfactor)); //allocating the new root
			*(uint*)(start + (1 << curfactor)) = l - 1;
			t = (E**)start - 1;
			y = ~(uint)0;
			for (; ++y != x;) { //copying the old root into the new root
				*(start + y) = *(cur + y);
				*(t - y) = *(c - y);
			}
			start--; //skipping copying the node
			t++;
			for (; ++y != l;) {
				*(start + y) = *(cur + y);
				*(t - y) = *(c - y);
			}
			start++;
			delete (c - (2 << curfactor) + 1);
			if (curfactor > 1) { //if the root's maximum size is less than 2, the root should be deleted, and this is checking that
				delete path;
				return true;
			}
			cur = start;
			c = (E**)start - 1;
		}
		//this is for if the root is being deleted
		if (levels != 1) { //if there is only 1 level, special things need to happen
			if (x == 0) { //sets the root equal to the node that wasn't deleted
				start = *(c - 1);
			}
			else {
				start = *c;
			}
			goodfactor = (goodfactor + d) >> 1; //fixes the values of the fields
			curfactor = goodfactor;
			levels--;
			delete (c - 1);
			delete path;
			return true;
		}
		if (l > 1) { //if there will still be elements in the root after the deletion
			if (x == 0) {
				*cur = *(cur + 1);
				*c = *(c - 1);
			}
			*(uint*)(cur + 2) = 1;
			delete path;
			return true;
		}
		//if the tree will be empty, which should reset the tree to how it was after construction
		*((E**)start - 1) = (E*)malloc((sizeof(E) << k) + 4);
		*(uint*)(*((E**)start - 1) + (1 << k)) = 0;
		delete path;
		return true;
	}
	bool insert(E insert) {
		T<E>* path = (T<E>*)malloc(sizeof(T<E>) * levels);
		unsigned char level = levels;
		unsigned char factor = goodfactor;
		uint l = *(uint*)(start + (1 << curfactor));
		E* cur = start;
		unsigned char z = curfactor - 1;
		uint x = 1 << z;
		if (length == 0 || insert < min) { //if and only of the inserted value is smaller than the min, min values in arrays need updating
			for (;level;) {
				*cur = insert;
				*(path + --level) = T<E>(cur, 0);
				cur = *((E**)cur - 1);
			}
			*cur = insert;
			insert = min;
			min = *cur;
			if (length == 0) {
				delete path;
				length++;
				++*(uint*)(cur + (1 << k));
				return true;
			}
		}
		else { //same algorithm as contains
			for (;factor >= k + d; factor = (factor + d) >> 1, z = factor - 1, x = 1 << z, l = *(uint*)(cur + (1 << factor))) {
				for (;z;) {
					x >= l || insert < *(cur + x) ? x -= 1 << --z : x += 1 << --z;
				}
				x >= l || insert < *(cur + x) ? x-- : x;
				*(path + --level) = T<E>(cur, x);
				cur = *((E**)cur - (x + 1));
			}
		}
		l = *(uint*)(cur + (1 << k));
		z = k - 1;
		x = 1 << z;
		for (;z;) {
			x >= l || insert < *(cur + x) ? x -= 1 << --z : x += 1 << --z;
		}
		x >= l || insert < *(cur + x) ? x-- : x;
		if (x < l && !(insert > *(cur + x))) { //if the value is already in the tree
			delete path;
			return false;
		}
		x++; //x is now the index of where the inserted element should go
		if (l < 1 << k) { //if the block isn't full, inserts it into the block
			++*(uint*)(cur + (1 << k));
			E* to = cur + x;
			cur += l + 1;
			for (; --cur != to;) {
				*cur = *(cur - 1);
			}
			*to = insert;
			delete path;
			length++;
			return true;
		}
		E* to;
		uint y;
		E* next = (E*)malloc((sizeof(E) << k) + 4); //makes the 2nd block the 2nd half of cur splits into
		if (x < 1 << (k - 1)) { // if the inserted element will be cur after the split
			*(uint*)(cur + (1 << k)) = (1 << (k - 1)) + 1;
			*(uint*)(next + (1 << k)) = 1 << (k - 1);
			y = 1 << (k - 1);
			for (;;) { //copies the 2nd half of cur into next
				*(next + (y - (1 << (k - 1)))) = *(cur + y);
				if (++y == 1 << k) {
					break;
				}
			}
			to = cur + x;
			cur += (1 << (k - 1)) + 1;
			for (; --cur != to;) { //inserts the element into cur
				*cur = *(cur - 1);
			}
			*to = insert;
		}
		else { //if the inserted element will be in next after the split
			*(uint*)(cur + (1 << k)) = 1 << (k - 1);
			*(uint*)(next + (1 << k)) = (1 << (k - 1)) + 1;
			y = (1 << (k - 1)) - 1;
			for (; ++y != x;) { //copies the 2nd half of cur into next
				*(next + (y - (1 << (k - 1)))) = *(cur + y);
			}
			*(next + (y-- - (1 << (k - 1)))) = insert; //inserts the element into next where it should belong
			for (;++y != 1 << k;) {
				*(next + (y - (1 << (k - 1)) + 1)) = *(cur + y);
			}
		}
		level = 0;
		factor = k + d;
		for (;level < levels - 1; level++, factor = (factor << 1) - d) { //iterates up the tree to insert nodes
			cur = (path + level)->start; //the node that something will be inserted into
			x = (path + level)->x + 1; //the index into where the inserted value should be
			l = *(uint*)(cur + (1 << factor)); //the length of cur
			if (l < (uint)1 << factor) { //if cur isn't full, just insert the value
				++*(uint*)(cur + (1 << factor));
				to = cur + x;
				cur += l + 1;
				for (;--cur != to;) { //inserts the value into cur
					*cur = *(cur - 1);
				}
				*to = *next;
				E** c = (E**)(cur - x) - 1;
				E** t = c - x;
				c -= l + 1;
				for (;++c != t;) { //also inserts
					*c = *(c + 1);
				}
				*t = next;
				delete path;
				length++;
				return true;
			}
			// if cur is full
			E* old = next; //the thing that will be inserted
			next = (E*)(malloc(((sizeof(E) + sizeof(E*)) << factor) + 4) + (sizeof(E*) << factor)); //makes the node into which the 2nd half of cur will split into
			E** c = (E**)cur - 1;
			E** t = (E**)next - 1;
			if (x < (uint)1 << (factor - 1)) { //if the value should be in cur
				*(uint*)(cur + (1 << factor)) = (1 << (factor - 1)) + 1;
				*(uint*)(next + (1 << factor)) = 1 << (factor - 1);
				y = 1 << (factor - 1);
				for (;;) { //copies the 2nd half of cur into next
					*(next + y - (1 << (factor - 1))) = *(cur + y);
					*(t - (y - (1 << (factor - 1)))) = *(c - y);
					if (++y == (uint)1 << factor) {
						break;
					}
				}
				to = cur + x;
				cur += (1 << (factor - 1)) + 1;
				for (;--cur != to;) { //inserts the element into cur
					*cur = *(cur - 1);
				}
				*to = *old;
				c = (E**)(cur - x) - 1;
				t = c - x;
				c -= (1 << (factor - 1)) + 1;
				for (; ++c != t;) { //also inserts
					*c = *(c + 1);
				}
				*t = old;
			}
			else { // if the value should be in next
				*(uint*)(cur + (1 << factor)) = 1 << (factor - 1);
				*(uint*)(next + (1 << factor)) = (1 << (factor - 1)) + 1;
				y = (1 << (factor - 1)) - 1;
				for (;++y != x;) { //copies the 2nd half of cur into next
					*(next + (y - (1 << (factor - 1)))) = *(cur + y);
					*(t - (y - (1 << (factor - 1)))) = *(c - y);
				}
				*(next + (y - (1 << (factor - 1)))) = *old; // inserts the value
				*(t - (y-- - (1 << (factor - 1)))) = old;
				for (;++y != (uint)1 << factor;) {
					*(next + (y - (1 << (factor - 1)) + 1)) = *(cur + y);
					*(t - (y - (1 << (factor - 1)) + 1)) = *(c - y);
				}
			}
		}
		// if something's being inserted into the root
		cur = start;
		x = (path + level)->x + 1; // the index where the inserted thing should be
		l = *(uint*)(start + (1 << curfactor));
		if (l < (uint)1 << curfactor) { //if the root isn't full, it should just be inserted
			++*(uint*)(cur + (1 << curfactor));
			to = cur + x;
			cur += l + 1;
			for (;--cur != to;) { //inserts the value into the root
				*cur = *(cur - 1);
			}
			*to = *next;
			E** c = (E**)(cur - x) - 1;
			E** t = c - x;
			c -= l + 1;
			for (;++c != t;) {
				*c = *(c + 1);
			}
			*t = next;
			delete path;
			length++;
			return true;
		}
		if (curfactor != goodfactor) { //if the root is full, but it's current branching factor isn't the maximum branching factor it can have, it should double in size
			curfactor++;
			E* old = next;
			E* next = (E*)(malloc(((sizeof(E) + sizeof(E*)) << curfactor) + 4) + (sizeof(E*) << curfactor)); //creates the new root with double the size
			E** t = (E**)next - 1;
			E** c = (E**)start - 1;
			y = ~(uint)0;
			for (; ++y != x;) { //copies the old root into the new root
				*(next + y) = *(start + y);
				*(t - y) = *(c - y);
			}
			*(next + y) = *old; //puts the value into the new root
			*(t - y--) = old;
			for (;++y != (uint)1 << (curfactor - 1);) {
				*(next + (y + 1)) = *(start + y);
				*(t - (y + 1)) = *(c - y);
			}
			delete path;
			delete ((E**)start - (1 << (curfactor - 1)));
			start = next;
			*(uint*)(start + (1 << curfactor)) = (1 << (curfactor - 1)) + 1;
			length++;
			return true;
		}
		//if the root is full and at it's maximum allowed branching factor, the trees height needs to increase and a new root made
		E* old = next;
		next = (E*)(malloc(((sizeof(E) + sizeof(E*)) << curfactor) + 4) + (sizeof(E*) << curfactor)); //makes the node into which the 2nd half of the old root will split into
		E** c = (E**)cur - 1;
		E** t = (E**)next - 1;
		if (x < (uint)1 << (curfactor - 1)) { //if the value should be in the old root
			*(uint*)(start + (1 << curfactor)) = (1 << (curfactor - 1)) + 1;
			*(uint*)(next + (1 << curfactor)) = 1 << (curfactor - 1);
			y = 1 << (curfactor - 1);
			for (;;) { //copies the 2nd half of the old root into next
				*(next + (y - (1 << (curfactor - 1)))) = *(cur + y);
				*(t - (y - (1 << (curfactor - 1)))) = *(c - y);
				if (++y == (uint)1 << curfactor) {
					break;
				}
			}
			to = cur + x;
			cur += (1 << (curfactor - 1)) + 1;
			for (; --cur != to;) { //inserts into the old root
				*cur = *(cur - 1);
			}
			*to = *old;
			c = (E**)(cur - x) - 1;
			t = c - x;
			c -= (1 << (curfactor - 1)) + 1;
			for (; ++c != t;) { //also inserts
				*c = *(c + 1);
			}
			*t = old;
		}
		else { //if the value should be in next
			*(uint*)(start + (1 << curfactor)) = 1 << (curfactor - 1);
			*(uint*)(next + (1 << curfactor)) = (1 << (curfactor - 1)) + 1;
			y = (1 << (curfactor - 1)) - 1;
			for (;++y != x;) { //copies the 2nd half of the old root into next
				*(next + (y - (1 << (curfactor - 1)))) = *(cur + y);
				*(t - (y - (1 << (curfactor - 1)))) = *(c - y);
			}
			*(next + (y - (1 << (curfactor - 1)))) = *old; //inserts the value into next
			*(t - (y-- - (1 << (curfactor - 1)))) = old;
			for (;++y != (uint)1 << curfactor;) {
				*(next + (y - (1 << (curfactor - 1)) + 1)) = *(cur + y);
				*(t - (y - (1 << (curfactor - 1)) + 1)) = *(c - y);
			}
		}
		cur = (E*)(malloc((sizeof(E) + sizeof(E*) + 2) << 1) + (sizeof(E*) << 1)); //creates the new root and makes it point to the old root and next
		*(uint*)(cur + 2) = 2;
		*cur = *start;
		*((E**)cur - 1) = start;
		*(cur + 1) = *next;
		*((E**)cur - 2) = next;
		start = cur;
		curfactor = 1;
		goodfactor = (goodfactor << 1) - d;
		levels++;
		length++;
		delete path;
		return true;
	}
	bool contains(E search) {
		if (length == 0 || search < min) { //the binary search won't work if search < min, but is much faster than a traditional binary search, so this
			return 0;
		}
		unsigned char factor = goodfactor; //2^factor is the current nodes branching factor
		E* cur = start; //the current node
		unsigned char z = curfactor - 1; // 2^(z-1) is how much x will increment/decrement by after a comparison
		uint x = 1 << z; //the with which a comparison will happen
		uint l = *(uint*)(start + (1 << curfactor)); //the size of cur
		for (; factor >= k + d; factor = (factor + d) >> 1, z = factor - 1, x = 1 << z, l = *(uint*)(cur + (1 << factor))) { //iterates across all levels adjusting values
			for (;z;) { // as long as the amount x will change by after a comparison is greater than 0
				x >= l || search < *(cur + x) ? x -= 1 << --z : x += 1 << --z; //if x is not a real value or search < x, x should always decrement, else increment
			}
			x >= l || search < *(cur + x) ? cur = *((E**)cur - x) : cur = *((E**)cur - (x + 1)); // after the loop, search >= x - 1 and < x + 1, so this chooses between x and x- 1
		}
		l = *(uint*)(cur + (1 << k));
		z = k - 1;
		x = 1 << z;
		for (;z;) { //does the same search procedure on a block
			x >= l || search < *(cur + x) ? x -= 1 << --z : x += 1 << --z;
		}
		x >= l || search < *(cur + x) ? x-- : x; //now search >= x and < x + 1
		if (x >= l || search > *(cur + x)) { // if the value isn't found
			return false;
		}
		return true;
	}
	E* sort() {
		if (length == 0) {
			return 0;
		}
		T<E>* path = (T<E>*)malloc(sizeof(T<E>) * levels);
		unsigned char level = levels;
		E* cur = start;
		for (;level;) {
			(path + --level)->start = cur;
			(path + level)->x = 0;
			cur = *((E**)cur - 1);
		}
		unsigned char* size = (unsigned char*)malloc(levels);
		*size = k + d;
		unsigned char s = 1;
		for (; s < levels - 1; s++) {
			*(size + s) = (*(size + s - 1) << 1) - d;
		}
		*(size + levels - 1) = curfactor;
		E* ans = (E*)malloc(sizeof(E*) * length);
		uint x = 0;
		uint y;
		for (;;) {
			cur = *((E**)path->start - path->x - 1);
			y = 0;
			uint l = *(uint*)(cur + (1 << k));
			for (; y < l;) {
				*(ans + x++) = *(cur + y++);
			}
			if (x == length) {
				return ans;
			}
			for (s = 0;; s++) {
				if ((path + s)->x + 1 < *(uint*)((path + s)->start + (1 << *(size + s)))) {
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
#endif /* GTREE_H_ */
