// General parameters
log_payloads = false;
log_local_metrics = false;
num_nodes = 3;

// Node 1
node1:{
	type = AP;
	CORNET_IP = 192.168.1.2;
	CRTS_IP = 10.0.0.2;
	CE = CE_AP;
	layers = PHY;
	freq_tx = 460e6;
	freq_rx = 440e6;
}

// Node 2
node2:{
	type = UE;
	CORNET_IP = 192.168.1.3;
	CRTS_IP = 10.0.0.3;
	CE = CE_UE;
	layers = PHY;
	traffic = constant;
	freq_tx = 440e6;
	freq_rx = 460e6;
}

// Node 3
node3:{
	type = interferfer;
	CORNET_IP = 192.168.1.4;
	power = 10;
	signal = CW;
	behavior = constant;
}
