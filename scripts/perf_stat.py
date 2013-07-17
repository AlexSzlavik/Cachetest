#Compute statistics of the performance counter data
#We are interested in the percentage differnce of each memory level access

import sys

if(len(sys.argv) != 3):
    print('Error')
    exit(1)

resdir = sys.argv[1]
cpuid = sys.argv[2]

#Opening data
l1file = open(resdir + "/l1_hits_" + cpuid + ".dat","r")
l2file = open(resdir + "/l2_hits_" + cpuid + ".dat","r")
l3file = open(resdir + "/l3_hits_" + cpuid + ".dat","r")
lramfile = open(resdir + "/dram_hits_" + cpuid + ".dat","r")

l1data=[]
l2data=[]
l3data=[]
lramdata=[]
alldata=[]

#Read out all the data and split it
for i in l1file.readlines():
    l1data.append(i.split())

alldata.append(l1data)
for i in l2file.readlines():
    l2data.append(i.split())

alldata.append(l2data)
for i in l3file.readlines():
    l3data.append(i.split())

alldata.append(l3data)
for i in lramfile.readlines():
    lramdata.append(i.split())

alldata.append(lramdata)

#Closing out
l1file.close()
l2file.close()
l3file.close()
lramfile.close()

fout=open(resdir + "/percentages_" + cpuid + ".dat","w")

#Calculate our stuff
tot=[]
for i,g in enumerate(l1data):    #Totals
    entry=[]
    entry.append(int(g[0]))
    
    total = float(l1data[i][1])+float(l2data[i][1])+float(l3data[i][1])+float(lramdata[i][1])

    l1calc = float(l1data[i][1])/total
    l2calc = float(l2data[i][1])/total
    l3calc = float(l3data[i][1])/total
    lramcalc = float(lramdata[i][1])/total

    entry.append(l1calc)
    entry.append(l2calc)
    entry.append(l3calc)
    entry.append(lramcalc)
    tot.append(entry)

    outstring="%d %.5f %.5f %.5f %.5f\n" % tuple(entry)
    fout.write(outstring)

#Write data
fout.close()
