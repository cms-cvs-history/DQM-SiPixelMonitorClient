#include "DQM/SiPixelMonitorClient/interface/SiPixelWebInterface.h"
#include "DQM/SiPixelMonitorClient/interface/SiPixelActionExecutor.h"
#include "DQM/SiPixelMonitorClient/interface/SiPixelInformationExtractor.h"
#include "DQM/SiPixelMonitorClient/interface/ANSIColors.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/WebComponents/interface/CgiWriter.h"
#include "DQMServices/WebComponents/interface/CgiReader.h"
#include "DQMServices/WebComponents/interface/ConfigBox.h"
#include "DQMServices/WebComponents/interface/Navigator.h"
#include "DQMServices/WebComponents/interface/ContentViewer.h"
#include "DQMServices/WebComponents/interface/GifDisplay.h"
#include "DQMServices/WebComponents/interface/Select.h"
#include "DQMServices/WebComponents/interface/HTMLLink.h"
#include "DQMServices/WebComponents/interface/Button.h"

#include <SealBase/Callback.h>
#include <map>
#include <iostream>

using namespace std ;


//____________________________________________________________________________________________________
/*
  Create your widgets in the constructor of your web interface
*/
SiPixelWebInterface::SiPixelWebInterface(std::string	      theContextURL, 
                                         std::string 	      theApplicationURL,
					 DQMOldReceiver ** _mui_p)
  : WebInterface(theContextURL, theApplicationURL, _mui_p)
{
  theActionFlag = NoAction;
  actionExecutor_ = 0;
  infoExtractor_  = 0;
  tkMapOptions_.push_back("Persistant");
  tkMapOptions_.push_back("Temporary");
  tkMapCreated = false;
  qflag_=0.;
  createAll();

  if (actionExecutor_ == 0) actionExecutor_ = new SiPixelActionExecutor();
  if (infoExtractor_  == 0) infoExtractor_  = new SiPixelInformationExtractor();
}

//____________________________________________________________________________________________________
//
// -- Create default and customised Widgets
// 
void SiPixelWebInterface::createAll() { 
  HTMLLink *link = new HTMLLink(getApplicationURL(), 
                               "50px", 
			       "50px", 
				"<i>SiPixelWebInterface</i>", 
				"/temporary/Online.html");
  
  page_p = new WebPage(getApplicationURL());
  page_p->add("htmlLink", link);
}

//____________________________________________________________________________________________________
//
// --  Destructor
// 
SiPixelWebInterface::~SiPixelWebInterface() {
  if (actionExecutor_) delete actionExecutor_;
  actionExecutor_ = 0;
  if (infoExtractor_)  delete infoExtractor_;
  infoExtractor_  = 0; 
}

