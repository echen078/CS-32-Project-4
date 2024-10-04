#include <iostream>
#include <fstream>
#include <string>
#include <functional>
#include <stack>
#include <filesystem>
using namespace std;

#include "hashTable.h"


////////////////////////////////////////////////////////
// createRevision function
/////////////////////////////////////////////////////////

char determineDeliminator(string addInstruction) //determines deliminator and adds to file
{
    char delimiter = '!';
    int s = 0;
    while (s < addInstruction.size())
    {
        if(addInstruction[s] == delimiter)
        {
            s = 0; // re-loop thru addInstruction to see if delimiter exists in it
            delimiter++;
        }
        else s++;
    }
    
    return delimiter;
}


void createRevision(istream& fold, istream& fnew, ostream& frevision)
{
    // 1: convert file to string
    string oldS, newS;
    char c;
    
    // convert old
    while (fold.get(c))
    {
        oldS += c;
    }
    
    // convert new
    while (fnew.get(c))
    {
        newS += c;
    }
    
    int N = 8;  // size of sequence
    string addInstruction = ""; // keeps track of the add instructions
    
    
    // 2: insert old file sequences into hash table
    HashTable sequenceTable(oldS.size()*1.5);
    for (int i = 0; i <= oldS.size()-N; i++)
    {
        string subsequence = oldS.substr(i,N);
        sequenceTable.insert(subsequence, i);
    }
    
    // 3: Processing new file's string
    int j = 0;
    while (j < newS.size())
    {
        string subsequence = newS.substr(j,N);
        vector<int> offsetVector = sequenceTable.search(subsequence); // gives off of the sequence pair from table (ONLY GIVES THE FIRST NODE)  --> fix: search have to loop thru all nodes
        
        
        
    // 3a + b: search for sequence, if find - determine length + write copy instruction
        if (!offsetVector.empty())
        {
            // input addInstruction to revision file
            if (addInstruction != "")
            {
                frevision << "+" << determineDeliminator(addInstruction) << addInstruction << determineDeliminator(addInstruction);
            }
            
            addInstruction = ""; // reset addInstruction
            
            
            // copy instruction
            int maxLen = 0;
            int offsetOfMax = offsetVector[0]; // offset of where it first appears
            
            // loop thru each offset where the sequence appears
            for(int i = 0; i < offsetVector.size(); i++)
            {
                // get the length (of the current of set)
                int curLen = N;
                int curOffset = offsetVector[i];
                while (j + curLen < newS.size() && curOffset + curLen < oldS.size() && newS[j+curLen] == oldS[curOffset+curLen])
                {
                    curLen++;
                }
                if (curLen > maxLen)
                {
                    offsetOfMax = offsetVector[i];
                    maxLen = curLen;
                }
            }
            
            frevision << "#" << offsetOfMax << "," << maxLen;
            j += maxLen;
        }
        
    // 3c: not found - write add instruction
        else
        {
            addInstruction += newS[j];
            j++;
        }
    }
    if (addInstruction != "") // add the rest of the instruction to file
    {
        frevision << "+" << determineDeliminator(addInstruction) << addInstruction << determineDeliminator(addInstruction);
        addInstruction = "";
    }
    
}


//////////////////////////////////////
// revise function
//////////////////////////////////////


