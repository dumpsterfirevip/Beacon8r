import os
# python 3X I believe.
# MIT license and all that. part of Beacon8r code. @Beacon8r and @dj_ir0ngruve on twitter.

# Apologies for potato code. Python's not my main language. Yes, this was developed on windows so separator slashes are wrong for linux. Ran out of time to test right now for cross platform.

#Basically this takes any big list of names in a file named "biglistGenAccessPoints.txt", big list get it? 
#and chunks through them and spits out a complete Arduino folder/file set you can use to program an ESP8266 
#It creates the new folders and files in: genDuinoFolder


#Change these variables before running
beaconpattern = r"FULLPATHTO\beacon8pattern.txt"  # Top part of each file 
listaccesspoints = r"FULLPATHTO\biglistGenAccessPoints.txt" # The list of access points you want to chunk out
genDuinoFolder = r"FULLPATHTO_FOLDER_YOU_WANT_RESULTS_IN" #Where you set the reformatted chunks to be created in.


spliton = 2980 #Number of beacons to advertise per ESP8266. 2980 is near upper limit of space on chip I think. 130k / 1980 = roughly 44 ish ESP8266 folder/.ino file sets. Note that Folder name and *.ino file name I think have to be the same.
#Note: 13-17 per ESP8266 should be used for stability of viewing on a phone's wifi list. 
#You can have around 4-5 total esp8266's near each other powered by portable usb batteries no big deal. 
#First year I put each esp8266 in a ziploc bag for a smaller project.. Kapton tape covering each unit is better.
#Heat can get up to 140F not sure what that is real temperature.

#Finally... You can broadcast Emoji's. Arduino ide is fine with emojis but I haven't been able to put emojis in list here and not have python crash.

read_dataBP = 'blank'
bcn8rPlaceName = "beacon8rCluster_"

if not os.path.exists(genDuinoFolder):
    os.makedirs(genDuinoFolder)

with open(beaconpattern) as f:
		read_dataBP = f.read()
		f.closed

print(read_dataBP)



def write_file ():		
	with open(genDuinoFolder + "\\"+ currWkngName+ "\\"+ currWkngName +".ino", "w") as cf:
			cf.write(fileInProgress+ "\r\n}\r\n")
			cf.closed		


counter =0
programs =0
first = 0

currWkngName = bcn8rPlaceName + str(programs)
fileInProgress = ""


with open(listaccesspoints) as bl:
		for line in bl:			
			counter += 1
			if( counter %spliton == 0 or first ==0):
				if(len(fileInProgress) >0):
					#write fileInProgress to disk.
					if not os.path.exists(genDuinoFolder + "\\"+ currWkngName):
						os.makedirs(genDuinoFolder + "\\"+ currWkngName)
					#writeFile...
					write_file()
				programs +=1
				first =1
				currWkngName = bcn8rPlaceName + str(programs)
				print(currWkngName +"  and "+ currWkngName + r".ino")
				# Create directory if doesn't exists
				if not os.path.exists(genDuinoFolder + "\\"+ currWkngName):
					os.makedirs(genDuinoFolder + "\\"+ currWkngName)
				#set stored first part of program
				fileInProgress = read_dataBP
			fileInProgress += '	snBcn("'+line[:-1] + '");\r' #removes \r\n at end of line then adds \r where needed. 

			
		bl.closed
		
		write_file()

		
print(counter) # How many things have you done?
print(programs) # Oh why, the humanity, oh why!!

#This is what I imagine python programmers reading my code to the end feel like: https://xkcd.com/1513/ 

