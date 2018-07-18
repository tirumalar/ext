
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/utsname.h>

#define MSG_LEN 1024*30
#define MSG_DEL 0x1f

int processCommand(char *str);
char * illegalCharacters(char * str, char * illegal);
bool m_debug = 0;
char* findFirstDelimiter(char * buffer,int bufferlen, char * delimiters);
int main(){
	if (access("/home/mynxtw", F_OK ) != -1)
		m_debug = 1;

    int listenPort = 1234;
	char buffer[MSG_LEN];
	char msg[MSG_LEN];
	char delimiters[32];
	int len = 0;
	sprintf(delimiters, "%c", MSG_DEL);
    // Create a socket
    int s0 = socket(AF_INET, SOCK_STREAM, 0);
    if (s0 < 0) {
        perror("Cannot create a socket"); exit(1);
    }

    // Fill in the address structure containing self address
    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(struct sockaddr_in));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(listenPort);        // Port to listen
    myaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    // Bind a socket to the address
    int res = bind(s0, (struct sockaddr*) &myaddr, sizeof(myaddr));
    if (res < 0) {
        perror("Cannot bind a socket"); exit(1);
    }

    // Set the "LINGER" timeout to zero, to close the listen socket
    // immediately at program termination.
    struct linger linger_opt = { 1, 0 }; // Linger active, timeout 0
    setsockopt(s0, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));

    while (1) {
		// Now, listen for a connection
		res = listen(s0, 1);    // "1" is the maximal length of the queue
		if (res < 0) {
			perror("Cannot listen"); exit(1);
		}

		// Accept a connection (the "accept" command waits for a connection with
		// no timeout limit...)
		struct sockaddr_in peeraddr;
		socklen_t peeraddr_len;

		int s1 = accept(s0, (struct sockaddr*) &peeraddr, &peeraddr_len);
		if (s1 < 0) {
			perror("Cannot accept"); exit(1);
		}

		// A connection is accepted. The new socket "s1" is created
		// for data input/output. The peeraddr structure is filled in with
		// the address of connected entity, print it.
		if (m_debug){
		printf(
			"Connection from IP %d.%d.%d.%d, port %d\n",
			(ntohl(peeraddr.sin_addr.s_addr) >> 24) & 0xff, // High byte of address
			(ntohl(peeraddr.sin_addr.s_addr) >> 16) & 0xff, // . . .
			(ntohl(peeraddr.sin_addr.s_addr) >> 8) & 0xff,  // . . .
			ntohl(peeraddr.sin_addr.s_addr) & 0xff,         // Low byte of addr
			ntohs(peeraddr.sin_port)
		);
		}

        memset(buffer, 0, MSG_LEN);
        memset(msg, 0, MSG_LEN);

		res = read(s1, buffer, MSG_LEN);
		char * firstDelimiter = findFirstDelimiter(buffer, 1024, delimiters);
		*firstDelimiter = ' ';
		int lengthBytes = (int)(firstDelimiter - buffer);
		int msglen;
		sscanf(buffer,"%d", &msglen);
		*firstDelimiter = MSG_DEL;
		int totalRead = res;
		while(totalRead < msglen)
		{
			totalRead += read(s1, buffer+totalRead, msglen -totalRead );

		}

		if (res < 0) {
			perror("Read error"); exit(1);
		}
		if (m_debug)
			printf("Received %d bytes:\n%s\n", res, buffer);

		// process command

		int cmdID = processCommand(buffer);

		usleep(100000);		// wait 100ms

		if (cmdID == 999 || cmdID == 998) {
			close(s1);
			continue;
		}

#if 0
		// get result
        if (access("/home/mynxtw", F_OK ) != -1) {
        	// open file
            FILE *fp = fopen("/home/mynxtw", "r");
            if (fp == NULL) {
                printf("Failed to open file %s\n", "mynxtw");
                continue;
            }
            memset(buffer, 0, MSG_LEN);
            len = fread (msg,1, MSG_LEN,fp);
            if (len == 0) {
                printf("%d: Failed to read command file mynxtw\n", cmdID);
                len = sprintf(buffer, "0 0");

            }
            else
            {
            	if (m_debug)
            		printf("success read command file mynxtw: %s\n", msg);
                len = sprintf(buffer, "%d %s", len, msg);

            }
            fclose (fp);

            // remove file
            //if( remove("/home/mynxtw") != 0 )
                //printf("Failed to remove %s file\n", "mynxtw");

        }
        else
#endif
        {
        	len = sprintf(buffer, "0 0");
        }

		write(s1, buffer, len);

		close(s1);          // Close the data socket
    }
    res = close(s0);    // Close the listen socket
    return 0;
}
char* findFirstDelimiter(char * buffer,int bufferlen, char * delimiters)
		{
			for( int i = 0; i < bufferlen; i++)
				for(int j= 0 ; j < strlen(delimiters); j++)
					if(*(buffer + i) == delimiters[j])
						return buffer+i;
			return 0;
		}
