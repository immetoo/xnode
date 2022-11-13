#include <XnodeBaseNetwork.h>

const char pmNetworkChipBoot[] PROGMEM = "Init eth0";
const char pmNetworkChipError[] PROGMEM = "ERR: No chip";
const char pmNetworkLinkError[] PROGMEM = "ERR: No link";
const char pmNetworkStaticBoot[] PROGMEM = "Static IP";
const char pmNetworkDhcpBoot[] PROGMEM = "Request dhcp";
const char pmNetworkDhcpDone[] PROGMEM = "dhcp done";
const char pmNetworkDhcpError[] PROGMEM = "ERR: dhcp failed";
const char pmNetworkDnsLookup[] PROGMEM = "Lookup dns";
const char pmNetworkResultError[] PROGMEM = "# Invalid result:0x";
const char pmNetworkResultPing[] PROGMEM = "Server replied";

const char pmNetworkUsePrefix[] PROGMEM = "eth0_";
const char pmNetworkConfigInfo[] PROGMEM = "net_info";
const char pmNetworkConfigInfoEth0[] PROGMEM = "net_info_eth0";
const char pmNetworkConfigMac[] PROGMEM = "net_mac";
const char pmNetworkConfigIp[] PROGMEM = "net_ip";
const char pmNetworkConfigMask[] PROGMEM = "net_mask";
const char pmNetworkConfigGate[] PROGMEM = "net_gate";
const char pmNetworkConfigDns[] PROGMEM = "net_dns";
const char pmNetworkConfigKey[] PROGMEM = "net_key";
const char pmNetworkConfigId[] PROGMEM = "net_id";

const char pmNetworkServerHost[] PROGMEM = "net_srv_host";
const char pmNetworkServerIp[] PROGMEM = "net_srv_ip";
const char pmNetworkServerLookup[] PROGMEM = "net_srv_lup";

const char pmNetworkServer[] PROGMEM = NET_XNODE_HOST;
const char pmNetworkUrlPost[] PROGMEM = NET_URL_POST;
const char pmNetworkUrlParaPostType[] PROGMEM = NET_URL_PARA_POST_TYPE;
const char pmNetworkUrlParaReqCnt[] PROGMEM = NET_URL_PARA_REQ_CNT;
const char pmNetworkUrlParaNetId[] PROGMEM = NET_URL_PARA_NET_ID;
const char pmNetworkUrlParaNodeData[] PROGMEM = NET_URL_PARA_NODE_DATA;
const char pmNetworkUrlParaNodeNumber[] PROGMEM = NET_URL_PARA_NODE_NUMBER;

const char pmNetworkInfoTXOpen[] PROGMEM = "net_tx_open";
const char pmNetworkInfoTX[] PROGMEM = "net_tx";
const char pmNetworkInfoRX[] PROGMEM = "net_rx";

const char pmNetworkPing[] PROGMEM = "net_ping";

unsigned long net_ping_boot_timer = ZERO;
unsigned long net_ping_host_timer = ZERO;
unsigned long net_dhcp_retry_timer = ZERO;
unsigned long net_dns_lookup_timer = ZERO;
uint8_t net_status_eth = ZERO;
uint8_t net_status_ip = ZERO;
uint8_t net_status_dns = ZERO;
uint8_t net_status_data = ZERO;
uint8_t net_status_ping = ZERO;
uint8_t net_status_reply = ZERO;

uint8_t net_tx_open = ZERO;
unsigned long net_tx = ZERO;
unsigned long net_rx = ZERO;

char line_buf[128];
#define ETH_BUFFER_SIZE 450
byte Ethernet::buffer[ETH_BUFFER_SIZE];
Stash stash;
byte stashStart;
byte stashSession;
XnodeBaseNetwork XBNetwork;

void XnodeBaseNetwork::begin() {
	XBHardware.changeLed(SYS_LED_NETWORK, SYS_LED_STATUS_ON);
	setupChip();
	setupIp();
	XSerial.printCommentLineP(pmNetworkDnsLookup);
	XSerial.executeCommandP(pmNetworkServerLookup);
	XSerial.executeCommandP(pmNetworkConfigInfo);
	XSerial.executeCommandP(pmNetworkConfigInfoEth0);
	unsigned long time = millis();
	net_dhcp_retry_timer = time + (unsigned long) NET_DHCP_RETRY_TIME;
	net_dns_lookup_timer = time + (unsigned long) NET_DNS_LOOKUP_TIME;
	net_ping_boot_timer  = time + (unsigned long) NET_PING_BOOT_TIME;
	net_ping_host_timer  = time + (unsigned long) NET_PING_HOST_TIME;
	XBHardware.changeLed(SYS_LED_NETWORK, SYS_LED_STATUS_OFF);
}

