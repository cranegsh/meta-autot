# !/bin/bash
# copy file or folder remotely to crane@192.168.0.77

# copy the file or folder specified by the first parameter
# copy to the folder specified by the second parameter in /home/crane/deploy/
# the password is 2024
#scp -r $1 crane@192.168.0.77:/home/crane/deploy/$2
#pscp -pw 2024 $1 crane@192.168.0.77:/home/crane/deploy/$2

#remote_path=crane@192.168.0.10:/home/crane/project
remote_path=crane@192.168.0.77:/home/crane/deploy
password=2024
scp -r $1 $remote_path/$2
#pscp -pw $password $1 $remote_path/$2/

