from turtle import color
import matplotlib.pyplot as plt

i=0
input_file1 = "cubicfit_" + str(i) + ".cwnd"
input_file2 = "cubic_" + str(i) + ".cwnd"

x1 = []
y1 = []
x2 = []
y2 = []

with open(input_file1) as fp: 
    for line in fp:
        arr = line.strip().split()
        x1.append(float(arr[0]))
        y1.append(float(arr[2]))
        
with open(input_file2) as fp: 
    for line in fp:
        arr = line.strip().split()
        x2.append(float(arr[0]))
        y2.append(float(arr[2]))
	

plt.plot(x1, y1, color='grey', label='Cubic Fit') 
plt.plot(x2, y2, color='r', label='Cubic') 
plt.xlabel('Time (s)')
plt.ylabel('Congestion Window') 
plt.title('Congestion Window vs Time')
plt.legend()
plt.show()