void XnodeBaseNetwork::loop() {
	if (net_status_eth != ZERO) {
		return; // no chip
	}
	loopAutoSetup(); // auto dhcp and dns refresh
	if (net_status_ip != ZERO || net_status_dns != ZERO) {
		return; // no ip or dns
	}
	ether.packetLoop(ether.packetReceive());
	loopDataRX(); // rx first so that on tx-req packetLoop is runned first before RX code.
	loopDataTX();
	loopPingBoot();
	loopPingHost();
}

// prev version needed about ~32 requests before network is working, now it works direct but we keep ping.
void XnodeBaseNetwork::loopPingBoot() {
	if (net_status_ping == ONE) {
		return;
	}
	unsigned long time = millis();
	if (time < net_ping_boot_timer) {
		return; // wait
	}
	net_ping_boot_timer = time + (unsigned long) NET_PING_BOOT_TIME;
	if (net_status_reply == ZERO) {
		line_buf[ZERO] = ZERO;
		sendPing(); // keep sending ping requests.
		return;
	}
	if (net_status_ping == ZERO) {
		XSerial.print(CHAR_NEWLINE);// new line bacause the dot progress bar
		XSerial.printCommentLineP(pmNetworkResultPing); // let user know network is up.
		XSerial.printPromt();
		net_tx_open = ZERO; // reset open request counter
	}
	net_status_ping = ONE+ONE; // print once
	if (XBHardware.config.net_key.u8[ZERO] == ZERO) {
		sendInit(); // send init request.
		net_ping_boot_timer += (unsigned long) NET_PING_BOOT_TIME; // add extra wait time
		return;
	}
	net_status_ping = ONE; // never run this function again.
}

void XnodeBaseNetwork::loopPingHost() {
	if (millis() < net_ping_host_timer) {
		return; // wait
	}
	sendPing();
	net_ping_host_timer = millis() + (unsigned long) NET_PING_HOST_TIME;
}

void XnodeBaseNetwork::xxteaEncrypt(unsigned long v[2]) {
	unsigned int i;
	unsigned long v0=v[0], v1=v[1], sum=0, delta=0x9E3779B9;
	for (i=0; i < XXTEA_NUM_ROUNDS; i++) {
		v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + XBHardware.config.net_key.u32[sum & 3]);
		sum += delta;
		v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + XBHardware.config.net_key.u32[(sum>>11) & 3]);
	}
	v[0]=v0; v[1]=v1;
}

void XnodeBaseNetwork::xxteaDecrypt(unsigned long v[2]) {
	unsigned int i;
	uint32_t v0=v[0], v1=v[1], delta=0x9E3779B9, sum=delta*XXTEA_NUM_ROUNDS;
	for (i=0; i < XXTEA_NUM_ROUNDS; i++) {
		v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + XBHardware.config.net_key.u32[(sum>>11) & 3]);
		sum -= delta;
		v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + XBHardware.config.net_key.u32[sum & 3]);
	}
	v[0]=v0; v[1]=v1;
}

int XnodeBaseNetwork::parseReplyData(word off) {
	memset(line_buf, NULL, sizeof(line_buf));
	if (off != 0) {
		uint16_t pos = off;
		int line_pos = 0;
		
		// Skip over header until data part is found
		while (Ethernet::buffer[pos]) {
			if (Ethernet::buffer[pos - 1] == '\n' && Ethernet::buffer[pos] == '\r') {
				break;
			}
			pos++;
		}
		pos += 2;
		while (Ethernet::buffer[pos]) {
			if (line_pos < 49) {
				line_buf[line_pos] = Ethernet::buffer[pos];
				line_pos++;
			} else {
				break;
			}
			pos++;
		}
		line_buf[line_pos] = '\0';
		return line_pos;
	}
	return 0;
}