//____________________________________________________________________________________________________
// 
// -- Handles requests from WebElements submitting non-default requests 
//
void SiPixelWebInterface::handleEDARequest(xgi::Input* in,xgi::Output* out, int niter) {
  DQMStore* bei = (*mui_p)->getBEInterface();
  CgiReader reader(in);
  reader.read_form(requestMap_);
  // get the string that identifies the request:
  std::string requestID = get_from_multimap(requestMap_, "RequestID");
  cout << ACGreen << ACBold << ACReverse 
       << "[SiPixelWebInterface::handleEDARequest]"
       << ACCyan 
       << " requestID " 
       << ACPlain
       << requestID << endl;

  if (requestID == "IsReady") {
    theActionFlag = NoAction;    
      returnReplyXml(out, "ReadyState", "wait");
  } else if (requestID == "CheckQTResults") {
   theActionFlag = QTestResult;
  } else if (requestID == "updateIMGCPlots") {	  // <-----------------
    theActionFlag = NoAction;	 
    std::string MEFolder = get_from_multimap(requestMap_, "MEFolder");
    //cout << ACYellow << ACBold
    //     << "[SiPixelWebInterface::handleEDARequest] "
    //	 << ACPlain
    //	 << "Collecting ME from folder " 
    //	 << MEFolder
    //	 << endl ;
    out->getHTTPResponseHeader().addHeader("Content-Type", "text/html");
    out->getHTTPResponseHeader().addHeader("Pragma", "no-cache");   
    out->getHTTPResponseHeader().addHeader("Cache-Control", "no-store, no-cache, must-revalidate,max-age=0");
    out->getHTTPResponseHeader().addHeader("Expires","Mon, 26 Jul 1997 05:00:00 GMT");
    bei->cd() ;
    bei->cd(MEFolder) ;

    vector<std::string> meList = bei->getMEs() ;
    
    *out << MEFolder << " " ;
    bei->cd() ;
    for(vector<std::string>::iterator it=meList.begin(); it!=meList.end(); it++)
    {
     *out << *it << " " ;
    }

  } else if (requestID == "getIMGCPlot") {	  // <-----------------
    std::string plot    = get_from_multimap(requestMap_, "Plot");
    std::string folder  = get_from_multimap(requestMap_, "Folder");
    std::string canvasW = get_from_multimap(requestMap_, "canvasW");
    std::string canvasH = get_from_multimap(requestMap_, "canvasH");
    std::stringstream fullPath ;
    fullPath << folder << "/" << plot ;
    out->getHTTPResponseHeader().addHeader("Content-Type", "image/png");
    out->getHTTPResponseHeader().addHeader("Pragma", "no-cache");   
    out->getHTTPResponseHeader().addHeader("Cache-Control", "no-store, no-cache, must-revalidate,max-age=0");
    out->getHTTPResponseHeader().addHeader("Expires","Mon, 26 Jul 1997 05:00:00 GMT");
    *out << infoExtractor_->getIMGCImage(bei, fullPath.str(), canvasW, canvasH ).str();
    theActionFlag = NoAction;    

  } else if (requestID == "SetupQTest") {	  // <-----------------
   theActionFlag = setupQTest;

  } else if (requestID == "CreateSummary") {	  // <-----------------
    theActionFlag = Summary;

  } else if (requestID == "CreateTkMap") {
    std::string name = "TkMap";
    std::string comment;
    if (tkMapCreated) comment = "Successful";
    else  comment = "Failed";
    returnReplyXml(out, name, comment);
    theActionFlag = CreateTkMap;    
  } else if (requestID == "SingleModuleHistoList") {
    theActionFlag = NoAction;
    infoExtractor_->readModuleAndHistoList(bei, out);    
  } else if (requestID == "ModuleHistoList") {
    theActionFlag = NoAction;
    string sname = get_from_multimap(requestMap_, "StructureName");
    cout<<"in EDARequest: structure name= "<<sname<<endl;
    infoExtractor_->readModuleHistoTree(bei, sname, out);    
  } else if (requestID == "SummaryHistoList") {
    theActionFlag = NoAction;
    string sname = get_from_multimap(requestMap_, "StructureName");
    infoExtractor_->readSummaryHistoTree(bei, sname, out);    
  } else if (requestID == "AlarmList") {
    theActionFlag = NoAction;
    string sname = get_from_multimap(requestMap_, "StructureName");
    infoExtractor_->readAlarmTree(bei, sname, out);    
  } else if (requestID == "ReadQTestStatus") {
    theActionFlag = NoAction;
    string path = get_from_multimap(requestMap_, "Path");
    infoExtractor_->readStatusMessage(bei, path, out);
  } else if (requestID == "PlotAsModule") {
    theActionFlag = PlotSingleModuleHistos;    
  } else if (requestID == "PlotHistogramFromPath") {
   theActionFlag = PlotHistogramFromPath;
  } else if (requestID == "PlotSingleHistogram") {
    theActionFlag = PlotSingleHistogram;

  } else if (requestID == "PlotTkMapHistogram") {
//    cout << ACYellow << ACBold 
//    	 << "[SiPixelWebInterface::handleEDARequest()]" 
//   	 << ACPlain
//   	 << " Requested PlotTkMapHistogram (theOut: "
//	 << theOut
//	 << ")" 
//   	 << endl ;
    string theMEName = get_from_multimap(requestMap_, "MEName");
    string theModId  = get_from_multimap(requestMap_, "ModId");
    infoExtractor_->plotTkMapHisto(bei, theModId, theMEName);
    out->getHTTPResponseHeader().addHeader("Content-Type", "image/png");
    out->getHTTPResponseHeader().addHeader("Pragma", "no-cache");   
    out->getHTTPResponseHeader().addHeader("Cache-Control", "no-store, no-cache, must-revalidate,max-age=0");
    out->getHTTPResponseHeader().addHeader("Expires","Mon, 26 Jul 1997 05:00:00 GMT");
    *out << infoExtractor_->getNamedImage(theMEName).str();
    theActionFlag = NoAction;    
  } else if (requestID == "UpdatePlot") {
    string theMEName = get_from_multimap(requestMap_, "MEName");
    out->getHTTPResponseHeader().addHeader("Content-Type", "image/png");
    out->getHTTPResponseHeader().addHeader("Pragma", "no-cache");   
    out->getHTTPResponseHeader().addHeader("Cache-Control", "no-store, no-cache, must-revalidate,max-age=0");
    out->getHTTPResponseHeader().addHeader("Expires","Mon, 26 Jul 1997 05:00:00 GMT");
    *out << infoExtractor_->getImage().str();
    theActionFlag = NoAction;    
  } else if (requestID == "UpdateTkMapPlot") {
    string theMEName = get_from_multimap(requestMap_, "MEName");
    out->getHTTPResponseHeader().addHeader("Content-Type", "image/png");
    out->getHTTPResponseHeader().addHeader("Pragma", "no-cache");   
    out->getHTTPResponseHeader().addHeader("Cache-Control", "no-store, no-cache, must-revalidate,max-age=0");
    out->getHTTPResponseHeader().addHeader("Expires","Mon, 26 Jul 1997 05:00:00 GMT");
    *out << infoExtractor_->getNamedImage(theMEName).str();
     theActionFlag = NoAction;    

  } else if (requestID == "GetMEList") {
    theActionFlag = NoAction;
    infoExtractor_->readModuleAndHistoList(bei, out);    

  } else if (requestID == "periodicTrackerMapUpdate") {
   theActionFlag = NoAction;
   periodicTkMapUpdate(out) ;

  } else if  (requestID == "globalQFlag") {
    //cout << ACYellow << ACBold << "[SiPixelWebInterface::handleEDARequest]  " 
    //     << ACPlain << "Compute global Pixel quality flag" << endl;
    theActionFlag = ComputeGlobalQualityFlag;
  } else if (requestID == "dumpModIds") {
    theActionFlag = dumpModIds;
  }
    
  performAction();
}

