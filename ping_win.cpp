#include <winsock2.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <stdio.h>
#include <string>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)

int main()
{
    const int COUNT          = 4;
	const int TIMEOUT_IN_SEC = 3;
	const int N              = 32;

	int minRTT = INT_MAX;
	int lost_packages_count, maxRTT, avgRTT;
	lost_packages_count = maxRTT = avgRTT = 0;

	char request[N], adress[N], host[N];

	WSAStartup(MAKEWORD(2, 2), new WSADATA);
	printf_s("Adress: "); scanf_s("%s", adress);
	hostent *remoteHost = gethostbyname(adress);
	if (!remoteHost) printf_s("Host have not found.\n");
	else
	{
		strcpy_s(host, inet_ntoa(*((in_addr *)remoteHost->h_addr)));
		HANDLE hIcmpFile = IcmpCreateFile();
		unsigned long destAddr = inet_addr(host);
		DWORD replySize = sizeof(ICMP_ECHO_REPLY) + sizeof(request);
		LPVOID replyBuf = new char[replySize];
		DWORD timeout = TIMEOUT_IN_SEC * 1000;

		printf_s("\nping %s (%s)...\n\n", adress, host);
		for (int i = 1; i <= COUNT; i++)
		{
			DWORD dwRetVal = IcmpSendEcho(hIcmpFile, destAddr, request, sizeof(request), NULL, replyBuf, replySize, timeout);
			if (!dwRetVal)
			{
				printf_s("%d: %s\n", i, std::string(TIMEOUT_IN_SEC, '*').c_str());
				lost_packages_count++;
			}
			else
			{
				PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)replyBuf;
				struct in_addr replyAddr;
				replyAddr.S_un.S_addr = pEchoReply->Address;
				ULONG RTT = pEchoReply->RoundTripTime;
				printf_s("%d: %d bytes, %d ms, TTL=%d.\n",
					i, pEchoReply->DataSize, RTT, pEchoReply->Options.Ttl);
				maxRTT = max(maxRTT, RTT);
				minRTT = min(minRTT, RTT);
				avgRTT += RTT;
			}
		}
		int received_packages_count = COUNT - lost_packages_count;
		printf_s("\nSUMMARY\nPackages sent=%d, received=%d, lost=%d.\n",
			COUNT, received_packages_count, lost_packages_count);
		if (received_packages_count)
			printf_s("Round-trip time: min=%dms, max=%dms, average=%dms\n",
				minRTT, maxRTT, avgRTT / received_packages_count);
		printf_s("\n");
	}
}