void XnodeBaseNetwork::sendStart(char postType) {
	if (stashSession != ZERO) {
		int maxWait = 100;
		while (stashSession != ZERO) {
			loop();
			maxWait--;
			if (maxWait == ZERO) {
				break;
			}
		} 
		
#ifdef DEBUG_NETWORK
		if (stashSession != ZERO) {
			Serial.println(F("#D kill prev tcp req"));
		}
#endif	
		//return; // data is already sending (TODO: move ??)
	}
	
	stashStart = stash.create();
	stash.print(XUtil.UNPSTR(pmNetworkUrlParaPostType));
	stash.print(postType);
	stash.print(XUtil.UNPSTR(pmNetworkUrlParaReqCnt));
	stash.print(net_tx++);
	stash.print(XUtil.UNPSTR(pmNetworkUrlParaNetId));
	for (byte i = ZERO; i < NET_BYTE_NET_ID_SIZE; ++i) {
		byte data = XBHardware.config.net_id[i];
		if (data<0xF) {
			stash.print(ZERO); // extra zero to have fixed length hex string
		}
		stash.print(data, HEX);
	}
}

void XnodeBaseNetwork::sendPing() {
	sendStart(NET_URL_PT_PING);
	net_status_data = NET_URL_PT_PING;
}

void XnodeBaseNetwork::sendInit() {
#ifdef DEBUG_NETWORK
	Serial.println(F("#D req init"));
#endif
	sendStart(NET_URL_PT_INIT);
	net_status_data = NET_URL_PT_INIT;
}

NetKey x_data;

void XnodeBaseNetwork::sendNodeData(byte senderId, char* data) {
	sendStart(NET_URL_PT_DATA);
	stash.print(XUtil.UNPSTR(pmNetworkUrlParaNodeNumber));
	stash.print((int) senderId);
	stash.print(XUtil.UNPSTR(pmNetworkUrlParaNodeData));

	char* dstring = data;
	while (*dstring != 0) {
		char value = *dstring;
		dstring++;
		stash.print(value);
	}
	
	/*
	byte i = 0;
	char* dstring = data;
	while (*dstring != 0) {
		x_data.u8[i] = *dstring;
		dstring++;
		i++;
		if (i==8) {
			i = 0;
			xxteaEncrypt(x_data.u32);
			for (byte i = 0; i < 8; ++i) {
				char value = x_data.u8[i];
				if (value == 0) {
					stash.print('00');
					continue;
				}
				if (value <= 0xF) {
					stash.print('0');
				}
				stash.print(value,HEX);
			}
		}
	}
	while(i < 8) {
		x_data.u8[i] = 0;
		i++;
	}
	xxteaEncrypt(x_data.u32);
	for (byte i = 0; i < 8; ++i) {
		char value = x_data.u8[i];
		if (value == 0) {
			stash.print('00');
			continue;
		}
		if (value <= 0xF) {
			stash.print('0');
		}
		stash.print(value,HEX);
	}
	*/
	net_status_data = NET_URL_PT_DATA;
}

void XnodeBaseNetwork::loopDataTX() {
	if (net_status_data == ZERO) {
		return; // no data to send
	}
	if (!ether.isLinkUp()) {
		if (net_status_ip == ZERO) {
			XSerial.printCommentLineP(pmNetworkLinkError);
		}
		net_status_ip = ONE;
		return; // link down; request new ip when link gets up.
	}
	
	XBHardware.changeLed(SYS_LED_NETWORK, SYS_LED_STATUS_ON);
	
#ifdef DEBUG_NETWORK
	Serial.print(F("#D sendHttp pt="));
	Serial.print((char)net_status_data);
	Serial.print(F(" size="));
	Serial.println((char)stash.size(),DEC);
#endif
	
	stash.save();
	Stash::prepare(PSTR("POST http://$F/$F HTTP/1.1" "\r\n"
			"Host: $F" "\r\n"
			"Cache-Control: no-cache" "\r\n"
			"Content-type: application/x-www-form-urlencoded;\r\n"
			"Accept: application/xhtml+xml" "\r\n"
			"Content-Length: $D" "\r\n"
			"\r\n"
			"$H"), pmNetworkServer, pmNetworkUrlPost, pmNetworkServer, stash.size(), stashStart);
	net_tx_open++;
	net_status_data = ZERO;
	stashSession = ether.tcpSend(); // releases also stash buf
	//stash.release();
}

void XnodeBaseNetwork::loopDataRX() {
	if (stashSession == ZERO) {
		return; // no reply to search for
	}
	const char* msgStart = ether.tcpReply(stashSession);
	if (msgStart == ZERO) {
#ifdef DEBUG_NETWORK
		Serial.println(F("#D wait tcpReply"));
#endif
		delay(100);
		return; // no reply yet
	}
	
	net_rx++;
	stashSession = ZERO;
	parseReplyData((int) *msgStart);
	handleServerResult();
	XBHardware.changeLed(SYS_LED_NETWORK, SYS_LED_STATUS_OFF);
}