//____________________________________________________________________________________________________
//
// -- Setup Quality Tests
// 
void SiPixelWebInterface::setupQTests() {
  DQMStore * bei = (*mui_p)->getBEInterface();
  actionExecutor_->setupQTests(bei);
}

//____________________________________________________________________________________________________
//
// -- Read Configurations 
//
void SiPixelWebInterface::readConfiguration(int& tkmap_freq, 
                                            int& sum_barrel_freq, 
					    int& sum_endcap_freq, 
					    int& sum_grandbarrel_freq, 
					    int& sum_grandendcap_freq, 
					    int& message_limit,
					    int& source_type){
  if (actionExecutor_)  {
    if (actionExecutor_->readConfiguration(tkmap_freq,
                                           sum_barrel_freq,
					   sum_endcap_freq,
					   sum_grandbarrel_freq,
					   sum_grandendcap_freq,
					   message_limit,
					   source_type));
  } else {
    tkmap_freq = -1;
    sum_barrel_freq   = -1;
    sum_endcap_freq   = -1;
    sum_grandbarrel_freq   = -1;
    sum_grandendcap_freq   = -1;
    message_limit   = -1;
    source_type  = -1;
  }
}

//____________________________________________________________________________________________________
void SiPixelWebInterface::readConfiguration(int& tkmap_freq,int& summary_freq){
  if (actionExecutor_)  {
    if (actionExecutor_->readConfiguration(tkmap_freq,summary_freq));
  } else {
    tkmap_freq = -1;
    summary_freq = -1;
  }
}

