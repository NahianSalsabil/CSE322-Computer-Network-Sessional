from xml.etree import ElementTree as ET
import sys
et=ET.parse(sys.argv[1])

total_received_bits = 0
simulation_start = 0.0
simulation_stop = 0.0

for flow in et.findall("FlowStats/Flow"):

	total_received_bits += int(flow.get('rxBytes'))
	tx_start = float(flow.get('timeFirstRxPacket')[:-2])
	tx_stop  = float(flow.get("timeLastRxPacket")[:-2])

	if simulation_start == 0.0:
		simulation_start = tx_start
	
	elif tx_start < simulation_start:
		simulation_start = tx_start

	if tx_stop > simulation_stop:
		simulation_stop = tx_stop

print("Total Received bits = " + str(total_received_bits*8))
print("Receiving Packet Start time = " + str(simulation_start*1e-9))
print("Receiving Packet End time = " + str(simulation_stop*1e-9))

duration = (simulation_stop-simulation_start)*1e-9

print("Receiving Packet Duration time = " + str(duration))
print("Network Throughput = " + str(total_received_bits*8 / (duration*1e6)) + " Mbps")