void XnodeBaseNetwork::handleServerResult() {
	if (line_buf[ZERO] != NET_URL_RESULT_OK) {
		XSerial.printCharP(pmNetworkResultError);
		Serial.print(line_buf[ZERO], HEX); // TODO: check
#ifdef DEBUG_NETWORK
		Serial.print(CHAR_SPACE);
		for (int i=0;i<10;i++) {
			if (line_buf[i] != ZERO) {
				Serial.print(line_buf[i]);
			}
		}
#endif
		XSerial.print(CHAR_NEWLINE);
		return;
	}
	net_tx_open--; // valid result
	net_status_reply = ONE; // we have reply
	
	if (line_buf[ONE] == ZERO) {
		return; // Only X returns so nop.
	}
	
	if (line_buf[ONE] == NET_URL_PT_INIT) {
		XSystem.executeCommand(line_buf + ONE + ONE,true); // skip Xi chars
		loop();     // Else we miss most of the time the second init request.
		loop(); // an other extra to be sure we done miss the second request.
		sendInit(); // request next init command
		return;
	}
	if (line_buf[ONE] == NET_URL_RESULT_OK) {
		XSystem.executeCommand(line_buf + ONE + ONE,true); // skip XX chars
		loop();
		loop();
		sendNodeData(RF_BASE_NODE_ID,XSystem.replyBuffer); // send reply as
	}
}

bool XnodeBaseNetwork::isUp() {
	return ZERO == (net_status_eth + net_status_ip + net_status_dns);
}

void XnodeBaseNetwork::loopAutoSetup() {
	unsigned long time = millis();
	// auto dhcp
	if (net_status_ip == ONE) {
		if (time < net_dhcp_retry_timer) {
			return; // wait
		}
		net_dhcp_retry_timer = millis() + (unsigned long) NET_DHCP_RETRY_TIME;
		setupIp();
		if (net_status_ip == ONE) {
			return; // failed dhcp
		}
		net_status_dns = ONE;       // when link plugs in later
		net_dns_lookup_timer = ONE; // then do direct dns lookup
	}
	// auto dns
	if (net_status_dns == ONE) {
		if (time < net_dns_lookup_timer) {
			return; // wait
		}
		net_dns_lookup_timer = time + (unsigned long) NET_DNS_LOOKUP_TIME;
		XSerial.executeCommandP(pmNetworkServerLookup);
	}
	// auto dns refresh
	if (net_status_dns == ZERO && time > net_dns_lookup_timer) {
		net_status_dns = ONE;
	}
}

// FIXME: without chip it takes 3 watchdog reboots before no chip result ?? (prev lib version)
void XnodeBaseNetwork::setupChip() {
	XSerial.printCommentLineP(pmNetworkChipBoot);
	if (ether.begin(sizeof Ethernet::buffer, XBHardware.config.net_mac,	HW_PIN_SPI_ETH0_CS) == ZERO) {
		XSerial.printCommentLineP(pmNetworkChipError);
		XBHardware.changeLed(SYS_LED_ERROR, SYS_LED_STATUS_BLINK);
		net_status_eth = ONE;
		return;
	}
	ether.disableBroadcast();
	ether.disableMulticast();
	delay(100); // needed for link isLinkUp to return valid value.
}

