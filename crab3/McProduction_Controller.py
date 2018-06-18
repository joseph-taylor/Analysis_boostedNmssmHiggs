import os
import sys

# 1. select the correct options in this script!!!
# 2. make sure you are in the Analysis_boostedNmssmHiggs/crab3 repository
# 3. run with $ python McProduction_Controller.py

# must use one fixed CMSSW version for the full chain. (Can use manual dataset entry to mitigate this)
# make sure you have a valid grid certificate and the correct (for your CMSSW environment) crab loaded.

# note that if a crab project is old the info disapears and thus you cannot get the publication status!!!
# ie can't get the files to run over in the next step!!! Need to enter them maually!!! Sorry:(
#################################################################
#################################################################
#################################################################
#################################################################
###### @ U S E R @ O P T I O N S @ ##############################


mode = 'submit'
# mode = 'resubmit'
# mode = 'checkStatus'

whichPartOfProcess = 'processMc01' # turns madgraph LHE into cmssw GENSIM
# whichPartOfProcess = 'processMc02' # step one of GENSIM into AOD
# whichPartOfProcess = 'processMc03' # step two of GENSIM into AOD
# whichPartOfProcess = 'processMc04' # turns AOD in MINIAOD
# whichPartOfProcess = 'patTupleAddBTag' # turns AOD into patTuple form (no longer part of the full workflow)


#-----------------------------------------------
##### INFO constant workflow INFO ##############
madGraphProjects = [
						 'mH50p0_mSusy800p0_ratio0p99_splitting0p1_600000events',
						 'mH50p0_mSusy1200p0_ratio0p99_splitting0p1_600001events',
						 'mH50p0_mSusy1600p0_ratio0p99_splitting0p1_600000events',
						 'mH50p0_mSusy2000p0_ratio0p99_splitting0p1_600003events',
						 'mH50p0_mSusy2200p0_ratio0p99_splitting0p1_600000events',
						 'mH50p0_mSusy2400p0_ratio0p99_splitting0p1_600000events',
						 'mH50p0_mSusy2600p0_ratio0p99_splitting0p1_600001events',
					]

outputPrimaryDatasetIntro = 'nmssmSignalCascadeV05_13TeV2017'

storageSite = 'T2_UK_SGrid_RALPP'
processMc_cmsswVersion = 'CMSSW_9_4_0_patch1'
simulationYear = 2017
blacklist = ['T3_US_UMiss']
dryRun = False

# This switches default is False. The script finds the dataset to use for you.
# However if the crab project has expired this will no longer work.
# You need to set manuallySetData to True
# And fill in the list dataSetsToUse to correspond to madGraphProjects
manuallySetData = False
# manuallySetData = True
dataSetsToUse = [
				# 'DAS_DATASET',
				# 'DAS_DATASET',
				# 'etc',
                ]
#-----------------------------------------------

#-------------------------------------------
##### INFO 'processMc01' INFO ##############
editionNamePro01 = "ed94Xv1"

eventsPerJob = 450
totalNumberOfEvents = 600000 # -1 to select them all
localMadGraphProjectStore = '/storage/jt15104/madGraphProjects/nmssmCascadeAnalysis_v05/paramCard_type03/'
pathWithinMadgraphProject = 'Events/run_01/unweighted_events.lhe' # ensure that you have unzipped these files using python/unzipMadgraphLhe.py
#-------------------------------------------

#------------------------------------------- Note that running submission of processMc02
##### INFO 'processMc02' INFO ############## requires valid editionNamePro01
editionNamePro02 = "ed94Xv1"

filesPerJobPro02 = 3
totalNumberOfFilesPro02 = -1 # -1 to select them all
#-------------------------------------------

#------------------------------------------- Note that running submission of processMc03
##### INFO 'processMc03' INFO ############## requires valid editionNamePro02
editionNamePro03 = "ed94Xv1"

filesPerJobPro03 = 1
totalNumberOfFilesPro03 = -1 # -1 to select them all
#-------------------------------------------

#------------------------------------------- Note that running submission of processMc04
##### INFO 'processMc04' INFO ############## requires valid editionNamePro03
editionNamePro04 = "ed94Xv1"

filesPerJobPro04 = 2
totalNumberOfFilesPro04 = -1 # -1 to select them all
#-------------------------------------------

