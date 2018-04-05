"""with open("output.txt","r") as f:
    with open("plot.txt","wb") as output: 
        for line in f:
            print line
            if  (!line.startswith('U')):
                output.write(line)"""
import matplotlib.pyplot as plt
a=[]
stin=[]
c=[]
x = [[0 for i in range(10)] for j in range(12)]
i=0
j=0
with open('output.txt',"r+") as f:
    new_f = f.readlines()
    f.seek(0)
    for line in new_f:
        if "ALGORITHM" not in line:
            if "U" not in line:
                f.write(line)
                
                a= line.split(',')
                
                if len(a)==3:
                    
                    c.append(a[2].strip().split('\n'))

                    
    f.truncate()
k=0

for i in range(0,12):
    for j in range(0,10):
        x[i][j]=int(float(c[k][0]))
        k+=1
U=0.05
U1=[0.05,0.15,0.25,0.35,0.45,0.55,0.65,0.75,0.85,0.95]
print x[0]
print U1
stin.append("10 tasks distributed in D[C,T]")
stin.append('25 tasks distributed in D[C,T]')
stin.append('10 tasks distributed in D[C+(T-C)/2,T]')
stin.append('25 tasks distributed in D[C+(T-C)/2,T]')
for i in range(0,4):
    plt.figure(i+1)
    plt.plot(U1,x[3*i],label='EDF')
    plt.plot(U1,x[3*i+1],label='RM')
    plt.plot(U1,x[3*i+2],label='DM')
    plt.xlabel('Utilization')
    plt.ylabel('% of tasks that are schedulable')
    plt.legend()
    plt.title(stin[i])
    plt.savefig('img'+str(i)+'.png')
    plt.show()
    plt.close()
