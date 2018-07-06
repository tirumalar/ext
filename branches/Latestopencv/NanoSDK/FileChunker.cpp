#include<FileChunker.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <md5.h>
#include <EyelockNanoDevice_server.h>
#include "UtilityFunctions.h"
#include "logging.h"
#include <sys/stat.h>

const char logger[] = "FileChunker";

using namespace std;
bool isFileExist(const char* fname){
	FILE *fptr = fopen(fname,"r");
	bool ret = false;
	if(fptr){
		ret = true;
		fclose(fptr);
	}
	return ret;
}

int getFileSize(FILE* fp)
{
  fseek(fp,0,SEEK_END);
  int sz = ftell(fp);
  fseek(fp,0,SEEK_SET);
  return sz;
}

static int string2Enum(std::string retString){
	if(retString.compare("SUCCESS") == 0){
			return SUCCESS;
		}
		else if(retString.compare("CONNECTION_FLOW_ERROR") == 0){
			return CONNECTION_FLOW_ERROR;
		}
		else if(retString.compare("FILE_ACCESS_ERROR") == 0){
				return FILE_ACCESS_ERROR;
		}
	return UNSUCCESSFUL;
}

void FileChunker::receiveChunkAndAppendFile(std::map<std::string, std::string> & _return, const std::vector<std::string> & chunkList){

	static int current_bucketNo;
	static int total_buckets;
	string header = chunkList[0];
	int chunkSize = atoi(chunkList[1].c_str());

	std::string checkSum = chunkList[3];
	if(strncmp(MD5(chunkList[2]).hexdigest().c_str(),checkSum.c_str(),32) != 0){
		_return["RET_VALUE"] = "CONNECTION_FLOW_ERROR";
		return;
	}
	FILE* fp = 0;
	string filePath = chunkList[4];

	if(strncmp(header.c_str(),"START:",6) == 0 || header.compare("FULL") == 0){
		if(isFileExist(filePath.c_str())){
			stringstream ss;
			time_t t = time(0);
			ss<<filePath<<t<<".bin";
			EyelockLog(logger, INFO, "File %s exists. New name generated: %s", filePath.c_str(), ss.str().c_str());
			filePath = ss.str();
			_return["FILENAME"] = filePath;
		}
		else
		{
			size_t pos = filePath.find_last_of("/");
			string destDir = (string::npos == pos) ? "": filePath.substr(0, pos);
			if (!destDir.empty())
			{
				struct stat s;
				int err = stat(destDir.c_str(), &s);
				if(-1 == err) {
				    if (ENOENT == errno) {
						EyelockLog(logger, INFO, "Path %s will be created", destDir.c_str());
						stringstream ss;
						ss << "mkdir -p " << destDir;
						RunSystemCmd(ss.str().c_str());
						sync();
				    } else {
				    	EyelockLog(logger, ERROR, "Error accessing %s", destDir.c_str());
				    	_return["RET_VALUE"] = "FILE_ACCESS_ERROR";
						return;
				    }
				} else {
				    if(!S_ISDIR(s.st_mode)) {
				    	EyelockLog(logger, ERROR, "Error accessing %s (it's a file)", destDir.c_str());
				    	_return["RET_VALUE"] = "FILE_ACCESS_ERROR";
						return;
				    }
				}
			}
		}
		fp = fopen(filePath.c_str(),"w+b");
		if(header.compare("FULL") == 0){
			current_bucketNo = 0;
		}
		else{
			current_bucketNo = 1;
			string bktNoStr = header.substr(6);
			total_buckets = atoi(bktNoStr.c_str());
		}
		//fp = fopen(filePath.c_str(),"w+b"); // second time???
	}
	else if(header.compare("END") == 0){
		if(total_buckets != (current_bucketNo+1)){
			_return["RET_VALUE"] = "CONNECTION_FLOW_ERROR";
			return;
		}
		fp = fopen(filePath.c_str(),"a+b");
		total_buckets = 0;
		current_bucketNo = 0;
	}
	else if(strncmp(header.c_str(),"BUCKET:",7) == 0){
		string bktNoStr = header.substr(7);
		int bktNo = atoi(bktNoStr.c_str());
		if(current_bucketNo != (bktNo-1)){
			current_bucketNo = 0;
			_return["RET_VALUE"] = "CONNECTION_FLOW_ERROR";
			return;
		}
		current_bucketNo = bktNo;
		fp = fopen(filePath.c_str(),"a+b");
	}

	if(fp == 0L){
		_return["RET_VALUE"] = "FILE_ACCESS_ERROR";
		return;
	}

	fwrite(chunkList[2].c_str(),chunkSize,1,fp);
	fclose(fp);

	_return["FILENAME"] = filePath;
	_return["RET_VALUE"] = "SUCCESS";
}