#------------------------------------------- Note that running submission of patTupleAddBTag
##### INFO 'patTupleAddBTag' INFO ########## requires valid editionNamePro03
editionNamePAT = "ed94Xv1"

filesPerJobPAT = 3
totalNumberOfFilesPAT = -1 # -1 to select them all
#-------------------------------------------


#################################################################
#################################################################
### ## # HELPFUL INFO # ## ###
#
# outputPrimaryDataset = outputPrimaryDatasetIntro + '_' + madGraphProjectsStripOffEvents[i]
# partOneUniqueName = outputPrimaryDatasetIntro + '_processMc01_' + editionNamePro01 + '_' + madGraphProjectsStripOffEvents[i]
# partTwoUniqueName = outputPrimaryDatasetIntro + '_processMc02_' + editionNamePro02 + '_' + madGraphProjectsStripOffEvents[i]
# partThreeUniqueName = outputPrimaryDatasetIntro + '_processMc03_' + editionNamePro03 + '_' + madGraphProjectsStripOffEvents[i]
# partFourUniqueName = outputPrimaryDatasetIntro + '_processMc04_' + editionNamePro04 + '_' + madGraphProjectsStripOffEvents[i]
# patTupleUniqueName = outputPrimaryDatasetIntro + '_patTupleAddBTag_' + editionNamePAT + '_' + madGraphProjectsStripOffEvents[i]
# (the unique names must be less than 100 characters)
#
# the public location of the ouput will go here:
# 1. /<outputPrimaryDataset>/taylor-<partOneUniqueName>-<randomHash>/USER
# 2. /<outputPrimaryDataset>/taylor-<partTwoUniqueName>-<randomHash>/USER
# 3. /<outputPrimaryDataset>/taylor-<partThreeUniqueName>-<randomHash>/USER
# 4. /<outputPrimaryDataset>/taylor-<partFourUniqueName>-<randomHash>/USER
# 5. /<outputPrimaryDataset>/taylor-<patTupleUniqueName>-<randomHash>/USER
#
# crab projects will go here:
# 1. ./crab_projects/crab_<partOneUniqueName>/
# 2. ./crab_projects/crab_<partTwoUniqueName>/
# 3. ./crab_projects/crab_<partThreeUniqueName>/
# 4. ./crab_projects/crab_<partFourUniqueName>/
# 5. ./crab_projects/crab_<patTupleUniqueName>/
#
# output ROOT files will go here:
# 1. /hdfs/dpm/phy.bris.ac.uk/home/cms/store/user/taylor/<outputPrimaryDataset>/<partOneUniqueName>/<dateStamp>/0000/
# 2. /hdfs/dpm/phy.bris.ac.uk/home/cms/store/user/taylor/<outputPrimaryDataset>/<partTwoUniqueName>/<dateStamp>/0000/
# 3. /hdfs/dpm/phy.bris.ac.uk/home/cms/store/user/taylor/<outputPrimaryDataset>/<partThreeUniqueName>/<dateStamp>/0000/
# 4. /hdfs/dpm/phy.bris.ac.uk/home/cms/store/user/taylor/<outputPrimaryDataset>/<partFourUniqueName>/<dateStamp>/0000/
# 5. /hdfs/dpm/phy.bris.ac.uk/home/cms/store/user/taylor/<outputPrimaryDataset>/<patTupleUniqueName>/<dateStamp>/0000/
#
# the names of the output ROOT files will be:
# 1. nmssmSignal_GENSIM_<jobNum>.root (as stated by the config file processMc01_mgLheToGenSim_cfg.py)
# 2. nmssmSignal_AODSIMstep1of2_<jobNum>.root (as stated by the config file processMc02_genSimToAOD_step1of2_cfg.py)
# 3. nmssmSignal_AODSIMstep2of2_<jobNum>.root (as stated by the config file processMc03_genSimToAOD_step1of2_cfg.py)
# 4. nmssmSignal_MINIAODSIM_<jobNum>.root (as stated by the config file processMc04_AODToMINIAOD_cfg.py)
# 5. bTagPatTuple_<jobNum>.root (as stated by the config file patTuple_addBTagging_cfg.py)
###############################

#################################################################
#################################################################
#################################################################
#################################################################
#################################################################

