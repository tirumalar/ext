#include <iostream>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>
#include "KeyMgr.h"
using namespace std;

void ProcessFileString (char* arg, char* cmd, std::string &inputFile, std::string &outputFile, std::string& key)
{
	if(strncasecmp(arg,"-i",2)==0)
	{
		inputFile = cmd;
	}
	else if(strncasecmp(arg,"-o",2)==0)
	{
		outputFile = cmd;
	}
	else if(strncasecmp(arg,"-k",2)==0)
	{
		key = cmd;
	}

}

void ShowFileCommand (char* app, bool decrypt)
{
	if (decrypt)
	{
		cout << "Decrypting file command but not all parameters sent " << endl;
		cout << "-d : Decrypt file, parameters are inputfile outputfile" <<endl;
		cout << "Usage : " << app << " -d -i inputfile -o outputfile" <<endl;
	}
	else
	{
		cout << "Encrypting file command but not all parameters sent " << endl;
		cout << "-d : Encrypt file, parameters are inputfile outputfile" <<endl;
		cout << "Usage : " << app << " -e -i inputfile -o outputfile" <<endl;
	}
}

void ShowCommand (char* app)
{
	cout << "Supported parameters : " << endl;
	cout << "-a : Generate keys and add/update in db, parameters are numberofdays isDevice outputpem outkey" <<endl;
	cout << "Usage : " << app << " -a -i index -n numberofdays -d isDevice -h hostname" <<endl;
	cout << "-g : Generate bin file using the cert and key, parameters are index of device cert, index of host cert and key and output bin file" <<endl;
	cout << "Usage : " << app << " -g -d index of device -p index of pc -o Output_bin_file" <<endl;
	cout << "-c : Generate keys and bin file without add/update in db, parameters are numberofdays isDevice hostname output bin file" <<endl;
	cout << "Usage : " << app << " -c -n numberofdays -d isDevice -h hostname -o Output_bin_file" <<endl;
	cout << "-p : Adding keys to db, parameters are index, isDevice, hostname, certificate file,  private key file" <<endl;
	cout << "Usage : " << app << " -p -i index -d isDevice -h hostname -c certificate_file -k private_key_file" <<endl;
	cout << "-r : Deleting non-device keys from db except 'eyelock-pc' " <<endl;
	cout << "Usage : " << app << " -r " <<endl;
	cout << "-r : Get the number of keys in db " <<endl;
	cout << "Usage : " << app << " -n " <<endl;


	cout << "-d : Decrypt file, parameters are inputfile outputfile" <<endl;
	cout << "Usage : " << app << " -d -i inputfile -o outputfile" <<endl;
	cout << "-e : Encrypt file, parameters are inputfile outputfile" <<endl;
	cout << "Usage : " << app << " -e -i inputfile -o outputfile -k key" <<endl;
}

int acquireProcessLock(char *name) {
	char lockName[100];
	sprintf(lockName, "%s.lock", name);

	struct flock fl;

	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 1;

	int fdlock;
	if ((fdlock = open(lockName, O_WRONLY|O_CREAT, 0666)) == -1)
		return 0;

	if (fcntl(fdlock, F_SETLK, &fl) == -1)
		return 0;

	return 1;
}

/*
int main(int argc, char* argv[])
{
	KeyMgr km;
	std::string hostname = "nanonxt380";
	std::string numDays = "7305";
	std::string certFile = "temp.crt";
	std::string pkeyFile = "temp.key";
	km.GenerateCertKey(hostname, 123456, numDays, false, certFile, pkeyFile);
	km.WriteBinFile("certKey.bin");


	km.ReplaceCertKeyOnDevice(certFile, pkeyFile);
	km.AddToDatabase(-1, certFile, pkeyFile, hostname, false);
}
*/