int FileChunker::sendChunkFromFile(std::string inputFile,std::string& outPath,EyelockNano::EyelockNanoDeviceIf* receiver,int bucketSize){

	FILE* fp = fopen(inputFile.c_str(),"rb");

	if(fp == NULL) return FILE_ACCESS_ERROR;
	fseek(fp,0,SEEK_END);
	int fileSz = ftell(fp);
	fseek(fp,0,SEEK_SET);

	std::map<std::string,std::string> returnMap;
	returnMap["FILENAME"] = outPath;
	returnMap["RET_VALUE"] = "SUCCESS";
	int totalBucket = fileSz/bucketSize + (fileSz % bucketSize == 0 ? 0:1);

	if(totalBucket == 1){
		std::vector<string> chunk;
		chunk.push_back("FULL"); // header
		stringstream ss;
		ss<<fileSz;
		chunk.push_back(ss.str());//Size of the chunk

		chunk.push_back("");
		chunk[2].clear();
		chunk[2].resize(bucketSize);
		fread((void*)chunk[2].c_str(),fileSz,1,fp);// Data of the chunk
		chunk.push_back(MD5(chunk[2]).hexdigest());//CheckSum
		chunk.push_back(outPath);
		receiver->receiveChunkAndAppendFile(returnMap,chunk);
	}
	else{
		for(int i = 1;i <= totalBucket;i++){
			int chunkSize = 0;
			std::vector<string> chunk;
			if(i == 1){
				stringstream ss;
				ss<<"START:"<<totalBucket;
				chunk.push_back(ss.str());
				chunkSize = bucketSize;
				//std::cout<<"Copying";

			}
			else if(i == totalBucket){
				chunk.push_back("END");
				chunkSize = fileSz % bucketSize;
				//std::cout<<"End";
			}
			else{
				stringstream ss;
				ss<<"BUCKET:"<<i;
				chunk.push_back(ss.str());
				chunkSize = bucketSize;
			}

			stringstream ss;
			ss<<chunkSize;
			chunk.push_back(ss.str());
			chunk.push_back("");
			chunk[2].resize(chunkSize);
			fread((void*)chunk[2].c_str(),chunkSize,1,fp);
			chunk.push_back(MD5(chunk[2]).hexdigest());//CheckSum
			chunk.push_back(outPath);
			receiver->receiveChunkAndAppendFile(returnMap,chunk);
			outPath = returnMap["FILENAME"];
			//std::cout<<".";
			if(returnMap["RET_VALUE"].compare("SUCCESS") != 0){
				break;
			}
		}


	}
	outPath = std::string(returnMap["FILENAME"].c_str());
	fclose(fp);

	return string2Enum(returnMap["RET_VALUE"]);

}