#################################################################
#################################################################
#################################################################
#################################################################
#################################################################

#################################################################
#################################################################
#################################################################
#################################################################
#################################################################

if (simulationYear != 2016 and simulationYear != 2017):
	print "You have not set a correct year of simulation"
	print "You should be using 2016 or 2017"
	print "You fool"
	sys.exit()

cmsswVersion = os.popen("echo $CMSSW_VERSION", "r").readline()
cmsswVersion = cmsswVersion.rstrip()
# check that you are using the correct cmssw version
if cmsswVersion != processMc_cmsswVersion:
	print "You are using " + cmsswVersion
	print "You should be using " + processMc_cmsswVersion
	print "You Fool"
	sys.exit()

cmsswBase = os.popen("echo $CMSSW_BASE", "r").readline()
cmsswBase = cmsswBase.rstrip()
# check that you submit from the correct directory
dirShouldBe = cmsswBase + "/src/Analysis/Analysis_boostedNmssmHiggs/crab3"
if os.getcwd() != dirShouldBe:
	print "We are not in the correct directory to run this script"
	print "Get yourself in $CMSSW_BASE/src/Analysis/Analysis_boostedNmssmHiggs/crab3"
	print "You Fool"
	sys.exit()

# strip events section off madGraph info
madGraphProjectsStripOffEvents = []
for madGraphProject in madGraphProjects:
	for c in range(len(madGraphProject)-1, 0, -1):
		if madGraphProject[c] == '_':
			madGraphProjectsStripOffEvents.append(madGraphProject[:c])
			break

# if one is using Manually set datasets check that dataSetsToUse is same length as madGraphProjects
if manuallySetData == True and whichPartOfProcess != "processMc01":
	print "* * * * * * * * * * * * * * * * * * * * * * * * * * *"
	print "WARNING: you are manually setting the datasets to use"
	print "* * * * * * * * * * * * * * * * * * * * * * * * * * *"
	print ""
	if len(dataSetsToUse) != len(madGraphProjects):
		print "The number of datasets specified does not match the number of projects to run"
		print "Sort this out you fool"	
		sys.exit()


#-----------------------------------------------------------#
#----------------------processMc01--------------------------#
#-----------------------------------------------------------#

if mode == 'submit' and whichPartOfProcess == 'processMc01':

	for i in range(0,len(madGraphProjects)):

		outputPrimaryDataset = outputPrimaryDatasetIntro + '_' + madGraphProjectsStripOffEvents[i]
		partOneUniqueName = outputPrimaryDatasetIntro + '_processMc01_' + editionNamePro01 + '_' + madGraphProjectsStripOffEvents[i]
		inputFiles = os.path.join(localMadGraphProjectStore,madGraphProjects[i],pathWithinMadgraphProject)
	
		# create the tempory crab config file to submit
		f = open("temp_crab3config_processMc01.py", 'w')
		f.write("from CRABClient.UserUtilities import config\n")
		f.write("config = config()\n")
		f.write("config.Data.unitsPerJob = %d\n" % eventsPerJob)
		f.write("config.Data.totalUnits = %d\n" % totalNumberOfEvents)
		f.write("config.Data.splitting = 'EventBased'\n")
		f.write("config.Data.inputDBS = 'global'\n")
		f.write("config.Data.publication = True\n")
		# f.write("config.Data.ignoreLocality = True\n") # To avoid problems with data being stored on a black_listed site
		f.write("config.Data.outputPrimaryDataset = '%s'\n" % outputPrimaryDataset)
		f.write("config.Data.outputDatasetTag = '%s'\n" % partOneUniqueName)
		f.write("config.General.requestName = '%s'\n" % partOneUniqueName)
		f.write("config.General.workArea = 'crab_projects'\n")
		f.write("config.General.transferOutputs = True\n")
		f.write("config.General.transferLogs    = True\n")
		f.write("config.JobType.pluginName = 'PrivateMC'\n")
		if (simulationYear == 2016):
			f.write("config.JobType.psetName = 'processMc01_mgLheToGenSim_cfg.py'\n")
		elif (simulationYear == 2017):
			f.write("config.JobType.psetName = 'processMc01_mgLheToGenSim_2017_cfg.py'\n")
		f.write("config.JobType.inputFiles = ['%s']\n" % inputFiles)
		f.write("config.Site.storageSite = '%s'\n" % storageSite)
		f.write("config.Site.blacklist = %s\n" % str(blacklist) )
		f.close()
		print ""
		# os.system("cat temp_crab3config_processMc01.py") # for testing
		if (dryRun == False): os.system("crab submit -c temp_crab3config_processMc01.py") # for the real deal
		if (dryRun == True): os.system("crab submit -c temp_crab3config_processMc01.py --dryrun") # does a dryRun test
		os.system("rm temp_crab3config_processMc01.py")
		print ""



