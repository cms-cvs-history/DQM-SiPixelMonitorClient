#include "DQM/SiPixelMonitorClient/interface/SiPixelEDAClient.h"

// Framework
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "DQM/SiPixelMonitorClient/interface/SiPixelWebInterface.h"
#include "DQM/SiPixelMonitorClient/interface/SiPixelActionExecutor.h"
#include "DQM/SiPixelMonitorClient/interface/SiPixelInformationExtractor.h"
#include "DQM/SiPixelMonitorClient/interface/SiPixelUtility.h"

#include <SealBase/Callback.h>

#include "xgi/Method.h"
#include "xgi/Utils.h"

#include "cgicc/Cgicc.h"
#include "cgicc/FormEntry.h"
#include "cgicc/HTMLClasses.h"

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string>
#include <sstream>
#include <math.h>

#define BUF_SIZE 256

using namespace edm;
using namespace std;
//
// -- Constructor
//
SiPixelEDAClient::SiPixelEDAClient(const edm::ParameterSet& ps) :
  ModuleWeb("SiPixelEDAClient"){
 //cout<<"Entering  SiPixelEDAClient::SiPixelEDAClient: "<<endl;
 
  string localPath = string("DQM/SiPixelMonitorClient/test/loader.html");
  ifstream fin(edm::FileInPath(localPath).fullPath().c_str(), ios::in);
  char buf[BUF_SIZE];
  
  if (!fin) {
    cerr << "Input File: loader.html"<< " could not be opened!" << endl;
    return;
  }

  while (fin.getline(buf, BUF_SIZE, '\n')) { // pops off the newline character 
    html_out_ << buf ;
  }
  fin.close();

  edm::LogInfo("SiPixelEDAClient") <<  " Creating SiPixelEDAClient " << "\n" ;
  
  bei_ = Service<DQMStore>().operator->();

  summaryFrequency_      = ps.getUntrackedParameter<int>("SummaryCreationFrequency",20);
  tkMapFrequency_        = ps.getUntrackedParameter<int>("TkMapCreationFrequency",50); 
  staticUpdateFrequency_ = ps.getUntrackedParameter<int>("StaticUpdateFrequency",10);
  actionOnLumiSec_       = ps.getUntrackedParameter<bool>("ActionOnLumiSection",false);
  actionOnRunEnd_        = ps.getUntrackedParameter<bool>("ActionOnRunEnd",true);
  evtOffsetForInit_      = ps.getUntrackedParameter<int>("EventOffsetForInit",10);
  offlineXMLfile_        = ps.getUntrackedParameter<bool>("UseOfflineXMLFile",false);
  hiRes_                 = ps.getUntrackedParameter<bool>("HighResolutionOccupancy",false);
  noiseRate_             = ps.getUntrackedParameter<double>("NoiseRateCutValue",0.001);
  noiseRateDenominator_  = ps.getUntrackedParameter<int>("NEventsForNoiseCalculation",100000);
  
  // instantiate web interface
  sipixelWebInterface_ = new SiPixelWebInterface(bei_,offlineXMLfile_);
  sipixelInformationExtractor_ = new SiPixelInformationExtractor(offlineXMLfile_);
  sipixelActionExecutor_ = new SiPixelActionExecutor(offlineXMLfile_);
  
 //cout<<"...leaving  SiPixelEDAClient::SiPixelEDAClient. "<<endl;
}
//
// -- Destructor
//
SiPixelEDAClient::~SiPixelEDAClient(){
//  cout<<"Entering SiPixelEDAClient::~SiPixelEDAClient: "<<endl;
  
  edm::LogInfo("SiPixelEDAClient") <<  " Deleting SiPixelEDAClient " << "\n" ;
//  if (sipixelWebInterface_) {
//     delete sipixelWebInterface_;
//     sipixelWebInterface_ = 0;
//  }
//  if (trackerMapCreator_) {
//    delete trackerMapCreator_;
//    trackerMapCreator_ = 0;
//  }

//  cout<<"...leaving SiPixelEDAClient::~SiPixelEDAClient. "<<endl;
}
//
// -- Begin Job
//
void SiPixelEDAClient::beginJob(const edm::EventSetup& eSetup){
  //cout<<"Entering SiPixelEDAClient::beginJob: "<<endl;

  // Read the summary configuration file
  if (!sipixelWebInterface_->readConfiguration(tkMapFrequency_,summaryFrequency_)) {
     edm::LogInfo ("SiPixelEDAClient") <<"[SiPixelEDAClient]: Error to read configuration file!! Summary will not be produced!!!";
     summaryFrequency_ = -1;
     tkMapFrequency_ = -1;
     actionOnLumiSec_ = false;
     actionOnRunEnd_ = true;
     evtOffsetForInit_ = -1;
  }
  nLumiSecs_ = 0;
  nEvents_   = 0;

  //cout<<"...leaving SiPixelEDAClient::beginJob. "<<endl;
}
//
// -- Begin Run
//
void SiPixelEDAClient::beginRun(Run const& run, edm::EventSetup const& eSetup) {
  edm::LogInfo ("SiPixelEDAClient") <<"[SiPixelEDAClient]: Begining of Run";

}
//
// -- Begin  Luminosity Block
//
void SiPixelEDAClient::beginLuminosityBlock(edm::LuminosityBlock const& lumiSeg, 
                                            edm::EventSetup const& context) {
//  cout<<"Entering SiPixelEDAClient::beginLuminosityBlock: "<<endl;
  
  edm::LogInfo ("SiPixelEDAClient") <<"[SiPixelEDAClient]: Begin of LS transition";

//  cout<<"...leaving SiPixelEDAClient::beginLuminosityBlock. "<<endl;
}
//
//  -- Analyze 
//
void SiPixelEDAClient::analyze(const edm::Event& e, const edm::EventSetup& eSetup){
//  cout<<"[SiPixelEDAClient::analyze()] "<<endl;
  nEvents_++;  
  if(nEvents_==evtOffsetForInit_){
//  cout<<"Doing the initializing now!"<<endl;
    //cout << " Setting up QTests " << endl;
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::setupQTest);
    sipixelWebInterface_->performAction();
    //cout << " Creating Summary Histos" << endl;
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::Summary);
    sipixelWebInterface_->performAction();
    //cout << " Creating occupancy plots" << endl;
    sipixelActionExecutor_->bookOccupancyPlots(bei_, hiRes_);
    //cout << " Booking summary report ME's" << endl;
    sipixelInformationExtractor_->bookGlobalQualityFlag(bei_, noiseRate_);
  }
  sipixelWebInterface_->setActionFlag(SiPixelWebInterface::CreatePlots);
  sipixelWebInterface_->performAction();

}
//
// -- End Luminosity Block
//
void SiPixelEDAClient::endLuminosityBlock(edm::LuminosityBlock const& lumiSeg, edm::EventSetup const& eSetup) {
  //cout<<"Entering SiPixelEDAClient::endLuminosityBlock: "<<endl;

  edm::LogInfo ("SiPixelEDAClient") <<"[SiPixelEDAClient]: End of LS transition, performing the DQM client operation";

  nLumiSecs_++;

  edm::LogInfo("SiPixelEDAClient") << "====================================================== " << endl << " ===> Iteration # " << nLumiSecs_ << " " << lumiSeg.luminosityBlock() << endl  << "====================================================== " << endl;

  if(actionOnLumiSec_){
    //cout << " Updating Summary " << endl;
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::Summary);
    sipixelWebInterface_->performAction();
    //cout << " Checking QTest results " << endl;
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::QTestResult);
    sipixelWebInterface_->performAction();
     //cout << " Updating occupancy plots" << endl;
    sipixelActionExecutor_->bookOccupancyPlots(bei_, hiRes_);
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::Occupancy);
    sipixelWebInterface_->performAction();
    //cout  << " Checking Pixel quality flags " << endl;;
    bei_->cd();
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::ComputeGlobalQualityFlag);
    sipixelWebInterface_->performAction();
    bool init=true;
    sipixelInformationExtractor_->fillGlobalQualityPlot(bei_,init,eSetup);
    //cout << " Checking for new noisy pixels " << endl;
    init=true;
    if(noiseRate_>=0.) sipixelInformationExtractor_->findNoisyPixels(bei_, init, noiseRate_, noiseRateDenominator_, eSetup);
  }   
         
  // -- Create TrackerMap  according to the frequency