//____________________________________________________________________________________________________
// -- Perform action
//
void SiPixelWebInterface::performAction() {
//cout<<"entering performAction..."<<endl;
  DQMStore * bei = (*mui_p)->getBEInterface();
  switch (theActionFlag) {
  case SiPixelWebInterface::CreateTkMap :
    {
     if (createTkMap()) {
       tkMapCreated = true;
//       theOut->getHTTPResponseHeader().addHeader("Content-Type", "text/xml");
//      *theOut << "<?xml version=\"1.0\" ?>"	     << endl;
//      *theOut << "<TkMap>"			     << endl;
//      *theOut << " <Response>Successfull</Response>" << endl;
//      *theOut << "</TkMap>"			     << endl;
     }
      break;
    }
  case SiPixelWebInterface::Summary :
    {
      actionExecutor_->createSummary(bei);
      break;
    }
  case SiPixelWebInterface::setupQTest :
    {
      actionExecutor_->setupQTests(bei);
      break;
    }
  case SiPixelWebInterface::QTestResult :
    {
      actionExecutor_->checkQTestResults(bei);
      break;
    }
  case SiPixelWebInterface::PlotSingleModuleHistos :
    {
      infoExtractor_->plotSingleModuleHistos(bei, requestMap_);
      break;
    }
  case SiPixelWebInterface::PlotTkMapHistogram :
    {
      string theMEName = get_from_multimap(requestMap_, "MEName");
      string theModId  = get_from_multimap(requestMap_, "ModId");
      infoExtractor_->plotTkMapHisto(bei, theModId, theMEName);

      cout << ACYellow << ACBold 
           << "[SiPixelWebInterface::PlotTkMapHistogram()]"
           << ACPlain
           << " Buffer for "
 	   <<  theMEName
 	   << " stored away: shipping back header (" << theOut << ")" 
           << endl ;
//      theOut->getHTTPResponseHeader().addHeader("Content-Type", "image/png");
//      theOut->getHTTPResponseHeader().addHeader("Pragma", "no-cache");   
//      theOut->getHTTPResponseHeader().addHeader("Cache-Control", "no-store, no-cache, must-revalidate,max-age=0");
//      theOut->getHTTPResponseHeader().addHeader("Expires","Mon, 26 Jul 1997 05:00:00 GMT");
//      cout << ACYellow << ACBold 
//           << "[SiPixelWebInterface::PlotTkMapHistogram()]"
//           << ACPlain
//           << " Buffer for "
// 	   <<  theMEName
// 	   << " stored away: shipping back data (" << theOut << ")" 
//           << endl ;
//      *theOut << infoExtractor_->getNamedImage(theMEName).str();
      break;
    }
  case SiPixelWebInterface::PlotSingleHistogram :
    {
      infoExtractor_->plotSingleHistogram(bei, requestMap_);
      break;
    }
  case SiPixelWebInterface::periodicTrackerMapUpdate :
    {
      break;
    }
  case SiPixelWebInterface::PlotHistogramFromPath :
    {
      infoExtractor_->plotHistosFromPath(bei, requestMap_);
      break;
    }
  case SiPixelWebInterface::ComputeGlobalQualityFlag  :
    {
      qflag_ = infoExtractor_->computeGlobalQualityFlag(bei);
      break;
    }
  case SiPixelWebInterface::dumpModIds  :
    {
      actionExecutor_->dumpModIds(bei);
      break;
    }
  case SiPixelWebInterface::NoAction :
    {
      break;
    }
  }
  setActionFlag(SiPixelWebInterface::NoAction);
//  cout<<"leaving performAction..."<<endl;
}


//____________________________________________________________________________________________________
void SiPixelWebInterface::returnReplyXml(xgi::Output * out, 
                                         const std::string& name, 
					 const std::string& comment){
   out->getHTTPResponseHeader().addHeader("Content-Type", "text/xml");
  *out << "<?xml version=\"1.0\" ?>" << std::endl;
  *out << "<TkMap>" << endl;
  *out << " <Response>" << comment << "</Response>" << endl;
  *out << "</TkMap>" << endl;

}


//____________________________________________________________________________________________________
bool SiPixelWebInterface::createTkMap() {
  DQMStore * bei = (*mui_p)->getBEInterface();
  if (theActionFlag == SiPixelWebInterface::CreateTkMap) {
    string sname     = get_from_multimap(requestMap_, "MEName");
    string theTKType = get_from_multimap(requestMap_, "TKMapType");
    actionExecutor_->createTkMap(bei, sname, theTKType);
    return true;
  } else {
    return false;
  }
}



//____________________________________________________________________________________________________
void SiPixelWebInterface::periodicTkMapUpdate(xgi::Output * out)
{
  DQMStore * bei = (*mui_p)->getBEInterface();
  string sname     = get_from_multimap(requestMap_, "MEName");
  string theTKType = get_from_multimap(requestMap_, "TKMapType");
  infoExtractor_->sendTkUpdatedStatus(bei, out, sname, theTKType) ;
}