//expects null terminated string for str and illegal
//efficiency: O(n, j)
char * illegalCharacters(char * str, char * illegal)
{
	int len = strlen(str);
	int illlen = strlen(illegal);
	for(int i = 0; i < len; i++)
	{
		for(int j = 0; j < illlen; j++)
			if(str[i] == illegal[j])
				return str+i;
	}
	return NULL;
}
void makeStrings(char* str,  char * tokens)
{
	int len = strlen(str);
		int illlen = strlen(tokens);
		for(int i = 0; i < len; i++)
		{
			for(int j = 0; j < illlen; j++)
				if(str[i] == tokens[j])
					 str[i] =0; //create null terms
		}
}
int isReadOnlyFs()
{
	struct utsname sysInfo;
	if (uname(&sysInfo))
	{
		return 0;
	}
	return (strncmp(sysInfo.release, "3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0", 37) == 0);
}
int processCommand(char *str)
{
	// use atoi to convert token string into int
	char * pch;
	int i = 0;
	char *szParam[8];
	char delimiters[32];
	int nParams = 0;


	//1  2  test.db  hello  4
	sprintf(delimiters, "%c", MSG_DEL);
	pch = strtok (str, delimiters);
	while (pch != NULL)
	{
		pch = strtok (NULL, delimiters);
		szParam[i++] = pch;
	}

	nParams = i-1;
	if (m_debug){
	    for (i = 0; i < nParams; ++i)
	        printf("%s\n", szParam[i]);
	}

	char szTokenBuffer[1024];
	//char filename[1024];
	int nToken = atoi(szParam[0]);
	int len = 0;

    // check special char in command line "!”$&’`()*,;<=>?@[\]^{|}"
	//pch = strtok (str,"!$&’`()*,;<=?@[]^{|}\"");

	if (nToken == 47 || nToken == 48 || nToken == 49) {
		pch = illegalCharacters(str,"”");
	}
	else {
		pch = illegalCharacters(str,"!”$&’`()*,;<=?@[]^{|}");
	}
	if (pch != NULL) {
		printf("pch %s\n", pch);
		printf("nxtW error!!! - received special character in package: %s\n", str);
		// clock sync return code - 1 ==> IP address or host name is invalid
		if (nToken == 42) {
			system("echo '1' >/home/nxtwResult");
		}
		return 0;
	}

	makeStrings(str, delimiters);
	switch (nToken)
	{
		// ApplicationDetails.php
		case 1:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/www/scripts/psocver.sh");
			break;
		}

		// ApplicationDetails.php
		case 2:
		{
			len = sprintf(szTokenBuffer, "/home/www/scripts/psocver.sh");
			break;
		}

		// interfaceeditor.php
		case 3:
		{
			len = sprintf(szTokenBuffer, "cp /etc/hostname /etc/FactoryHostname");
			break;
		}

		// interfaceeditor.php
		case 4:
		{
			// Requires 1 parameters
#ifndef HBOX_PG
			const char* path = (isReadOnlyFs()) ? "/tmp/etc/hostname" : "/etc/hostname";
			len = sprintf(szTokenBuffer, "echo %s > '%s'", szParam[1], path);
#else
			len = sprintf(szTokenBuffer, "echo %s > '/etc/hostname'", szParam[1]);
#endif
			system(szTokenBuffer);
			return nToken;
		}

		// interfaceeditor.php
		case 5:
		{
		   // Requires 1 parameters
			len = sprintf(szTokenBuffer, "[ -e %s ] && cp %s %s || echo > %s", szParam[1], szParam[1], szParam[2], szParam[1]);
			system(szTokenBuffer);
			return nToken;
		}

		// interfaceeditor.php
		case 6:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "echo > %s;echo > %s", szParam[1], szParam[2]);
			system(szTokenBuffer);
			return nToken;
		}

		// interfaceeditor.php
		case 7:
		{
			// Requires 2 parameters
			//len = sprintf(szTokenBuffer, "echo -e %s >> /etc/network/%s-restore", szParam[1], szParam[2]);
			//system(szTokenBuffer);
			//sprintf(filename, "%s", szParam[2]);
			//FILE * f = fopen(filename,"w");
			//if (f) {
			//	fwrite(szParam[1],1, strlen(szParam[1]),f);
			//	fclose(f);
			//}
#ifndef HBOX_PG
			const char* path = (isReadOnlyFs()) ? "/tmp/etc/network" : "/etc/network";
			len = sprintf(szTokenBuffer, "cp %s %s/%s", szParam[1], path, szParam[2]);
#else
			len = sprintf(szTokenBuffer, "cp %s /etc/network/%s", szParam[1], szParam[2]);
#endif
			break;

		}

		// interfaceeditor.php
		case 8:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "ping -w 2 -c 1 %s", szParam[1]);
			break;
		}

		// interfaceeditor.php
		case 9:
		{
			// Requires 2 parameters
			//len = sprintf(szTokenBuffer, "echo -e %s >> %s", szParam[1], szParam[2]);
			//system(szTokenBuffer);
			//return 9;
#ifndef HBOX_PG
			const char* path = (isReadOnlyFs()) ? "/tmp/etc/network" : "/etc/network";
			len = sprintf(szTokenBuffer, "cp %s %s/%s", szParam[1], path, szParam[2]);
#else
			len = sprintf(szTokenBuffer, "cp %s /etc/network/%s", szParam[1], szParam[2]);
#endif
			break;
		}

		// interfaceeditor.php
		case 10:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "cp %s %s", szParam[1], szParam[2]);
			break;
		}

		// interfaceeditor.php
		case 11:
		{
			// Requires 3 parameters
			len = sprintf(szTokenBuffer, "cp %s %s", szParam[1], szParam[2]);
			break;
		}

		// interfaceeditor.php
		case 12:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/www/scripts/SetIp.sh");
			break;
		}

		// interfaceeditor.php
		case 13:
		{
			// Requires 5 parameters
			len = sprintf(szTokenBuffer, "/home/www/scripts/SetIp.sh %s %s %s %s %s", szParam[1], szParam[2], szParam[3], szParam[4], szParam[5]);
			break;
		}

		// interfaceeditor.php
		case 14:	// Not need
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "echo -e \\\"%s\\\" >>test.txt", szParam[1]);
			system(szTokenBuffer);
			return nToken;
		}

		// interfaceeditor.php
		case 15:
		{
			// Requires 4 parameters
			len = sprintf(szTokenBuffer, "md5sum %s > %s", szParam[1], szParam[2]);
			system(szTokenBuffer);
			return nToken;
		}

		// interfaceeditor.php
		case 16:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "touch %s", szParam[1]);
			system(szTokenBuffer);
			return nToken;
		}

		// interfaceeditor.php
		case 17:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/www/scripts/SetDHCP.sh");
			break;
		}

		// interfaceeditor.php
		case 18:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "rm %s", szParam[1]);
			break;
		}

		// interfaceeditor.php
		case 19:
		{
			// Requires 2 parameters
#ifndef HBOX_PG
			const char* path = (isReadOnlyFs()) ? "/tmp/etc/network" : "/etc/network";
			len = sprintf(szTokenBuffer, "cp %s/%s-bkup  %s/%s", path, szParam[1], path, szParam[2]);
#else
			len = sprintf(szTokenBuffer, "cp /etc/network/%s-bkup  /etc/network/%s", szParam[1], szParam[2]);
#endif
			break;
		}

		// interfaceeditor.php
		case 20:
		{
			// Requires 2 parameters
			len = sprintf(szTokenBuffer, "%s/%s", szParam[1], szParam[2]);
			break;
		}

		// keymgmt.php
		case 21:
		{
			// Requires 3 parameters
			len = sprintf(szTokenBuffer, "cd /home/root;./KeyMgr -c -n %s -d 0 -h %s -o %s", szParam[1], szParam[2], szParam[3]);
			break;
		}

		// keymgmt.php
		case 22:
		{
			// Requires 3 parameters
			len = sprintf(szTokenBuffer, "cd /home/root;./KeyMgr -p -i -1 -d 0 -h %s -c %s -k %s;sync;killall -KILL Eyelock;", szParam[1], szParam[2], szParam[3]);
			break;
		}

		// keymgmt.php
		case 23:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "cp %s /home/root/rootCert/certs", szParam[1]);
			break;
		}

		// keymgmt.php
		case 24:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "cp /home/root/%s.key /home/root/rootCert/certs", szParam[1]);
			break;
		}

		// keymgmt.php
		case 25:
		{
			len = sprintf(szTokenBuffer, "cd /home/root;sync;killall -KILL Eyelock;");
			break;
		}

		// keymgmt.php
		case 26:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/www/scripts/makePFX.sh");
			break;
		}

		// keymgmt.php
		case 27:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "/home/www/scripts/makePFX.sh %s", szParam[1]);
			break;
		}

		// keymgmt.php
		case 28:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "cp %s /home/root/rootCert/certs/download.pfx", szParam[1]);
			break;
		}

		// restarteyelock.php
		case 29:
		{
			len = sprintf(szTokenBuffer, "reboot");
			break;
		}

		// restarteyelock.php
		case 30:
		{
			len = sprintf(szTokenBuffer, "killall -s SIGKILL Eyelock");
			break;
		}

		// restarteyelock.php
		case 31:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/www/scripts/KillApplication.sh");
			break;
		}

		// restarteyelock.php
		case 32:
		{
			len = sprintf(szTokenBuffer, "/home/www/scripts/KillApplication.sh");
			break;
		}

		// rpc.php
		case 33:
		{
			// Requires 4 parameters
			len = sprintf(szTokenBuffer, "cd %s;sh %s rgb %s %s &", szParam[1], szParam[2], szParam[3], szParam[4]);
			break;
		}

		// rpc.php
		case 34:
		{
			// Requires 4 parameters
			len = sprintf(szTokenBuffer, "cd %s;sh %s last %s %s", szParam[1], szParam[2], szParam[3], szParam[4]);
			break;
		}

		// rpc.php
		case 35:
		{
			// Requires 6 parameters
			len = sprintf(szTokenBuffer, "cd %s;./%s %s %s %s %s", szParam[1], szParam[2], szParam[3], szParam[4], szParam[5], szParam[6]);
			break;
		}

		// rpc.php
		case 36:
		{
			// Requires 2 parameters
			len = sprintf(szTokenBuffer, "cd %s;touch %s.run", szParam[1], szParam[2]);
			break;
		}

		// rpc.php
		case 37:
		{
			len = sprintf(szTokenBuffer, "/home/root/scripts/factoryReset.sh");
			break;
		}

		// rpc.php
		case 38:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/www/scripts/KillApplication.sh");
			break;
		}

		// rpc.php
		case 39:
		{
			len = sprintf(szTokenBuffer, "/home/www/scripts/KillApplication.sh");
			break;
		}

		// rpc.php
		case 40:
		{
			//system("i2cset -y 3 0x2e 4 7");
			break;
		}

		// rpc.php
		case 41:
		{
			//system("i2cset -y 3 0x2e 4 8");
			break;
		}

		// rpc.php
		case 42:
		{
			// Requires 1 parameters
			//len = sprintf(szTokenBuffer, "rdate -s %s 2>&1; echo $?", szParam[1]);
			//len = sprintf(szTokenBuffer, "/bin/ntpclient -s -c 2 -i 3 -h %s >/dev/null; echo $?", szParam[1]);
			len = sprintf(szTokenBuffer, "/home/root/scripts/clockSync.sh %s", szParam[1]);
			system(szTokenBuffer);
			return nToken;
		}

		// rpc.php
		case 43:
		{
			len = sprintf(szTokenBuffer, "/sbin/hwclock -w");
			break;
		}

		// rpc.php
		case 44:
		{
			len = sprintf(szTokenBuffer, "date 2>&1");
			system(szTokenBuffer);
			return nToken;
		}

		// rpc.php
		case 45:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "date -u -s %s 2>&1", szParam[1]);
			system(szTokenBuffer);
			return nToken;
		}

		// rpc.php
		case 46:
		{
			len = sprintf(szTokenBuffer, "/sbin/hwclock -w");
			break;
		}

		// rpc.php
		case 47:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "echo -e \"%s\\n%s\\n\" | passwd -a MD5 %s; passwd -l %s", szParam[1], szParam[2], szParam[3], szParam[3]);
			break;
		}

		// rpc.php
		case 48:
		{
			// Requires 2 parameters
			len = sprintf(szTokenBuffer, "echo -e \"%s\\n%s\\n\" | passwd -a MD5; passwd -l %s", szParam[1], szParam[2], szParam[2]);
			break;
		}

		// rpc.php
		case 49:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "passwd -d %s 2>&1; passwd -l %s", szParam[1], szParam[1]);
			system(szTokenBuffer);
			return nToken;
		}

		// upgrade.php
		case 50:
		{
			//len = sprintf(szTokenBuffer, "/home/root/nano_led 100 80 0 1");
			break;
		}

		// upgrade.php
		case 51:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "/home/www/scripts/testExist.sh \"%s\"", szParam[1]);
			break;
		}

		// upgrade.php
		case 52:
		{
			// Requires 7 parameters
			len = sprintf(szTokenBuffer, "cd %s;/home/root/KeyMgr -d -i %s -o %s;mv %s %s", szParam[1], szParam[2], szParam[3], szParam[4], szParam[5]);
			break;
		}

		// upgrade.php
		case 53:
		{
			len = sprintf(szTokenBuffer, "rm -rf /home/upgradeTemp");
			break;
		}

		// upgrade.php
		case 54:
		{
			len = sprintf(szTokenBuffer, "mkdir /home/upgradeTemp");
			break;
		}

		// upgrade.php
		case 55:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/upgradeTemp/root/patch.sh");
			break;
		}

		// upgrade.php
		case 56:
		{
			len = sprintf(szTokenBuffer, "/home/upgradeTemp/root/patch.sh");
			break;
		}

		// upgrade.php
		case 57:
		{
			len = sprintf(szTokenBuffer, "rm /home/upgradeTemp/root/test.db");
			break;
		}

		// upgrade.php
		case 58:
		{
			// Requires 7 parameters
			len = sprintf(szTokenBuffer, "rm /home/upgradeTemp/root/keys.db");
			break;
		}

		// upgrade.php
		case 59:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "cd %s;chmod a+x install.sh;sh install.sh master update", szParam[1]);
			break;
		}

		// upgrade.php
		case 60:
		{
			// Requires 1 parameters
			//len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cd %s;chmod a+x install.sh;sh install.sh slave restore'", szParam[1]);
			break;
		}

		// upgrade.php
		case 61:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "cd %s;chmod a+x install.sh;sh install.sh master restore", szParam[1]);
			break;
		}

		// upgrade.php
		case 62:
		{
			// Requires 2 parameters
			len = sprintf(szTokenBuffer, "cd %s; chmod a+x %s", szParam[1], szParam[2]);
			break;
		}

		// upgrade.php
		case 63:
		{
			len = sprintf(szTokenBuffer, "[ ! -e /home/root/identityNkillEyelock.sh ] && echo 0 || echo 1;");
			break;
		}

		// upgrade.php
		case 64:
		{
			// Requires 1 parameters
			sprintf(szTokenBuffer, "sh /home/root/%s", szParam[1]);
			break;
		}

		// upgrade.php
		case 65:
		{
			len = sprintf(szTokenBuffer, "killall -s SIGKILL bash");
			break;
		}

		// upgrade.php
		case 66:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "killall -s SIGKILL %s", szParam[1]);
			break;
		}

		// uploadcertificate.php
		case 67:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "mkdir %s", szParam[1]);
			break;
		}

		// uploadcertificate.php
		case 68:
		{
			len = sprintf(szTokenBuffer, "openssl enc -e -aes-256-cbc -in /home/portableTemplateCerts/selectedPFX.txt -out /home/portableTemplateCerts/selectedPFX.enc -K 00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF -iv FFEEDDCCBBAA99887766554433221100");
			break;
		}

		// uploadcertificate.php
		case 69:
		{
			len = sprintf(szTokenBuffer, "openssl enc -d -aes-128-cbc -in /home/portableTemplateCerts/selectedPFX.enc -out /home/portableTemplateCerts/selectedPFX.dec -K 00112233445566778899AABBCCDDEEFF00112233445566778899AABBCCDDEEFF -iv FFEEDDCCBBAA99887766554433221100");
			break;
		}

		// uploadcertificate.php
		case 70:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "rm %s", szParam[1]);
			break;
		}
		case 88:
		{
#ifndef HBOX_PG
			len = sprintf(szTokenBuffer, "chown www:www /home/root/keys.db");
#else
			len = sprintf(szTokenBuffer, "chown www-data:www-data /home/root/keys.db");
#endif			
		    break;
		}
		//new cases
		case 101:
		{
			len = sprintf(szTokenBuffer, "/home/www/scripts/psocver.sh");
		    break;
		}
		case 166:
		{
			len = sprintf(szTokenBuffer, "/home/root/reloadinterfaces.sh");
			break;
		}
		case 171:
		{
			len = sprintf(szTokenBuffer, "/home/www/scripts/SetDHCP.sh");
		    break;
		}
		case 180:
		{
			// Requires 1 parameters
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'rm %s'", szParam[1]);
			break;
		}
		case 200:
		{
		     len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'ps -e | grep Eyelock'");
		     break;
		}

		case 201:
		{
#ifndef HBOX_PG
			system("chown -R www:www /tmp/*");
#else		
			system("chown -R www-data:www-data /tmp/*");
#endif
			return nToken;
		}
		case 202:  //need to return the output from this command to the PHP through the socket
		{
			len = 0;//sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cd /home/root;rm Eyelock.run'");
			break;
		}
		case 203: //combine these 3 into a single command
		{
			//system("ssh root@192.168.40.2 'cd /home/root;touch Eyelock.run'");
			//system("ssh root@192.168.40.2 'sleep 2'");
			//system("ssh root@192.168.40.2 'sync'");
			return nToken;
		}
		case 204:
		{
			len = 0;//sprintf(szTokenBuffer, "ssh root@192.168.40.2 'mkdir -p %s'", szParam[1]);
			break;
		}
		case 205:
		{
			len = 0;//sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cd %s;tar cf %s %s'", szParam[1], szParam[2], szParam[3]);
			break;
		}
		case 206:
		{
			//system("ssh root@192.168.40.2 reboot");
			return nToken;
		}
		case 207:
		{
			len = 0;//sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cd /home;tar xf %s'",  szParam[1]);
			break;
		}
		case 208:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 %s", szParam[1]);
			break;
		}
		case 209:
		{
			len = sprintf(szTokenBuffer, "cat %s | ssh root@192.168.40.2 'cat > %s/%s'", szParam[1], szParam[2], szParam[3]);
			break;
		}
		case 210:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cd %s;tar xf %s'", szParam[1], szParam[2]);
			break;
		}
		case 211:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cd %s;chmod a+x install.sh;sh install.sh slave update'", szParam[1]);
			break;
		}
		case 212:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'rm -R %s'",szParam[1]);
			break;
		}
		case 213:
		{
			//sprintf("ssh root@192.168.40.2 'cd %s; rm *.*'", szParam[1]);
			break;
		}
		case 250:
		{
			system("scp root@192.168.40.2:/home/root/nxtLog.log /home/slavelogs/nxtLog.log");
			system("scp root@192.168.40.2:/var/log/messages /home/slavelogs/messages");
			system("scp root@192.168.40.2:/home/root/tempNxtEvent.log /home/slavelogs/tempNxtEvent.log");
			return nToken;
		}
		case 251:
		{
			char string[50];
			int paramNum = atoi(szParam[1]);
			switch (paramNum)
			{
				case 7:
					strcpy(string, "GRI.HBDebug");
					break;
				case 8:
					strcpy(string, "GRI.HDDebug");
					break;
				case 17:
					strcpy(string, "MT9P001.Debug");
					break;
				default:
					return nToken;
			}
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 \"sed -i '/%s/d' /home/root/Eyelock.ini\"", string);
			system(szTokenBuffer);
			if (strcmp(szParam[2],"true")==0){
				len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 \"sed -i '$ a %s=true' /home/root/Eyelock.ini\"", string);
				system(szTokenBuffer);
			}
			return nToken;
		}
		case 310:
		{
			len = sprintf(szTokenBuffer, "rm /home/root/Eyelock.run");
		    break;
		}