//  if (tkMapFrequency_ != -1 && nLumiBlock%tkMapFrequency_ == 1) {
//    cout << " Creating Tracker Map " << endl;
//    trackerMapCreator_->create(bei_);
//    //sipixelWebInterface_->setTkMapFlag(true);
//
//  }
  // Create predefined plots
//  if (nLumiBlock%staticUpdateFrequency_  == 1) {
//    cout << " Creating predefined plots " << endl;
//    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::PlotHistogramFromLayout);
//    sipixelWebInterface_->performAction();
//  }


  //cout<<"...leaving SiPixelEDAClient::endLuminosityBlock. "<<endl;
}
//
// -- End Run
//
void SiPixelEDAClient::endRun(edm::Run const& run, edm::EventSetup const& eSetup){
  //cout<<"Entering SiPixelEDAClient::endRun: "<<endl;

  //edm::LogVerbatim ("SiPixelEDAClient") <<"[SiPixelEDAClient]: End of Run, saving  DQM output ";
  //int iRun = run.run();
  
  if(actionOnRunEnd_){
    //cout << " Updating Summary " << endl;
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::Summary);
    sipixelWebInterface_->performAction();
    //cout << " Checking QTest results " << endl;
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::QTestResult);
    sipixelWebInterface_->performAction();
    //cout << " Updating occupancy plots" << endl;
    sipixelActionExecutor_->bookOccupancyPlots(bei_, hiRes_);
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::Occupancy);
    sipixelWebInterface_->performAction();
    //cout  << " Checking Pixel quality flags " << endl;;
    bei_->cd();
    sipixelWebInterface_->setActionFlag(SiPixelWebInterface::ComputeGlobalQualityFlag);
    sipixelWebInterface_->performAction();
    bool init=true;
    sipixelInformationExtractor_->fillGlobalQualityPlot(bei_,init,eSetup);
    //cout << " Checking for new noisy pixels " << endl;
    init=true;
    if(noiseRate_>=0.) sipixelInformationExtractor_->findNoisyPixels(bei_, init, noiseRate_, noiseRateDenominator_, eSetup);
  }
  
  //cout<<"...leaving SiPixelEDAClient::endRun. "<<endl;
}