bool revise(istream& fold, istream& frevision, ostream& fnew)
{
    // convert revision file to string
    string oldS, revS;
    char c;
    
    // convert old
    while (fold.get(c))
    {
        oldS += c;
    }
    
    // convert new
    while (frevision.get(c))
    {
        revS += c;
    }
    
    stack<char> delimStack;
    
    int i = 0;
    while (i < revS.size())
    {
        char ch = revS[i];
        switch (ch) {
            case '+':
                if (i+2 < revS.size())
                {
                    // push delimiter
                    delimStack.push(revS[i+1]);
                    i += 2; // start of add instruction
                    while (revS[i] != delimStack.top())
                    {
                        fnew << revS[i];
                        if (i+1 >= revS.size()) // reach the end and did not find delimiter
                        {
                            return false;
                        }
                        i++;
                    }
                    // i reached the ending delimiter
                    delimStack.pop();
                    i++;
                    break;
                }
                else
                    return false; // not enough characters to have both delimiter after +
                break;
            case '#': {
                i++;
                // get offset (up til ',')
                string offsetS;
                int offset;
                while (revS[i] != ',')
                {
                    offsetS += revS[i];
                    if (i+1 >= revS.size()) // reach the end and did not find ','
                    {
                        return false;
                    }
                    i++;
                }
                
                if (!(offsetS == ""))
                {
                    offset = stoi(offsetS);
                }
                else
                    return false; // not a valid offset
                
                
                i++;
                // get length
                string lenS;
                int len;
                while (revS[i] != '+' && revS[i] != '#' && i < revS.size())  // MAKE SURE NEXT HAS TO BE + (?)
                {
                    lenS += revS[i];
                    i++;
                }
                
                if (!(lenS == ""))
                {
                    len = stoi(lenS);
                }
                else
                    return false; // not a valid length
                
                
                if (offset + len <= oldS.size())
                {
                    string seqToCopy = oldS.substr(offset, len);
                    fnew << seqToCopy;
                }
                else
                    return false;
                
                break; }
            case '\n':
            case '\r':
                i++;
                break;
            default:
                return false;
                break;
        }
    }
    return true; // after finishing
}




//===============================================



////////////////////////////////////////
// main (test code)
////////////////////////////////////////


#ifndef TESTNUM
#define TESTNUM 0
#endif


#if TESTNUM == 0

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <filesystem>
using namespace std;

bool runtest(string oldName, string newName, string revisionName, string newName2)
{
    if (revisionName == oldName  ||  revisionName == newName  ||
        newName2 == oldName  ||  newName2 == revisionName  ||
        newName2 == newName)
    {
        cerr << "Files used for output must have names distinct from other files" << endl;
        return false;
    }
    ifstream oldFile(oldName, ios::binary);
    if (!oldFile)
    {
        cerr << "Cannot open " << oldName << endl;
        return false;
    }
    ifstream newFile(newName, ios::binary);
    if (!newFile)
    {
        cerr << "Cannot open " << newName << endl;
        return false;
    }
    ofstream revisionFile(revisionName, ios::binary);
    if (!revisionFile)
    {
        cerr << "Cannot create " << revisionName << endl;
        return false;
    }
    createRevision(oldFile, newFile, revisionFile);
    revisionFile.close();
    
    oldFile.clear();   // clear the end of file condition
    oldFile.seekg(0);  // reset back to beginning of the file
    ifstream revisionFile2(revisionName, ios::binary);
    if (!revisionFile2)
    {
        cerr << "Cannot read the " << revisionName << " that was just created!" << endl;
        return false;
    }
    ofstream newFile2(newName2, ios::binary);
    if (!newFile2)
    {
        cerr << "Cannot create " << newName2 << endl;
        return false;
    }
    assert(revise(oldFile, revisionFile2, newFile2));
    newFile2.close();
    
    newFile.clear();
    newFile.seekg(0);
    ifstream newFile3(newName2, ios::binary);
    if (!newFile)
    {
        cerr << "Cannot open " << newName2 << endl;
        return false;
    }
    if ( ! equal(istreambuf_iterator<char>(newFile), istreambuf_iterator<char>(),
                 istreambuf_iterator<char>(newFile3), istreambuf_iterator<char>()))
    {
        cerr << newName2 << " is not identical to " << newName
        << "; test FAILED" << endl;
        return false;
    }
    return true;
}