/*
        NXTW_shell_exec("401"); //  /home/root/KeyMgr -d -i /home/firmware/fwHandler.tar.gz -o /home/firmware/fwHandlerExc.tar.gz
        NXTW_shell_exec("402"); // tar xvf /home/firmware/fwHandlerExc.tar.gz
        NXTW_shell_exec("405");  // chmod 777 /home/firmware/fwHandler/*
        NXTW_shell_exec("403");  // cd /home/firmware/fwHandler;/home/firmware/fwHandler/MultiChannelLogger -f"/home/updateFw1.log" -S
        NXTW_shell_exec("404");  // /home/firmware/fwHandler/fwHandler.sh upgrade
*/
		case 401:
		{
			len = sprintf(szTokenBuffer, " /home/root/KeyMgr -d -i /home/firmware/fwHandler.tar.gz -o /home/firmware/fwHandlerExc.tar.gz");
		    break;
		}
		case 402:
		{
			len = sprintf(szTokenBuffer, "cd /home/firmware/; tar xvf /home/firmware/fwHandlerExc.tar.gz");
		    break;
		}
		case 403:
		{
			len = sprintf(szTokenBuffer, "cd /home/firmware/fwHandler;/home/firmware/fwHandler/MultiChannelLogger -f'/home/updateFw1.log' -S");
		    break;
		}
		case 404:
		{
			// bash interpreter is required for fwHandler
			// originally this command called sh
			// on NXT sh symlinked to bash and it works fine
			// in Ubuntu sh symlinked to dash, and it causes errors with [[
			// when merging HBox and NXT branches, it's possible to call bash for both device types, or change a symlink on HBox
			len = sprintf(szTokenBuffer, "bash /home/firmware/fwHandler/fwHandler.sh upgrade");
		    break;
		}
		case 405:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/firmware/fwHandler/*");
		    break;
		}
		case 406:
		{
			len = sprintf(szTokenBuffer, "rm /home/osdps");
		    break;
		}

		case 510:
		{
			len = sprintf(szTokenBuffer, "chmod 777 /home/www/scripts/testExist.sh");
		    break;
		}
		case 511:
		{
			len = sprintf(szTokenBuffer, "/home/www/scripts/StartApplication.sh");
		    break;
		}

		case 997:
		{
			len = sprintf(szTokenBuffer, "/home/root/icm_communicator -r %s > /home/www/updateprogress", szParam[1]);
			system(szTokenBuffer);
			printf("===== bootloader: file name %s, command %s\n", szParam[1], szTokenBuffer);
			return nToken;
		}
		case 998:
		{
			len = sprintf(szTokenBuffer, "/home/root/icm_communicator -t %s -p %s > /home/www/updateprogress", szParam[1], szParam[2]);
			system(szTokenBuffer);
			return nToken;
		}
		case 999:
		{
			len = sprintf(szTokenBuffer, "/home/upgradeTemp/root/icm_communicator -t %s -p %s > /home/www/updateprogress", szParam[1], szParam[2]);
			system(szTokenBuffer);
			return nToken;
		}
		case 1330:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 %s", szParam[1]);
		    break;
		}
		case 1331:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cp /home/root/Eyelock.ini /home/EyelockBak.ini'");
		    break;
		}
		case 1332:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cd %s;/home/root/KeyMgr -d -i %s -o %s/out.tar.gz;mv %s/out.tar.gz %s'",szParam[1],szParam[2],szParam[3],szParam[4],szParam[5]);
		    break;
		}
		case 1333:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'cd %s;gzip -d %s;tar xf %s'",szParam[1],szParam[2],szParam[3]);
		    break;
		}
		case 1334:
		{
			system("chmod 777 /home/www/scripts/merge.sh");
			system("scp /home/www/scripts/merge.sh root@192.168.40.2:/home/root/merge.sh");
			system("ssh root@192.168.40.2 'cp /home/EyelockBak.ini /home/root/EyelockBak.ini'");
			system("ssh root@192.168.40.2 'chmod 777 /home/root/merge.sh'");
			system("ssh root@192.168.40.2 '/home/root/merge.sh'");
			return nToken;

		}
		case 1335:
		{
			len = sprintf(szTokenBuffer, "ssh root@192.168.40.2 'mkdir %s'",szParam[1]);
		    break;
		}
		case 1336:
		{
			len = sprintf(szTokenBuffer, "cat %s | ssh root@192.168.40.2 'cat > %s'",szParam[1],szParam[2]);
		    break;
		}
		case 1337:
		{
			system("ssh root@192.168.40.2 \"touch /home/UserLicense; killall -KILL Eyelock;\"");
			system("cd /home/root;sync;killall -KILL Eyelock;");
		    break;
		}
		case 1338:
		{
			//system("ssh root@192.168.40.2 'sync'");
			//system("sync");
		    break;
		}
		default:
			printf("Unknown token %d\n", nToken);
			len = sprintf(szTokenBuffer, "echo 'Unknown token %d\n'", nToken);
			break;
	}

	if (len) {
		sprintf(szTokenBuffer+len ,">/home/nxtwResult");
		system(szTokenBuffer);
	}
	if (m_debug)
		printf("%d Command: %s\n", nToken, szTokenBuffer);
	return nToken;
}
