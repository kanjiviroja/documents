import sys
import os

numArgs = len(sys.argv) 

if (numArgs < 8):
    print "Usage:python build.py FILENAME MFG_ID IMAGE_TYPE VERSION STRING TAG_ID TAG_FILE"
elif (numArgs > 8):
    print "Usage:python build.py FILENAME MFG_ID IMAGE_TYPE VERSION STRING TAG_ID TAG_FILE"
elif (numArgs == 8):
    # Test parameters
    CREATE = sys.argv[1]
    MFG_ID = sys.argv[2]
    TYPE = sys.argv[3]
    VERSION = sys.argv[4]
    STR = sys.argv[5]
    TAG_ID = sys.argv[6]
    TAG_FILE = sys.argv[7]
    TEST_STAND_EXE = 'D:\\TOOLS\\Ember\\EmberZNet5.3.1-GA\\em35x-ezsp\\tool\\image-builder\\image-builder-windows.exe'



command = '%s --create %s.ota --manuf-id %s --image-type %s --version %s --string %s --tag-id %s --tag-file %s' % (TEST_STAND_EXE,CREATE,MFG_ID,TYPE,VERSION,STR,TAG_ID,TAG_FILE)

#proc = subprocess.Popen(command)
proc = os.system(command)

if (proc == 0):
    print "============================Successfully ota file generate============================"
else:
    print "****************************Failed to generate OTA files******************************" 
    
    
    