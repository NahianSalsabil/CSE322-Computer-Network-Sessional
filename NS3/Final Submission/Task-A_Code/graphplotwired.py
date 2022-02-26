import string
import matplotlib.pyplot as plt

barWidth = 7
input_file1 = "wired_nodes.txt"
input_file2 = "wired_flow.txt"
input_file3 = "wired_packets.txt"
x = []
y = []
z = []
w = []
v = []

with open(input_file1) as fp: 
    for line in fp:
        arr = line.strip().split()
        x.append(float(arr[0]))
        y.append(float(arr[1])/1000)
        name = arr[2]
        z.append(float(name[:-2]))
        w.append(float(arr[3]))
        v.append(float(arr[4]))

plt.bar(x, y, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('No of Nodes')
plt.ylabel('Throughput (Kbps)') 
plt.title('Network Throughput vs Node')
plt.show()
 
plt.bar(x, z, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('No of Nodes')
plt.ylabel('End to End Delay (ns)') 
plt.title('End to End Delay vs Node')
plt.show()

plt.bar(x, w, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('No of Nodes')
plt.ylabel('Packet Delivery Ratio (%)') 
plt.title('Packet Delivery Ratio vs Node')
plt.show()

plt.bar(x, v, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('No of Nodes')
plt.ylabel('Packet Drop Ratio') 
plt.title('Packet Drop Ratio vs Node')
plt.show()

# #Flows
barWidth = 3.5
x = []
y = []
z = []
w = []
v = []

with open(input_file2) as fp: 
    for line in fp:
        arr = line.strip().split()
        x.append(float(arr[0]))
        y.append(float(arr[1])/1000)
        name = arr[2]
        z.append(float(name[:-2]))
        w.append(float(arr[3]))
        v.append(float(arr[4]))

plt.bar(x, y, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('No of Flows')
plt.ylabel('Throughput (Kbps)') 
plt.title('Network Throughput vs Flows')
plt.show()

plt.bar(x, z, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('No of Flows')
plt.ylabel('End to End Delay (ns)') 
plt.title('End to End Delay vs Flows')
plt.show()

plt.bar(x, w, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('No of Flows')
plt.ylabel('Packet Delivery Ratio (%)') 
plt.title('Packet Delivery Ratio vs Flows')
plt.show()


plt.bar(x, v, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('No of Flows')
plt.ylabel('Packet Drop Ratio') 
plt.title('Packet Drop Ratio vs Flows')
plt.show()

# Packet Per Second
x = []
y = []
z = []
w = []
v = []

with open(input_file3) as fp: 
    for line in fp:
        arr = line.strip().split()
        x.append(float(arr[0]))
        y.append(float(arr[1])/1000)
        name = arr[2]
        z.append(float(name[:-2]))
        w.append(float(arr[3]))
        v.append(float(arr[4]))
        
barWidth = 35
        

plt.bar(x, y, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('Packet Per Second')
plt.ylabel('Throughput (Kbps)') 
plt.title('Network Throughput vs Packet Per Second')
plt.show()

plt.bar(x, z, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('Packet Per Second')
plt.ylabel('End to End Delay (ns)') 
plt.title('End to End Delay vs Packet Per Second')
plt.show()

plt.bar(x, w, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('Packet Per Second')
plt.ylabel('Packet Delivery Ratio (%)') 
plt.title('Packet Delivery Ratio vs Packet Per Second')
plt.show()


plt.bar(x, v, color ='grey', width = barWidth, edgecolor ='grey')
plt.xlabel('Packet Per Second')
plt.ylabel('Packet Drop Ratio') 
plt.title('Packet Drop Ratio vs Packet Per Second')
plt.show()