int main(int argc, char* argv[])
{
	printf("Start Key Management\n");
	//Let me get a lock as only one executable will be enabled.
	if (!acquireProcessLock(argv[0])) {
		printf("Could not lock, another instance may be running...Exiting!");
		return -1;
	}

	// We should have at least 2 arguments
	if (argc >= 2)
	{
		KeyMgr km1;
		std::string inputFile,key;
		std::string outputFile;
		bool decrypt =false, encrypt = false, generate = false, add = false, create = false, push = false, remove = false, keysnumber = false;

		if(strncasecmp(argv[1],"-a",2)==0)
		{
			add = true;
		}
		else if(strncasecmp(argv[1],"-d",2)==0)
		{
			decrypt = true;
		}
		else if(strncasecmp(argv[1],"-e",2)==0)
		{
			encrypt = true;
		}
		else if(strncasecmp(argv[1],"-g",2)==0)
		{
			generate = true;
		}
		else if(strncasecmp(argv[1],"-c",2)==0)
		{
			create = true;
		}
		else if(strncasecmp(argv[1],"-p",2)==0)
		{
			push = true;
		}
		else if(strncasecmp(argv[1],"-r",2)==0)
		{
			remove = true;
		}
		else if(strncasecmp(argv[1],"-n",2)==0)
		{
			keysnumber = true;
		}
		else
		{
			ShowCommand (argv[0]);
		}
		if (decrypt || encrypt)
		{
			// We should have at least 6 arguments
			if (argc >= 6)
			{
				ProcessFileString (argv[2], argv[3], inputFile, outputFile,key);
				ProcessFileString (argv[4], argv[5], inputFile, outputFile,key);
				if ((inputFile.size() > 0) && (outputFile.size() > 0))
				{
					if (decrypt)
					{
						cout << "Decrypting file  : Input file :  " << inputFile  << " Output file :  " << outputFile << endl;
						km1.DecryptFile((char*)inputFile.c_str(), (char*)outputFile.c_str());
					}
					if (encrypt)
					{

						ProcessFileString (argv[6], argv[7], inputFile, outputFile,key);
						cout << "Encrypting file  : Input file :  " << inputFile  << " Output file :  " << outputFile << endl;
						km1.EncryptFile((char*)inputFile.c_str(), (char*)outputFile.c_str(),(char*)key.c_str());
					}
				}
				else
				{
					ShowFileCommand (argv[0], decrypt);
				}
			}
			else
			{
				ShowFileCommand (argv[0], decrypt);
			}
		}
		else if ((add) && (argc == 10))
		{
			bool device = -1;
			std::string hostname, days;
			int index = -1;
			if(strncasecmp(argv[2],"-i",2)==0)
			{
				index = atoi (argv[3]);
			}
			if(strncasecmp(argv[4],"-n",2)==0)
			{
				days = argv[5];
			}
			if (strncasecmp(argv[6],"-d",2)==0)
			{
				device = (atoi (argv[7])) ? true : false;
			}
			if (strncasecmp(argv[8],"-h",2)==0)
			{
				hostname = argv[9];
			}

			cout << "Generating keys and adding to db : Index :" << index << "Num Days : " << days;
			cout << " IsDevice :  " << (device ? "true": "false") << "; host name : " << hostname << endl;
			if (((hostname.length() == 0) && (!device))|| (days.length() == 0) || (device == -1) || ((!device) && (index == 1)))
			{
				cout << "Invalid input  " << endl;
				cout << "-a : Generate keys and add/update in db, parameters are numberofdays isDevice outputpem outkey" <<endl;
				cout << "Usage : " << argv[0] << " -a -i index -n numberofdays -d isDevice -h hostname" <<endl;
				return 0;
			}

			if (device)
			{
				index = 1;
				size_t len =0;
				char* line = NULL;
				FILE *fp = fopen ("/etc/hostname", "r");
				int size = getline(&line, &len, fp);
				if (size > 0)
				{
					hostname.clear();
					hostname.insert(0, line);
					hostname.resize (hostname.size () - 1);
				}
				printf ("host name : %s", hostname.c_str());
				fclose (fp);
			}
			km1.GenerateAndAddCertKey(index, hostname, 123456, days, device);
		}
		else if ((generate) && (argc == 8))
		{
			std::string outputFile;
			int indexOfDevice, indexOfPC;
			indexOfDevice = indexOfPC = -100;
			if(strncasecmp(argv[2],"-d",2)==0)
			{
				indexOfDevice = atoi (argv[3]);
			}
			if (strncasecmp(argv[4],"-p",2)==0)
			{
				indexOfPC =  atoi (argv[5]);
			}
			if (strncasecmp(argv[6],"-o",2)==0)
			{
				outputFile = argv[7];
			}
			cout << "Generating bin file : Index of device record :  " << indexOfDevice  << " Index of PC record :  ";
			cout  << indexOfPC << " Output bin File : " << outputFile << endl;

			if ((indexOfDevice < 1) || (indexOfPC < 1) || (outputFile.length() == 0))
			{
				cout << "Invalid input  " << endl;
				cout << "-g : Generate bin file using the cert and key, parameters are index of device record, ";
				cout << "index of pc record and output bin file" <<endl;
				cout << "Usage : " << argv[0] << " -g -d index of device record -p index of pc record -o Output_bin_file" << endl;
				return 0;
			}
			km1.GenerateBinFile(indexOfDevice, indexOfPC, outputFile);
		}
		else if ((create) && (argc == 10))
		{
			bool device = -1;
			std::string hostname, days, outBinFilename;
			if(strncasecmp(argv[2],"-n",2)==0)
			{
				days = argv[3];
			}
			if (strncasecmp(argv[4],"-d",2)==0)
			{
				device = (atoi (argv[5])) ? true : false;
			}
			if (strncasecmp(argv[6],"-h",2)==0)
			{
				hostname = argv[7];
			}
			if (strncasecmp(argv[8],"-o",2)==0)
			{
				outBinFilename = argv[9];
			}

			cout << "Generating keys. Num Days :  " << days << "; Output bin file: " << outBinFilename << endl;
			cout << " IsDevice :  " << (device ? "true": "false") << "; host name : " << hostname << endl;
			if (((hostname.length() == 0) && (!device))|| (days.length() == 0) || (device == -1))
			{
				cout << "Invalid input  " << endl;
				cout << "-c : Generate keys and bin file without add/update in db, parameters are numberofdays, isDevice, hostname, output bin file" <<endl;
				cout << "Usage : " << argv[0] << " -c -n numberofdays -d isDevice -h hostname -o Output_bin_file" <<endl;
				return 0;
			}

			if (device)
			{
				size_t len =0;
				char* line = NULL;
				FILE *fp = fopen ("/etc/hostname", "r");
				int size = getline(&line, &len, fp);
				if (size > 0)
				{
					hostname.clear();
					hostname.insert(0, line);
					hostname.resize (hostname.size () - 1);
				}
				printf ("host name : %s", hostname.c_str());
				fclose (fp);
			}
			string certFile = hostname + ".crt";
			string pkeyFile = hostname + ".key";
			km1.GenerateCertKey(hostname, 123456, days, device);
			int deviceIndex = 1;
			km1.GenerateBinFile(deviceIndex, certFile, pkeyFile, outBinFilename);
		}
		else if ((push) && (argc == 12))
		{
			bool device = -1;
			std::string hostname, certFile, pkeyFile;
			int index = -1;
			if(strncasecmp(argv[2],"-i",2)==0)
			{
				index = atoi (argv[3]);
			}
			if (strncasecmp(argv[4],"-d",2)==0)
			{
				device = (atoi (argv[5])) ? true : false;
			}
			if (strncasecmp(argv[6],"-h",2)==0)
			{
				hostname = argv[7];
			}
			if (strncasecmp(argv[8],"-c",2)==0)
			{
				certFile = argv[9];
			}
			if (strncasecmp(argv[10],"-k",2)==0)
			{
				pkeyFile = argv[11];
			}

			cout << "Adding keys to db.  Device index: " << index << "; certificate file: " << certFile << "; private key file: " << pkeyFile << endl;
			cout << " IsDevice:  " << (device ? "true": "false") << "; host name: " << hostname << endl;
			if (((hostname.length() == 0) && (!device)) || (device == -1) || ((!device) && (index == 1)))
			{
				cout << "Invalid input  " << endl;
				cout << "-p : Adding keys to db, parameters are index, isDevice, hostname, certificate file,  private key file" <<endl;
				cout << "Usage : " << argv[0] << " -p -i index -d isDevice -h hostname -c certificate_file -k private_key_file" <<endl;
				return 0;
			}

			if (device)
			{
				size_t len =0;
				char* line = NULL;
				FILE *fp = fopen ("/etc/hostname", "r");
				int size = getline(&line, &len, fp);
				if (size > 0)
				{
					hostname.clear();
					hostname.insert(0, line);
					hostname.resize (hostname.size () - 1);
				}
				printf ("host name : %s", hostname.c_str());
				fclose (fp);
				km1.ReplaceCertKeyOnDevice(certFile, pkeyFile);
			}

			km1.LoadCertKey(certFile, pkeyFile, device);
			km1.AddToDatabase(index, hostname, device);
		}
		else if ((remove) && (argc == 2))
		{
			cout << "-r : Deleting non-device keys from db except 'eyelock-pc' " <<endl;
			km1.DeleteAllKeys();
		}
		else if ((keysnumber) && (argc == 2))
		{
			cout << "-n : The number of keys in database: " << endl;
			cout << km1.GetKeyNumber() << endl;
		}
		else
		{
			ShowCommand (argv[0]);
		}
	}
	else
	{
		ShowCommand (argv[0]);
	}
	return 0;
}

