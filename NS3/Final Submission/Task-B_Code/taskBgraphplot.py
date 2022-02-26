import string
import matplotlib.pyplot as plt

input_file1 = "wirelesshigh_fit_nodes.txt"
input_file2 = "wirelesshigh_cubic_nodes.txt"

x = []
y1 = []
z1 = []
w1 = []
v1 = []
y2 = []
z2 = []
w2 = []
v2 = []
barWidth = 10

with open(input_file1) as fp: 
    for line in fp:
        arr = line.strip().split()
        x.append(float(arr[0]))
        y1.append(float(arr[1])/1000)
        name = arr[2]
        z1.append(float(name[:-2]))
        w1.append(float(arr[3]))
        v1.append(float(arr[4]))
        
with open(input_file2) as fp: 
    for line in fp:
        arr = line.strip().split()
        y2.append(float(arr[1])/1000)
        name = arr[2]
        z2.append(float(name[:-2]))
        w2.append(float(arr[3]))
        v2.append(float(arr[4]))
        
x2 = [i + barWidth for i in x]
        

plt.bar(x, y1, color ='grey', width = barWidth, edgecolor ='grey', label ='Cubic Fit')
plt.bar(x2, y2, color ='pink', width = barWidth, edgecolor ='grey', label ='Cubic')
plt.xlabel('No of Nodes')
plt.ylabel('Throughput (Kbps)') 
plt.title('Network Throughput vs Node')
plt.legend()
plt.show()

plt.bar(x, z1, color ='grey', width = barWidth, edgecolor ='grey', label ='Cubic Fit')
plt.bar(x2, z2, color ='pink', width = barWidth, edgecolor ='grey', label ='Cubic')
plt.xlabel('No of Nodes')
plt.ylabel('End to End Delay (ns)') 
plt.title('End to End Delay vs Node')
plt.legend()
plt.show()