void XnodeBaseNetwork::setupIp() {
	net_status_ip = ZERO;
	if (net_status_eth != ZERO) {
		return; // no chip
	}
	if (!ether.isLinkUp()) {
		net_status_ip = ONE; // do again
		XSerial.printCommentLineP(pmNetworkLinkError);
		return;
	}
	
	// Setup static if ip is given.
	if (XBHardware.config.net_ip[ZERO] != ZERO) {
		XSerial.printCommentLineP(pmNetworkStaticBoot);
		ether.staticSetup(XBHardware.config.net_ip, XBHardware.config.net_gate, XBHardware.config.net_dns);
		ether.copyIp(ether.netmask, XBHardware.config.net_mask);
		return;
	}
	XSerial.printCommentLineP(pmNetworkDhcpBoot);
	wdt_disable();
	if (!ether.dhcpSetup()) {
		net_status_ip = ONE; // do again
		XSerial.printCommentLineP(pmNetworkDhcpError);
		XBHardware.changeLed(SYS_LED_ERROR, SYS_LED_STATUS_ON); // TODO: fix led blinking on timer int.
	} else {
		XSerial.printCommentLineP(pmNetworkDhcpDone);
		XBHardware.changeLed(SYS_LED_ERROR, SYS_LED_STATUS_OFF); // set led off after recovery
	}
	wdt_enable(WDT_TIMEOUT); // no link or no dhcp will take long timeout ~30s
	
#ifdef DEBUG_NET_GATE
	if (net_status_ip == ZERO) {
		byte debug_gate[NET_BYTE_IP_SIZE] = DEBUG_NET_GATE; // transparent http proxy network config.
		ether.copyIp(ether.gwip, debug_gate);
		Serial.println(F("#D Used debug gate ip"));
	}
#endif
}

void XnodeBaseNetwork::cmdDnsLookup() {
	net_status_dns = ZERO;
	if (net_status_eth != ZERO) {
		return; // no chip
	}
	if (net_status_ip != ZERO) {
		net_status_dns = ONE;
		return; // no ip
	}
	
#ifdef DEBUG_NET_HISIP
	byte debug_hisip[NET_BYTE_IP_SIZE] = DEBUG_NET_HISIP;
	ether.copyIp(ether.hisip, debug_hisip);
	Serial.println(F("#D Used debug dns ip"));
	return;
#endif
	
	wdt_disable();
	if (!ether.dnsLookup(pmNetworkServer)) {
		net_status_dns = ONE;
		XSystem.buildReplyPValue(pmNetworkServerLookup,ZERO);
		XBHardware.changeLed(SYS_LED_ERROR, SYS_LED_STATUS_BLINK);
	} else {
		XSystem.buildReplyPValue(pmNetworkServerLookup,ONE);
		XBHardware.changeLed(SYS_LED_ERROR, SYS_LED_STATUS_OFF);
	}
	wdt_enable (WDT_TIMEOUT);
}

void XnodeBaseNetwork::cmdInfoEth0() {
	// Print currently used config
	XSystem.buildReplyCharP(pmNetworkUsePrefix);
	XSystem.buildReplyPValueIP(pmNetworkConfigIp, ether.myip);
	XSystem.buildReplyCharP(pmNetworkUsePrefix);
	XSystem.buildReplyPValueIP(pmNetworkConfigMask, ether.netmask);
	XSystem.buildReplyCharP(pmNetworkUsePrefix);
	XSystem.buildReplyPValueIP(pmNetworkConfigGate, ether.gwip);
	XSystem.buildReplyCharP(pmNetworkUsePrefix);
	XSystem.buildReplyPValueIP(pmNetworkConfigDns, ether.dnsip);
	XSystem.buildReplyCharP(pmNetworkUsePrefix);
	XSystem.buildReplyPValueByteA(pmNetworkConfigMac,ether.mymac, NET_BYTE_MAC_SIZE);
}

void XnodeBaseNetwork::cmdInfo() {
	// Print server value and lookup
	XSystem.buildReplyPValueP(pmNetworkServerHost,pmNetworkServer);
	XSystem.buildReplyPValueIP(pmNetworkServerIp, ether.hisip);
	
	// Print config
	XSystem.buildReplyPValueByteA(pmNetworkConfigMac,XBHardware.config.net_mac, NET_BYTE_MAC_SIZE);
	XSystem.buildReplyPValueIP(pmNetworkConfigIp, XBHardware.config.net_ip);
	XSystem.buildReplyPValueIP(pmNetworkConfigMask, XBHardware.config.net_mask);
	XSystem.buildReplyPValueIP(pmNetworkConfigGate, XBHardware.config.net_gate);
	XSystem.buildReplyPValueIP(pmNetworkConfigDns, XBHardware.config.net_dns);
	XSystem.buildReplyPValueByteA(pmNetworkConfigKey,XBHardware.config.net_key.u8, NET_BYTE_NET_KEY_SIZE);
	XSystem.buildReplyPValueByteA(pmNetworkConfigId,XBHardware.config.net_id, NET_BYTE_NET_ID_SIZE);
	
	XSystem.buildReplyPValue(pmNetworkInfoTXOpen, (int) net_tx_open);
	XSystem.buildReplyPValue(pmNetworkInfoTX, net_tx);
	XSystem.buildReplyPValue(pmNetworkInfoRX, net_rx);
}

