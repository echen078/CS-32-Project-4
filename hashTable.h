#ifndef hashTable_h
#define hashTable_h

#include <iostream>
#include <vector>
#include <string>
using namespace std;

struct Node {
    string seq;
    int offset;
    Node* next;
    
    Node(const string& s, int o): seq(s), offset(o), next(nullptr) {};
};


class HashTable {
private:
    vector<Node*> table;
    int numBuckets;
    int size;

    // Hash function to map values to key
    int hashFunction(const string& key) {
        hash<string> str_hash;
        unsigned int hashValue = str_hash(key);
        unsigned int index = hashValue % numBuckets;
        return index;
    }

public:
    // Constructor
    HashTable(int cap) : numBuckets(cap), size(0) {
        table.resize(numBuckets, nullptr);
    }

    // Insert a key into the hash table
    void insert(const string& seq, int offset) {
        int index = hashFunction(seq);
        Node* newNode = new Node(seq, offset);
        if(!table[index])
        {
            table[index] = newNode;
        }
        else // collision
        {
            // insert to the end
            Node* temp = table[index];
            while (temp->next)
            {
                temp = temp->next;
            }
            temp->next = newNode;
        }
        size++;
    }

    // Search for a key in the hash table, returns a vector of all offset where sequence appears
    vector<int> search(const string& seq) {
        vector<int> offsets;
        int index = hashFunction(seq); //hashing given the sequence
        Node* temp = table[index];
        while (temp)
        {
            if (temp->seq == seq)
            {
                offsets.push_back(temp->offset);
            }
            temp = temp->next;
        }
        return offsets; // not found
    }
    
    void display() const {
        for (int i = 0; i < numBuckets; ++i) {
            std::cout << "Index " << i << ": ";
            Node* temp = table[i];
            while (temp) {
                std::cout << "['" << temp->seq << "': " << temp->offset << "] -> ";
                temp = temp->next;
            }
            std::cout << "nullptr" << std::endl;
        }
    }
    
    
    // Destructor
    ~HashTable() {
        for (int i = 0; i < numBuckets; i++)
        {
            Node* temp = table[i];
            while (temp) {
                Node* toDelete = temp;
                temp = temp->next;
                delete toDelete;
            }
        }
    }
    
};


#endif /* hashTable_h */