void FileChunker::neededChunkFromFile(std::vector<std::string> & _return, const std::map<std::string, std::string> & neededchunkInfo){
	std::string filename = neededchunkInfo.find("FILENAME")->second;
	int bucketSize = atoi(neededchunkInfo.find("BUCKET_SIZE")->second.c_str());
	std::string perviousHeader = neededchunkInfo.find("PREVIOUS_HEADER")->second;

	FILE* fp = fopen(filename.c_str(),"rb");
	if(fp == NULL){
		_return.push_back("FILE_ACCESS_ERROR");
		return;
	}

	int fileSize = getFileSize(fp);
	int chunkSize = 0;
	if(perviousHeader.compare("NEW") == 0){
		if(bucketSize > fileSize){
			_return.push_back("FULL");
			chunkSize = fileSize;
		}
		else{
			int totalBucket = fileSize/bucketSize + (fileSize%bucketSize == 0?0:1);
			stringstream ss;
			ss<<"START:"<<totalBucket;
			_return.push_back(ss.str());
			chunkSize = bucketSize;
			//std::cout<<"Copying";
		}
	}
	else if(strncmp(perviousHeader.c_str(),"BUCKET:",7) == 0 || strncmp(perviousHeader.c_str(),"START:",6) == 0){
		int bktNo = 1;
		if(strncmp(perviousHeader.c_str(),"BUCKET:",7) == 0){
			string bktNoStr = perviousHeader.substr(7);
			bktNo = atoi(bktNoStr.c_str());
			//std::cout<<".";
		}

		size_t filePosition = bktNo * bucketSize;
		fseek(fp,filePosition,SEEK_SET);

		int totalSize = filePosition + bucketSize;
		if(totalSize > fileSize){
			_return.push_back("END");
			chunkSize = fileSize - filePosition;
			//std::cout<<"End";
		}
		else{
			stringstream ss;
			ss<<"BUCKET:"<<(bktNo+1);
			_return.push_back(ss.str());
			chunkSize = bucketSize;
		}

	}

	stringstream ss;
	ss<<chunkSize;
	_return.push_back(ss.str());
	_return.push_back("");
	_return[2].resize(chunkSize);
	fread((void*)_return[2].c_str(),chunkSize,1,fp);
	_return.push_back(MD5(_return[2]).hexdigest());//CheckSum of the data
	_return.push_back(filename);
}

int FileChunker::receiveChunksFromDevice(std::string file2Read,std::string& outFile,EyelockNano::EyelockNanoDeviceIf* sender,int bucketSize){

	if(isFileExist(outFile.c_str())){
		stringstream ss;
		time_t t = time(0);
		ss<<outFile<<t<<".bin";
		outFile = ss.str();
		//cout<<outFile<<endl;
	}

	std::map<std::string, std::string> neededchunkInfo;
	neededchunkInfo["FILENAME"] = file2Read;
	neededchunkInfo["PREVIOUS_HEADER"] = "NEW";
	stringstream ss;
	ss<<bucketSize;
	neededchunkInfo["BUCKET_SIZE"] = ss.str();
	std::string outheader = "NEW";

	FILE* fp = fopen(outFile.c_str(),"wb");
	if(fp == NULL) return FILE_ACCESS_ERROR;

	int totalChunks(0),ChunkCount(0);
	int status = SUCCESS;
	while(outheader.compare("END") != 0 && outheader.compare("FULL") != 0){
		std::vector<std::string> outChunk;
		sender->neededChunkFromFile(outChunk,neededchunkInfo);
		outheader = outChunk[0];

		int chunkSize = 0;
		if(outheader.compare("FULL") == 0){
			chunkSize = atoi(outChunk[1].c_str());
		}
		else if(strncmp(outheader.c_str(),"START:",6) == 0){
			chunkSize = atoi(outChunk[1].c_str());
			totalChunks = atoi(outheader.substr(6).c_str());
			//std::cout<<"Copying";
		}
		else if(strncmp(outheader.c_str(),"BUCKET:",7) == 0){
			int currentChunk = atoi(outheader.substr(7).c_str());
			if(currentChunk != ChunkCount+1){
				status = CONNECTION_FLOW_ERROR;
				break;
			}
			chunkSize = atoi(outChunk[1].c_str());
			//std::cout<<".";
		}
		else if(outheader.compare("END") == 0){
			if(totalChunks != ChunkCount+1){
				status = CONNECTION_FLOW_ERROR;
				break;
			}
			chunkSize = atoi(outChunk[1].c_str());
			//std::cout<<"End";
		}
		else{
			break;
		}

		string checkSum = outChunk[3];
		if(strncmp(MD5(outChunk[2]).hexdigest().c_str(),checkSum.c_str(),32) != 0){
			status = CONNECTION_FLOW_ERROR;
			break;
		}
		fwrite(outChunk[2].c_str(),chunkSize,1,fp);
		neededchunkInfo["PREVIOUS_HEADER"] = outheader;
		ChunkCount++;
	}

	fclose(fp);

	if(status != SUCCESS){
		remove(outFile.c_str());
		outFile = "";
		return status;
	}

	if(outheader.compare("FULL") == 0 || outheader.compare("END") == 0 )
		return SUCCESS;

	return UNSUCCESSFUL;
}