if mode == 'checkStatus' and whichPartOfProcess == 'processMc01':
	print ""
	print "*** CHECKING STATUS FOR PROCESSMC01 ***"
	print "NB if no info printed the task is most likely not bootstrapped yet"
	for i in range(0,len(madGraphProjects)):
		print ""
		print ""
		partOneUniqueName = outputPrimaryDatasetIntro + '_processMc01_' + editionNamePro01 + '_' + madGraphProjectsStripOffEvents[i]
		statusLines = os.popen("crab status -d crab_projects/crab_%s" % partOneUniqueName, "r").readlines()
		print partOneUniqueName
		foundJobLine = False
		for line in statusLines:
			if foundJobLine == False and line[:12] == "Jobs status:":
				print line.rstrip()
				foundJobLine = True
				continue
			if foundJobLine == True:
				if line == "\n":
					break
				print line.rstrip()
		foundPubLine = False
		for line in statusLines:
			if foundPubLine == False and line[:19] == "Publication status:":
				print line.rstrip()
				foundPubLine = True
				continue
			if foundPubLine == True:
				if line == "\n":
					break
				print line.rstrip()
	print ""



if mode == 'resubmit' and whichPartOfProcess == 'processMc01':

	for i in range(0,len(madGraphProjects)):
		partOneUniqueName = outputPrimaryDatasetIntro + '_processMc01_' + editionNamePro01 + '_' + madGraphProjectsStripOffEvents[i]
		os.system("crab resubmit -d crab_projects/crab_%s" % partOneUniqueName)
		print ""


		
#-----------------------------------------------------------#
#----------------------processMc02--------------------------#
#-----------------------------------------------------------#

if mode == 'submit' and whichPartOfProcess == 'processMc02':

	for i in range(0,len(madGraphProjects)):
		partOneUniqueName = outputPrimaryDatasetIntro + '_processMc01_' + editionNamePro01 + '_' + madGraphProjectsStripOffEvents[i]
		partTwoUniqueName = outputPrimaryDatasetIntro + '_processMc02_' + editionNamePro02 + '_' + madGraphProjectsStripOffEvents[i]

		# get the name of the DAS input dataset name
		if manuallySetData == False:
			inputDataset = []
			statusLines = os.popen("crab status -d crab_projects/crab_%s" % partOneUniqueName, "r").readlines()
			for line in statusLines:
				if line[:15] == "Output dataset:":
					inputDataset = line.rstrip()
					for c in range(15,len(inputDataset)):
						if inputDataset[c] == '/':
							inputDataset = inputDataset[c:]
							# print inputDataset # for debugging
							break
					break
		if manuallySetData == True:
			inputDataset = dataSetsToUse[i]

		# create the tempory crab config file to submit
		f = open("temp_crab3config_processMc02.py", 'w')
		f.write("from CRABClient.UserUtilities import config\n")
		f.write("config = config()\n")
		f.write("config.Data.inputDataset = '%s'\n" % inputDataset)
		f.write("config.Data.unitsPerJob = %d\n" % filesPerJobPro02)
		f.write("config.Data.totalUnits = %d\n" % totalNumberOfFilesPro02)
		f.write("config.Data.inputDBS = 'phys03'\n")
		f.write("config.Data.splitting = 'FileBased'\n")
		f.write("config.Data.publication = True\n")
		# f.write("config.Data.ignoreLocality = True\n") # To avoid problems with data being stored on a black_listed site
		f.write("config.Data.outputDatasetTag = '%s'\n" % partTwoUniqueName)
		f.write("config.General.requestName = '%s'\n" % partTwoUniqueName)
		f.write("config.General.workArea = 'crab_projects'\n")
		f.write("config.General.transferOutputs = True\n")
		f.write("config.General.transferLogs = True\n")
		f.write("config.JobType.pluginName = 'Analysis'\n")
		f.write("config.JobType.maxMemoryMB = 2500\n")
		f.write("config.JobType.maxJobRuntimeMin = 500\n")
		if (simulationYear == 2016):
			f.write("config.JobType.psetName = 'processMc02_genSimToAOD_step1of2_cfg.py'\n")
		elif (simulationYear == 2017):
			f.write("config.JobType.psetName = 'processMc02_genSimToAOD_step1of2_2017_cfg.py'\n")
		f.write("config.Site.storageSite = '%s'\n" % storageSite)
		f.write("config.Site.blacklist = %s\n" % str(blacklist) )
		f.close()
		print ""
		# os.system("cat temp_crab3config_processMc02.py") # for testing
		if (dryRun == False): os.system("crab submit -c temp_crab3config_processMc02.py") # for the real deal
		if (dryRun == True): os.system("crab submit -c temp_crab3config_processMc02.py --dryrun") # does a dryRun test
		os.system("rm temp_crab3config_processMc02.py")
		print ""



