#!/bin/bash

#############################################
#    AUTOMATE THE UPDATE & BUILD PROCESS    #
# - - - - - - - - - - - - - - - - - - - - - #
# 1) The script must be in the same         #
#     folder of the script to be updated;   #
# 3) The script name has this composition:  #
#     <execFileName>_update.sh            #
#############################################

### Setup

# Clear the terminal
clear

# Move in the current directory of the script
cd $PWD

# Automatically take the name of the file to launch
execFileName=$( echo $0 | cut -d'_' -f 1 )

echo "Name of the executable file: $execFileName"


### Phase 1: Save your credentials for a while 

# Get the credentials from the system
myCachedCredentials=$(echo -ne 'username=BlacknDecker\nprotocol=https\nhost=github.com\n' | git credential-cache get)


# If i don't have saved credentials, i take them
if [ -z "$myCachedCredentials" ]
then
   echo "Caching credentials for 120s!"
   git config --global credential.helper 'cache --timeout=120'
else
   echo "Credential found in the cache!"
fi

### Phase 2: Update the project, Build it and then Run!

# Update
echo "Updating..."
git pull
sleep 2s

# Build
echo "Building project..."
buildStatus=$( make )
echo "Done >> $buildStatus"
if [ "$?" -lt "2" ]
then
   echo "Running executable: ${execFileName}.exe"
   ./"${execFileName}.exe"   #Execute the builded file
else
   echo "Build Error!!!"
fi

# End
echo "--- Script END ---"


