#include<iostream>
#include<vector>
#include <string>
#include <map>
#include <EyelockNanoDevice.h>



class FileChunker{
public:
	static int sendChunkFromFile(std::string inputFile,std::string& outPath,EyelockNano::EyelockNanoDeviceIf* receiver,int bucketSize);
	static void receiveChunkAndAppendFile(std::map<std::string, std::string> & _return, const std::vector<std::string> & chunkList);
	static void neededChunkFromFile(std::vector<std::string> & _return, const std::map<std::string, std::string> & neededchunkInfo);
	static int receiveChunksFromDevice(std::string file2Read,std::string& outFile,EyelockNano::EyelockNanoDeviceIf* sender,int bucketSize);
};
