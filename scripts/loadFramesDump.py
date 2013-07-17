#!/usr/bin/python

import sys
import struct
import collections
import operator
import os

class LoadFramesDump:
    def __init__(self,fileName,PAGE_SIZE,LLC_SIZE,ASSOSC,LINE_SIZE):
        try:
            f=open(fileName,"r")
            a=f.read()
            f.close()
        except:
            print "File open/reading error"
            sys.exit(1)

        b=[]
        self.hist=collections.defaultdict(int)
        LINE_SIZE/=float(1024)

        NUM_PAGES_PER_LLC = LLC_SIZE/PAGE_SIZE

        SET_SIZE = float(ASSOSC)*float(LINE_SIZE)

        NUM_SETS = LLC_SIZE/SET_SIZE

        TRIP_SIZE = float(LINE_SIZE) * NUM_SETS

        PAGES_PER_TRIP = TRIP_SIZE/PAGE_SIZE

        PAGES_PER_SET = SET_SIZE / PAGE_SIZE

        SETS_PER_PAGE = 1/PAGES_PER_SET

        print "LLC_SIZE: ",LLC_SIZE
        print "PAGE_SIZE: ",PAGE_SIZE
        print "LINE_SIZE:",LINE_SIZE
        print "NUM_PAGES_PER_LLC: ",NUM_PAGES_PER_LLC
        print "PAGES_PER_TRIP:",PAGES_PER_TRIP
        print "SET_SIZE:",SET_SIZE
        print "NUM_SETS",NUM_SETS
        print "TRIP_SIZE",TRIP_SIZE
        print "PAGES_PER_SET:",PAGES_PER_SET
        print "SETS_PER_PAGE:",SETS_PER_PAGE

        s=0
        while s < len(a):
            seg=struct.unpack("<I",a[s:s+4])[0]
            totalFrames=0
            s+=4
            self.hist=collections.defaultdict(int)
            for i in range(s,s+seg*4,4):
                s+=4
                b=struct.unpack("<I",a[i:i+4])[0]
                #print "%x" % b
                self.hist[b%NUM_PAGES_PER_LLC] += 1
                totalFrames+=1

            self.sortHist = sorted(self.hist.iteritems(),key=operator.itemgetter(1))
            self.overCommit=collections.defaultdict(int)

            #print self.sortHist

            PageOverCommit=0
            UnusedSlots=0
            PMISS=0
            for i in self.sortHist:
                self.overCommit[i[0]%int(PAGES_PER_TRIP)]+=i[1]
            for i in self.overCommit.iteritems():
                excess=max(i[1]-ASSOSC,0)

                pmiss=float(excess)/(float(excess)+ASSOSC)
                PMISS+=pmiss*(float(i[1])/totalFrames)

                PageOverCommit+=excess
                UnusedSlots+=max(ASSOSC-i[1],0)

            print "OverCommit: ",PageOverCommit

            print self.overCommit
            print "NumberOfUnusedSlots: ",UnusedSlots
            print "MAX-USED: ",min(totalFrames,NUM_PAGES_PER_LLC)-len(self.sortHist)
            print "PercentCover:",float(len(self.sortHist))/NUM_PAGES_PER_LLC
            print "PercentCoverMAX-USED: ",float(len(self.sortHist))/(min(NUM_PAGES_PER_LLC,totalFrames))
            print "ProbMiss:", PMISS
            print "--------"


if __name__=="__main__":
    if(len(sys.argv)==6):
        LoadFramesDump(sys.argv[1],int(sys.argv[2]),int(sys.argv[3]),int(sys.argv[4]),int(sys.argv[5]))
    else:
        print "Usage: ",sys.argv[0]," <filename> <page_size in kB> <LLC size in kb> <assosciativity> <line size in bytes>"
        sys.exit(1)
