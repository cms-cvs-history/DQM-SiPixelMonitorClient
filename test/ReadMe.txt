1. Description of Classes :
=============================

    SiPixelWebInterface (web interface  for the SiPixels, inherited from WebInterface)
    SiPixelQualityTester (defines and attaches QualityTests to the MEs. the
                         tests are defined in test/sipixel_qualitytest_config_digi.xml)
    SiPixelActionExecutor (performs various actions as requested by WebInterface)
   
    SiPixelWebClient [OBSOLETE] (web client for the SiPixel, inherited from DQMWbClient)


    TrackerMap & TmModule [not yet implemented] (creates the TrackerMap SVG file)



2. Auxiliary files in test directory
=====================================
  
  o tracker.dat                      : needed to create TrackerMap
    trackermap.txt                   : header of the SVG file to be created

  o sendCmdToApp.pl                  : scripts needed to start the xdaq.exe
    webPingXDAQ.pl 
      
  o WebLib.js                        : Java script for the Web Widjet

  o setup.sh                         : creates necessary xml and scripts needed
                                       for a given environment

  o .SiPixelWebClient.xml            : used by setup.sh to create the real 
    .profile.xml                       SiPixelWebClient.xml profile.xml 
  
  o  .startMonitorClient             : used by setup.sh to create start script 
                                       startMonitorClient

  o  style.css                       : color, border... etc

  o  sipixel_qualitytest_config_digi.xml  : quality Test Configuration file 
                                            where the tests and the attachments 
                                            of tests with ME are defined, creates 
					    sipixel_qualitytest_config.xml
  o  sipixel_monitorelement_config_digi.xml : the MEs to be used in TrackerMap 
                                            and the summary plot are defined here, creates 
					    sipixel_monitorelement_config.xml


3. Running
=================

  o  Edit the file sipixel_dqm_source_example.cfg in DQM/SiPixelMonitorDigi/test, line 34:
     put the machine name you are starting the job from
  
  o  Edit the file .SiPixelWebClient.xml in DQM/SiPixelMonitorClient/test, line 25:
     set serverHost to the machine you are starting the job from
  
  o  Type: ./StartAll digi 
     to run the whole pixel digi DQM suit of Source, Collector and Client
     
  o  Two additional windows will pop up, one running the Source, the other one running the
     Collector. 
   
  o  Once Source and Collector have stopped chatting with each other (when you can see the progress
     of running over the events from the Source, and the updating of the Collector), start a web
     browser.
     
  o  Point your browser to http://lxplusxxx.cern.ch:1972/ where xxx is the number of the machine
     you started the job from. Click on the SiPixelWebClient symbol, then on 'Configure' and
     'Enable'. Once that is done, click on the 'SiPixelWebInterface' link towards the bottom of the
     page. This brings you to the actual SiPixel specific Web GUI!
     
     
4. Operation     
=======================     
     
  o  Click on 'Subscribe All' to start with. This takes a couple of minutes. 
  
  o  In the 'Single Module View' tab: 'Get Histo List': retrieves a list of 
     the ME's associated with the first module: 
     PixelBarrel/Layer_1/Ladder_01/Module_1, it's number is 302055684.
     They can be selected and plotted interactively with 'Draw Histogram(s)'. 
     
  o  'Update Summary': creates/updates the summary histos, combining all modules
     into larger structures like layers etc. These can be viewed interactively
     on the 'Summary View' tab. 
     
  o  'QTest Result': runs the pre-defined quality tests and attaches the results
     (errors/warnings/etc) to the ME's. These results can be viewed on the
     'Alarm View' tab.   
     
  o  'Save All': saves all subscribed ME's, including their quality test 
     results, and summary ME's in 
     DQM/SiPixelMonitorClient/test/SiPixelWebInterface.root
     
  o  'Collate ME' and 'Create Tk Map' are not yet working properly.
  
  o  You can terminate the client any time by typing ./StopAll in the shell.
     Once all input data are processed (5000 events), the Source window will
     be terminated. You can cleanly close everything else by typing ./StopAll