void XnodeBaseNetwork::parseCommandIp(const char* pmName, char** args, byte* ip_conf) {
	ether.parseIp(ip_conf, args[ZERO]);
	XSystem.buildReplyPValueIP(pmName, ip_conf);
	XSystem.configSave(); // auto save on change
}

void XnodeBaseNetwork::systemModuleCommandList() {
	XSystem.buildReplyCommandListP(pmNetworkConfigInfo);
	XSystem.buildReplyCommandListP(pmNetworkConfigInfoEth0);
	XSystem.buildReplyCommandListP(pmNetworkConfigMac);
	XSystem.buildReplyCommandListP(pmNetworkConfigIp);
	XSystem.buildReplyCommandListP(pmNetworkConfigMask);
	XSystem.buildReplyCommandListP(pmNetworkConfigGate);
	XSystem.buildReplyCommandListP(pmNetworkConfigDns);
	XSystem.buildReplyCommandListP(pmNetworkPing);
	XSystem.buildReplyCommandListP(pmNetworkServerLookup);
#ifdef DEBUG_NETWORK
	XSystem.buildReplyCommandListP(pmNetworkConfigId);  // keep secret that these cmd work
	XSystem.buildReplyCommandListP(pmNetworkConfigKey); // for init over network with this.
#endif
}

bool XnodeBaseNetwork::systemModuleCommandExecute(char* cmd, char** args) {
	if (XUtil.strcmpP(cmd, pmNetworkConfigInfo) == ZERO) {
		cmdInfo();
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmNetworkConfigInfoEth0) == ZERO) {
		cmdInfoEth0();
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmNetworkConfigId) == ZERO) {
		if (XSystem.buildReplyCommandArgumentError(cmd, args)) {
			return true;
		}
		XUtil.charsToByteA(args[ZERO],XBHardware.config.net_id, NET_BYTE_NET_ID_SIZE);
		XSystem.buildReplyPValueByteA(pmNetworkConfigId,XBHardware.config.net_id,NET_BYTE_NET_ID_SIZE);
		XSystem.configSave();
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmNetworkConfigKey) == ZERO) {
		if (XSystem.buildReplyCommandArgumentError(cmd, args)) {
			return true;
		}
		XUtil.charsToByteA(args[ZERO],XBHardware.config.net_key.u8, NET_BYTE_NET_KEY_SIZE);
		XSystem.buildReplyPValueByteA(pmNetworkConfigKey,XBHardware.config.net_key.u8,NET_BYTE_NET_KEY_SIZE);
		XSystem.configSave();
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmNetworkConfigMac) == ZERO) {
		if (XSystem.buildReplyCommandArgumentError(cmd, args)) {
			return true;
		}
		XUtil.charsToByteA(args[ZERO],XBHardware.config.net_mac, NET_BYTE_MAC_SIZE);
		XSystem.buildReplyPValueByteA(pmNetworkConfigMac,XBHardware.config.net_mac, NET_BYTE_MAC_SIZE);
		XSystem.configSave();
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmNetworkConfigIp) == ZERO) {
		if (XSystem.buildReplyCommandArgumentError(cmd, args)) {
			return true;
		}
		parseCommandIp(pmNetworkConfigIp, args, XBHardware.config.net_ip);
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmNetworkConfigMask) == ZERO) {
		if (XSystem.buildReplyCommandArgumentError(cmd, args)) {
			return true;
		}
		parseCommandIp(pmNetworkConfigMask, args, XBHardware.config.net_mask);
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmNetworkConfigGate) == ZERO) {
		if (XSystem.buildReplyCommandArgumentError(cmd, args)) {
			return true;
		}
		parseCommandIp(pmNetworkConfigGate, args, XBHardware.config.net_gate);
		return true;
		
	} else if (XUtil.strcmpP(cmd, pmNetworkConfigDns) == ZERO) {
		if (XSystem.buildReplyCommandArgumentError(cmd, args)) {
			return true;
		}
		parseCommandIp(pmNetworkConfigDns, args, XBHardware.config.net_dns);
		return true;
	} else if (XUtil.strcmpP(cmd, pmNetworkPing) == ZERO) {
		sendPing();
		return true;
	} else if (XUtil.strcmpP(cmd, pmNetworkServerLookup) == ZERO) {
		cmdDnsLookup();
		return true;
	}
	return false;
}