if mode == 'checkStatus' and whichPartOfProcess == 'processMc02':
	print ""
	print "*** CHECKING STATUS FOR PROCESSMC02 ***"
	print "NB if no info printed the task is most likely not bootstrapped yet"
	for i in range(0,len(madGraphProjects)):
		print ""
		print ""
		partTwoUniqueName = outputPrimaryDatasetIntro + '_processMc02_' + editionNamePro02 + '_' + madGraphProjectsStripOffEvents[i]
		statusLines = os.popen("crab status -d crab_projects/crab_%s" % partTwoUniqueName, "r").readlines()
		print partTwoUniqueName
		foundJobLine = False
		for line in statusLines:
			if foundJobLine == False and line[:12] == "Jobs status:":
				print line.rstrip()
				foundJobLine = True
				continue
			if foundJobLine == True:
				if line == "\n":
					break
				print line.rstrip()
		foundPubLine = False
		for line in statusLines:
			if foundPubLine == False and line[:19] == "Publication status:":
				print line.rstrip()
				foundPubLine = True
				continue
			if foundPubLine == True:
				if line == "\n":
					break
				print line.rstrip()
	print ""



if mode == 'resubmit' and whichPartOfProcess == 'processMc02':

	for i in range(0,len(madGraphProjects)):
		partTwoUniqueName = outputPrimaryDatasetIntro + '_processMc02_' + editionNamePro02 + '_' + madGraphProjectsStripOffEvents[i]
		os.system("crab resubmit -d crab_projects/crab_%s" % partTwoUniqueName)
		print ""


		
#-----------------------------------------------------------#
#----------------------processMc03--------------------------#
#-----------------------------------------------------------#