int main()
{
    assert(runtest("/Users/ellenchen/Desktop/p4test/mallmart1.txt", "/Users/ellenchen/Desktop/p4test/mallmart2.txt", "/Users/ellenchen/Desktop/p4test/mallmart3.txt", "/Users/ellenchen/Desktop/p4test/mallmart4.txt"));
    cerr << "mallmart Test PASSED" << endl;
    filesystem::path filePath1 = "/Users/ellenchen/Desktop/p4test/mallmart3.txt";
    cerr << "Revision file size is: " << filesystem::file_size(filePath1) << " bytes" << endl;

    assert(runtest("/Users/ellenchen/Desktop/p4test/greeneggs1.txt", "/Users/ellenchen/Desktop/p4test/greeneggs2.txt", "/Users/ellenchen/Desktop/p4test/greeneggs3.txt", "/Users/ellenchen/Desktop/p4test/greeneggs4.txt"));
    cerr << "greeneggs Test PASSED" << endl;
    filesystem::path filePath2 = "/Users/ellenchen/Desktop/p4test/greeneggs3.txt";
    cerr << "Revision file size is: " << filesystem::file_size(filePath2) << " bytes" << endl;
    
    assert(runtest("/Users/ellenchen/Desktop/p4test/warandpeace1.txt", "/Users/ellenchen/Desktop/p4test/warandpeace2.txt", "/Users/ellenchen/Desktop/p4test/warandpeace3.txt", "/Users/ellenchen/Desktop/p4test/warandpeace4.txt"));
    cerr << "warandpeace Test PASSED" << endl;
    filesystem::path filePath3 = "/Users/ellenchen/Desktop/p4test/warandpeace3.txt";
    cerr << "Revision file size is: " << filesystem::file_size(filePath3) << " bytes" << endl;
    
    assert(runtest("/Users/ellenchen/Desktop/p4test/strange1.txt", "/Users/ellenchen/Desktop/p4test/strange2.txt", "/Users/ellenchen/Desktop/p4test/strange3.txt", "/Users/ellenchen/Desktop/p4test/strange4.txt"));
    cerr << "strange Test PASSED" << endl;
    filesystem::path filePath4 = "/Users/ellenchen/Desktop/p4test/strange3.txt";
    cerr << "Revision file size is: " << filesystem::file_size(filePath4) << " bytes" << endl;
    
//
    // g32
//    assert(runtest("mallmart1.txt", "mallmart2.txt", "mallmart3.txt", "mallmart4.txt"));
//    cerr << "mallmart Test PASSED" << endl;
//    filesystem::path filePath1 = "mallmart3.txt";
//    cerr << "Revision file size is: " << filesystem::file_size(filePath1) << " bytes" << endl;
//
//    assert(runtest("greeneggs1.txt", "greeneggs2.txt", "greeneggs3.txt", "greeneggs4.txt"));
//    cerr << "greeneggs Test PASSED" << endl;
//    filesystem::path filePath2 = "greeneggs3.txt";
//    cerr << "Revision file size is: " << filesystem::file_size(filePath2) << " bytes" << endl;
//
//    assert(runtest("warandpeace1.txt", "warandpeace2.txt", "warandpeace3.txt", "warandpeace4.txt"));
//    cerr << "warandpeace Test PASSED" << endl;
//    filesystem::path filePath3 = "warandpeace3.txt";
//    cerr << "Revision file size is: " << filesystem::file_size(filePath3) << " bytes" << endl;
//    
//    assert(runtest("strange1.txt", "strange2.txt", "strange3.txt", "strange4.txt"));
//    cerr << "strange Test PASSED" << endl;
//    filesystem::path filePath4 = "strange3.txt";
//    cerr << "Revision file size is: " << filesystem::file_size(filePath4) << " bytes" << endl;
    
}


#else

#include <iostream>
#include <sstream>  // for istringstream and ostringstream
#include <string>
#include <cassert>
using namespace std;

void runtest(string oldtext, string newtext)
{
    istringstream oldFile(oldtext);
    istringstream newFile(newtext);
    ostringstream revisionFile;
    createRevision(oldFile, newFile, revisionFile);
    string result = revisionFile.str();
    cout << "The revision file length is " << result.size()
    << " and its text is " << endl;
    cout << result << endl;
    
    oldFile.clear();   // clear the end of file condition
    oldFile.seekg(0);  // reset back to beginning of the stream
    istringstream revisionFile2(result);
    ostringstream newFile2;
    assert(revise(oldFile, revisionFile2, newFile2));
    assert(newtext == newFile2.str());
}

int main()
{
    runtest("There's a bathroom on the right.",
            "There's a bad moon on the rise.");
    runtest("ABCDEFGHIJBLAHPQRSTUVPQRSTUV",
            "XYABCDEFGHIJBLETCHPQRSTUVPQRSTQQ/OK");
    cout << "All tests passed" << endl;
}


#endif
