//node length does not get updated when inserting on a copy of a node
#include <iostream>
#include <vector>
using namespace std;
constexpr unsigned int k = 4;

template<class E> struct Node {
    E* keys;
    Node<E>* nodes;
    unsigned int length;
    Node() {
        keys = nullptr;
        nodes = nullptr;
        length = 0;
    }
    Node(unsigned int size) {
        keys = (E*)malloc(size * sizeof(E));
        nodes = (Node<E>*)malloc(size * sizeof(Node<E>));
        length = 0;
    }
    void recursefree() {
        free(keys);
        if (nodes) {
            for (int x = 0; x < length; x++) {
                (nodes + x)->recursefree();
            }
            free(nodes);
        }
    }
    unsigned int const search(E key) {
        unsigned int min = 0;
        unsigned int max = length;
        while (min + 1 < max) {
            if (*(keys + (min + max) / 2) > key) {
                max = (min + max) / 2;
            }
            else {
                min = (min + max) / 2;
            }
        }
        return min;
    }
    void insert(E key, unsigned int index) {
        length++;
        for (unsigned int x = length - 1; x > index; x--) {
            *(keys + x) = *(keys + x - 1);
        }
        *(keys + index) = key;
    }
    void insert(Node<E> node, unsigned int index) {
        insert(*node.keys, index);
        for (unsigned int x = length - 1; x > index; x--) {
            *(nodes + x) = *(nodes + x - 1);
        }
        *(nodes + index) = node;
    }
    Node<E> split() {
        Node<E> ans = Node<E>(length);
        for (int x = 0; x < length / 2; x++) {
            *(ans.keys + x) = *(keys + x + (length + 1) / 2);
        }
        if (nodes) {
            for (int x = 0; x < length / 2; x++) {
                *(ans.nodes + x) = *(nodes + x + (length + 1) / 2);
            }
        }
        ans.length = length / 2;
        length = (length + 1) / 2;
        return ans;
    }
};

template<class E> struct Branch {
    Node<E> node;
    unsigned int index;
    Branch() {}
    Branch(Node<E> n, unsigned int i) {
        node = n;
        index = i;
    }
};

template<class E> struct GTree {
    Node<E> root;
    unsigned int height;
    unsigned int length;
    GTree(E min) {
        root = Node<E>(k);
        *root.keys = min;
        root.length = 1;
        height = 0;
        length = 1;
    }
    ~GTree() {
        root.recursefree();
    }
    void print() {
        vector<Node<E>> cur;
        cur.push_back(root);
        for (unsigned int h = 0; h < height; h++) {
            vector<Node<E>> next;
            for (Node<E> n : cur) {
                for (int x = 0; x < n.length; x++) {
                    cout << " " << *(n.keys + x);
                    next.push_back(*(n.nodes + x));
                }
                cout << ",";
            }
            cout << endl;
            cur = next;
        }
        for (Node<E> n : cur) {
            for (int x = 0; x < n.length; x++) {
                cout << " " << *(n.keys + x);
            }
            cout << ",";
        }
        cout << endl;
    }
    Branch<E>* const search(E key) {
        Branch<E>* path = (Branch<E>*)malloc((height + 1) * sizeof(Branch<E>));
        Node<E> cur = root;
        for (unsigned int h = 0;; h++) {
            unsigned int i = cur.search(key);
            *(path + h) = Branch<E>(cur, i);
            if (h == height) {
                return path;
            }
            cur = *(cur.nodes + i);
        }
    }
    bool const contains(E key) {
        Branch<E>* path = this->search(key);
        Branch<E> branch = *(path + height);
        free(path);
        return !(*(branch.node.keys + branch.index) < key);
    }
    void insert(E key) {
        length++;
        Branch<E>* path = this->search(key);
        Branch<E> branch = *(path + height);
        if (branch.node.length < k) {
            branch.node.insert(key, branch.index + 1);
            free(path);
            return;
        }
        Node<E> split = branch.node.split();
        if (branch.index < branch.node.length) {
            branch.node.insert(key, branch.index + 1);
        }
        else {
            split.insert(key, branch.index + 1 - branch.node.length);
        }
        unsigned int s = k;
        for (int h = height - 1; h >= 0; h--, s = s * s) {
            branch = *(path + h);
            if (branch.node.length < s) {
                branch.node.insert(split, branch.index + 1);
                free(path);
                return;
            }
            Node<E> temp = split;
            split = branch.node.split();
            if (branch.index < branch.node.length) {
                branch.node.insert(temp, branch.index + 1);
            }
            else {
                split.insert(temp, branch.index + 1 - branch.node.length);
            }
        }
        root = Node<E>(s * s);
        root.length = 2;
        *root.keys = *branch.node.keys;
        *root.nodes = branch.node;
        *(root.keys + 1) = *split.keys;
        *(root.nodes + 1) = split;
        free(path);
    }
    E* const sort() {
        E* sort = (E*)malloc(length * sizeof(E));
        Branch<E>* path = this->search(*root.keys);
        Branch<E> branch = *(path + height);
        for (int x = 0; x < length; x++) {
            if (branch.index == branch.node.length) {
                unsigned int h = height - 1;
                for (;(path + h)->index >= (path + h)->node.length - 1; h--);
                (path + h + 1)->node = *((path + h)->node.nodes + ++((path + h)->index));
                (path + ++h)->index = 0;
                for (;h < height; h++) {
                    (path + h + 1)->node = *(path + h)->node.nodes;
                    (path + h + 1)->index = 0;
                }
                Branch<E> branch = *(path + height);
            }
            cout << x << endl;
            *(sort + x) = *(branch.node.keys + branch.index++);
        }
        return sort;
    }
};