if mode == 'submit' and whichPartOfProcess == 'processMc03':

	for i in range(0,len(madGraphProjects)):
		partTwoUniqueName = outputPrimaryDatasetIntro + '_processMc02_' + editionNamePro02 + '_' + madGraphProjectsStripOffEvents[i]
		partThreeUniqueName = outputPrimaryDatasetIntro + '_processMc03_' + editionNamePro03 + '_' + madGraphProjectsStripOffEvents[i]

		# get the name of the DAS input dataset name
		if manuallySetData == False:
			inputDataset = []
			statusLines = os.popen("crab status -d crab_projects/crab_%s" % partTwoUniqueName, "r").readlines()
			for line in statusLines:
				if line[:15] == "Output dataset:":
					inputDataset = line.rstrip()
					for c in range(15,len(inputDataset)):
						if inputDataset[c] == '/':
							inputDataset = inputDataset[c:]
							# print inputDataset # for debugging
							break
					break
		if manuallySetData == True:
			inputDataset = dataSetsToUse[i]

		# create the tempory crab config file to submit
		f = open("temp_crab3config_processMc03.py", 'w')
		f.write("from CRABClient.UserUtilities import config\n")
		f.write("config = config()\n")
		f.write("config.Data.inputDataset = '%s'\n" % inputDataset)
		f.write("config.Data.unitsPerJob = %d\n" % filesPerJobPro03)
		f.write("config.Data.totalUnits = %d\n" % totalNumberOfFilesPro03)
		f.write("config.Data.inputDBS = 'phys03'\n")
		f.write("config.Data.splitting = 'FileBased'\n")
		f.write("config.Data.publication = True\n")
		# f.write("config.Data.ignoreLocality = True\n") # To avoid problems with data being stored on a black_listed site
		f.write("config.Data.outputDatasetTag = '%s'\n" % partThreeUniqueName)
		f.write("config.General.requestName = '%s'\n" % partThreeUniqueName)
		f.write("config.General.workArea = 'crab_projects'\n")
		f.write("config.General.transferOutputs = True\n")
		f.write("config.General.transferLogs = True\n")
		f.write("config.JobType.pluginName = 'Analysis'\n")
		f.write("config.JobType.maxMemoryMB = 2500\n")
		f.write("config.JobType.maxJobRuntimeMin = 500\n")
		if (simulationYear == 2016):
			f.write("config.JobType.psetName = 'processMc03_genSimToAOD_step2of2_cfg.py'\n")
		elif (simulationYear == 2017):
			f.write("config.JobType.psetName = 'processMc03_genSimToAOD_step2of2_2017_cfg.py'\n")
		f.write("config.Site.storageSite = '%s'\n" % storageSite)
		f.write("config.Site.blacklist = %s\n" % str(blacklist) )
		f.close()
		print ""
		# os.system("cat temp_crab3config_processMc03.py") # for testing
		if (dryRun == False): os.system("crab submit -c temp_crab3config_processMc03.py") # for the real deal
		if (dryRun == True): os.system("crab submit -c temp_crab3config_processMc03.py --dryrun") # does a dryRun test
		os.system("rm temp_crab3config_processMc03.py")
		print ""



if mode == 'checkStatus' and whichPartOfProcess == 'processMc03':
	print ""
	print "*** CHECKING STATUS FOR PROCESSMC03 ***"
	print "NB if no info printed the task is most likely not bootstrapped yet"
	for i in range(0,len(madGraphProjects)):
		print ""
		print ""
		partThreeUniqueName = outputPrimaryDatasetIntro + '_processMc03_' + editionNamePro03 + '_' + madGraphProjectsStripOffEvents[i]
		statusLines = os.popen("crab status -d crab_projects/crab_%s" % partThreeUniqueName, "r").readlines()
		print partThreeUniqueName
		foundJobLine = False
		for line in statusLines:
			if foundJobLine == False and line[:12] == "Jobs status:":
				print line.rstrip()
				foundJobLine = True
				continue
			if foundJobLine == True:
				if line == "\n":
					break
				print line.rstrip()
		foundPubLine = False
		for line in statusLines:
			if foundPubLine == False and line[:19] == "Publication status:":
				print line.rstrip()
				foundPubLine = True
				continue
			if foundPubLine == True:
				if line == "\n":
					break
				print line.rstrip()
	print ""



if mode == 'resubmit' and whichPartOfProcess == 'processMc03':

	for i in range(0,len(madGraphProjects)):
		partThreeUniqueName = outputPrimaryDatasetIntro + '_processMc03_' + editionNamePro03 + '_' + madGraphProjectsStripOffEvents[i]
		os.system("crab resubmit -d crab_projects/crab_%s" % partThreeUniqueName)
		print ""



#-----------------------------------------------------------#
#----------------------processMc04--------------------------#
#-----------------------------------------------------------#