//
// -- End Job
//
void SiPixelEDAClient::endJob(){
//  cout<<"In SiPixelEDAClient::endJob "<<endl;
  edm::LogInfo("SiPixelEDAClient") <<"[SiPixelEDAClient]: endjob called!";

}
//
// -- Create default web page
//
void SiPixelEDAClient::defaultWebPage(xgi::Input *in, xgi::Output *out)
{
//  cout<<"Entering SiPixelEDAClient::defaultWebPage: "<<endl;
      
  bool isRequest = false;
  cgicc::Cgicc cgi(in);
  cgicc::CgiEnvironment cgie(in);
  //  edm::LogInfo("SiPixelEDAClient") <<"[SiPixelEDAClient]: defaultWebPage "
  //             << " query string : " << cgie.getQueryString();
  //  if ( xgi::Utils::hasFormElement(cgi,"ClientRequest") ) isRequest = true;
  string q_string = cgie.getQueryString();
  if (q_string.find("RequestID") != string::npos) isRequest = true;
  if (!isRequest) {    
    *out << html_out_.str() << std::endl;
  }  else {
    // Handles all HTTP requests of the form
    int iter = nEvents_/100;
    sipixelWebInterface_->handleEDARequest(in, out, iter);
  }

//  cout<<"...leaving SiPixelEDAClient::defaultWebPage. "<<endl;
}