if mode == 'submit' and whichPartOfProcess == 'processMc04':

	for i in range(0,len(madGraphProjects)):
		partThreeUniqueName = outputPrimaryDatasetIntro + '_processMc03_' + editionNamePro03 + '_' + madGraphProjectsStripOffEvents[i]
		partFourUniqueName = outputPrimaryDatasetIntro + '_processMc04_' + editionNamePro04 + '_' + madGraphProjectsStripOffEvents[i]

		# get the name of the DAS input dataset name
		if manuallySetData == False:
			inputDataset = []
			statusLines = os.popen("crab status -d crab_projects/crab_%s" % partThreeUniqueName, "r").readlines()
			for line in statusLines:
				if line[:15] == "Output dataset:":
					inputDataset = line.rstrip()
					for c in range(15,len(inputDataset)):
						if inputDataset[c] == '/':
							inputDataset = inputDataset[c:]
							# print inputDataset # for debugging
							break
					break
		if manuallySetData == True:
			inputDataset = dataSetsToUse[i]

		# create the tempory crab config file to submit
		f = open("temp_crab3config_processMc04.py", 'w')
		f.write("from CRABClient.UserUtilities import config\n")
		f.write("config = config()\n")
		f.write("config.Data.inputDataset = '%s'\n" % inputDataset)
		f.write("config.Data.unitsPerJob = %d\n" % filesPerJobPro04)
		f.write("config.Data.totalUnits = %d\n" % totalNumberOfFilesPro04)
		f.write("config.Data.inputDBS = 'phys03'\n")
		f.write("config.Data.splitting = 'FileBased'\n")
		f.write("config.Data.publication = True\n")
		# f.write("config.Data.ignoreLocality = True\n") # To avoid problems with data being stored on a black_listed site		
		f.write("config.Data.outputDatasetTag = '%s'\n" % partFourUniqueName)
		f.write("config.General.requestName = '%s'\n" % partFourUniqueName)
		f.write("config.General.workArea = 'crab_projects'\n")
		f.write("config.General.transferOutputs = True\n")
		f.write("config.General.transferLogs = True\n")
		f.write("config.JobType.pluginName = 'Analysis'\n")
		f.write("config.JobType.maxJobRuntimeMin = 500\n")
		if (simulationYear == 2016):
			f.write("config.JobType.psetName = 'processMc04_AODToMINIAOD_cfg.py'\n")
		elif (simulationYear == 2017):
			f.write("config.JobType.psetName = 'processMc04_AODToMINIAOD_2017_cfg.py'\n")
		f.write("config.Site.storageSite = '%s'\n" % storageSite)
		f.write("config.Site.blacklist = %s\n" % str(blacklist) )
		f.close()
		print ""
		# os.system("cat temp_crab3config_processMc04.py") # for testing
		if (dryRun == False): os.system("crab submit -c temp_crab3config_processMc04.py") # for the real deal
		if (dryRun == True): os.system("crab submit -c temp_crab3config_processMc04.py --dryrun") # does a dryRun test
		os.system("rm temp_crab3config_processMc04.py")
		print ""



if mode == 'checkStatus' and whichPartOfProcess == 'processMc04':
	print ""
	print "*** CHECKING STATUS FOR PROCESSMC04 ***"
	print "NB if no info printed the task is most likely not bootstrapped yet"
	for i in range(0,len(madGraphProjects)):
		print ""
		print ""
		partFourUniqueName = outputPrimaryDatasetIntro + '_processMc04_' + editionNamePro04 + '_' + madGraphProjectsStripOffEvents[i]
		statusLines = os.popen("crab status -d crab_projects/crab_%s" % partFourUniqueName, "r").readlines()
		print partFourUniqueName
		foundJobLine = False
		for line in statusLines:
			if foundJobLine == False and line[:12] == "Jobs status:":
				print line.rstrip()
				foundJobLine = True
				continue
			if foundJobLine == True:
				if line == "\n":
					break
				print line.rstrip()
		foundPubLine = False
		for line in statusLines:
			if foundPubLine == False and line[:19] == "Publication status:":
				print line.rstrip()
				foundPubLine = True
				continue
			if foundPubLine == True:
				if line == "\n":
					break
				print line.rstrip()
	print ""



if mode == 'resubmit' and whichPartOfProcess == 'processMc04':

	for i in range(0,len(madGraphProjects)):
		partFourUniqueName = outputPrimaryDatasetIntro + '_processMc04_' + editionNamePro04 + '_' + madGraphProjectsStripOffEvents[i]
		os.system("crab resubmit -d crab_projects/crab_%s" % partFourUniqueName)
		print ""



#-----------------------------------------------------------#
#----------------------patTupleAddBTag----------------------#
#-----------------------------------------------------------#

if mode == 'submit' and whichPartOfProcess == 'patTupleAddBTag':

	for i in range(0,len(madGraphProjects)):
		partThreeUniqueName = outputPrimaryDatasetIntro + '_processMc03_' + editionNamePro03 + '_' + madGraphProjectsStripOffEvents[i]
		patTupleUniqueName = outputPrimaryDatasetIntro + '_patTupleAddBTag_' + editionNamePAT + '_' + madGraphProjectsStripOffEvents[i]

		# get the name of the DAS input dataset name
		if manuallySetData == False:
			inputDataset = []
			statusLines = os.popen("crab status -d crab_projects/crab_%s" % partThreeUniqueName, "r").readlines()
			for line in statusLines:
				if line[:15] == "Output dataset:":
					inputDataset = line.rstrip()
					for c in range(15,len(inputDataset)):
						if inputDataset[c] == '/':
							inputDataset = inputDataset[c:]
							# print inputDataset # for debugging
							break
					break
		if manuallySetData == True:
			inputDataset = dataSetsToUse[i]

		if (simulationYear != 2016):
			print "Only have a PatTuple config for 2016, exiting..."
			sys.exit()

		# create the tempory crab config file to submit
		f = open("temp_crab3config_patTuple.py", 'w')
		f.write("from CRABClient.UserUtilities import config\n")
		f.write("config = config()\n")
		f.write("config.Data.inputDataset = '%s'\n" % inputDataset)
		f.write("config.Data.unitsPerJob = %d\n" % filesPerJobPAT)
		f.write("config.Data.totalUnits = %d\n" % totalNumberOfFilesPAT)
		f.write("config.Data.inputDBS = 'phys03'\n")
		f.write("config.Data.splitting = 'FileBased'\n")
		f.write("config.Data.publication = False\n")
		# f.write("config.Data.ignoreLocality = True\n") # To avoid problems with data being stored on a black_listed site
		f.write("config.Data.outputDatasetTag = '%s'\n" % patTupleUniqueName)
		f.write("config.General.requestName = '%s'\n" % patTupleUniqueName)
		f.write("config.General.workArea = 'crab_projects'\n")
		f.write("config.General.transferOutputs = True\n")
		f.write("config.General.transferLogs = True\n")
		f.write("config.JobType.pluginName = 'Analysis'\n")
		f.write("config.JobType.psetName = 'patTuple_addBTagging_cfg.py'\n")
		f.write("config.Site.storageSite = '%s'\n" % storageSite)
		f.write("config.Site.blacklist = %s\n" % str(blacklist) )
		f.close()
		print ""
		# os.system("cat temp_crab3config_patTuple.py") # for testing
		if (dryRun == False): os.system("crab submit -c temp_crab3config_patTuple.py") # for the real deal
		if (dryRun == True): os.system("crab submit -c temp_crab3config_patTuple.py --dryrun") # does a dryRun test
		os.system("rm temp_crab3config_patTuple.py")
		print ""



if mode == 'checkStatus' and whichPartOfProcess == 'patTupleAddBTag':
	print ""
	print "*** CHECKING STATUS FOR PAT.TUPLE.ADDBTAG ***"
	print "NB if no info printed the task is most likely not bootstrapped yet"
	for i in range(0,len(madGraphProjects)):
		print ""
		print ""
		patTupleUniqueName = outputPrimaryDatasetIntro + '_patTupleAddBTag_' + editionNamePAT + '_' + madGraphProjectsStripOffEvents[i]
		statusLines = os.popen("crab status -d crab_projects/crab_%s" % patTupleUniqueName, "r").readlines()
		print patTupleUniqueName
		foundJobLine = False
		for line in statusLines:
			if foundJobLine == False and line[:12] == "Jobs status:":
				print line.rstrip()
				foundJobLine = True
				continue
			if foundJobLine == True:
				if line == "\n":
					break
				print line.rstrip()
		foundPubLine = False
		for line in statusLines:
			if foundPubLine == False and line[:19] == "Publication status:":
				print line.rstrip()
				foundPubLine = True
				continue
			if foundPubLine == True:
				if line == "\n":
					break
				print line.rstrip()
	print ""



if mode == 'resubmit' and whichPartOfProcess == 'patTupleAddBTag':

	for i in range(0,len(madGraphProjects)):
		patTupleUniqueName = outputPrimaryDatasetIntro + '_patTupleAddBTag_' + editionNamePAT + '_' + madGraphProjectsStripOffEvents[i]
		os.system("crab resubmit -d crab_projects/crab_%s" % patTupleUniqueName)
		print ""

#################################################################
#################################################################
#################################################################
#################################################################
#################################################################
