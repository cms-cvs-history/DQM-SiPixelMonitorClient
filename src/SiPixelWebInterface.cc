#include "DQM/SiPixelMonitorClient/interface/SiPixelWebInterface.h"
#include "DQM/SiPixelMonitorClient/interface/SiPixelActionExecutor.h"
#include "DQM/SiPixelMonitorClient/interface/SiPixelInformationExtractor.h"
#include "DQM/SiPixelMonitorClient/interface/ANSIColors.h"
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


/*
  Create your widgets in the constructor of your web interface
*/
SiPixelWebInterface::SiPixelWebInterface(std::string theContextURL, std::string theApplicationURL, MonitorUserInterface ** _mui_p)
  : WebInterface(theContextURL, theApplicationURL, _mui_p)
{
  theActionFlag = NoAction;
  actionExecutor_ = 0;
  infoExtractor_  = 0;
  tkMapOptions_.push_back("Persistant");
  tkMapOptions_.push_back("Temporary");
  tkMapCreated = false;
  createAll();

  if (actionExecutor_ == 0) actionExecutor_ = new SiPixelActionExecutor();
  if (infoExtractor_  == 0) infoExtractor_  = new SiPixelInformationExtractor();
}


//
// -- Create default and customised Widgets
// 
void SiPixelWebInterface::createAll() { 
  HTMLLink *link = new HTMLLink(getApplicationURL(), "50px", "50px", 
				"<i>SiPixelWebInterface</i>", 
				"/temporary/Online.html");
  
  page_p = new WebPage(getApplicationURL());
  page_p->add("htmlLink", link);
}
//
// --  Destructor
// 
SiPixelWebInterface::~SiPixelWebInterface() {
  if (actionExecutor_) delete actionExecutor_;
  actionExecutor_ = 0;
  if (infoExtractor_) delete infoExtractor_;
  infoExtractor_ = 0; 
}
// 
// -- Handles requests from WebElements submitting non-default requests 
//
void SiPixelWebInterface::handleCustomRequest(xgi::Input* in,xgi::Output* out)
  throw (xgi::exception::Exception)
{
//  cout<<"entering handleCustomRequest"<<endl;
  // put the request information in a multimap...
  //std::multimap<std::string, std::string> request_multimap;
  CgiReader reader(in);
  reader.read_form(requestMap_);

  // get the string that identifies the request:
  std::string requestID = get_from_multimap(requestMap_, "RequestID");
  cout << ACGreen << ACBold << ACReverse 
       << "[SiPixelWebInterface::handleCustomRequest]"
       << ACCyan 
       << " requestID " 
       << ACPlain
       << requestID << endl;
  if (requestID == "SubscribeAll") {
    theActionFlag = SubscribeAll;
  } else if (requestID == "SetupQTest") {
    theActionFlag = setupQTest;
  } else if (requestID == "CheckQTResults") {
    theActionFlag = QTestResult;
  } else if (requestID == "CreateSummary") {
     theActionFlag = Summary;
  } else if (requestID == "SaveToFile") {
     theActionFlag = SaveData;
  } else if (requestID == "CollateME") {
     theActionFlag = Collate;
  } else if (requestID == "CreateTkMap") {
     theActionFlag = CreateTkMap;
     theOut = out ;
  } else if (requestID == "OpenTkMap") {
    std::string name = "TkMap";
    std::string comment;
    if (tkMapCreated) comment = "Successful";
    else  comment = "Failed";
    returnReplyXml(out, name, comment);
    theActionFlag = NoAction;   
  } else if (requestID == "SingleModuleHistoList") {
    theActionFlag = NoAction;
    infoExtractor_->readModuleAndHistoList((*mui_p), out, actionExecutor_->getCollationFlag() );    
  } else if (requestID == "ModuleHistoList") {
    theActionFlag = NoAction;
    string sname = get_from_multimap(requestMap_, "StructureName");
    infoExtractor_->readModuleHistoTree((*mui_p), sname, out,
                          actionExecutor_->getCollationFlag());    
  } else if (requestID == "SummaryHistoList") {
    theActionFlag = NoAction;
    string sname = get_from_multimap(requestMap_, "StructureName");
    infoExtractor_->readSummaryHistoTree((*mui_p), sname, out,
                           actionExecutor_->getCollationFlag());    
  } else if (requestID == "AlarmList") {
    theActionFlag = NoAction;
    string sname = get_from_multimap(requestMap_, "StructureName");
    infoExtractor_->readAlarmTree((*mui_p), sname, out,
                           actionExecutor_->getCollationFlag());    
  } else if (requestID == "ReadQTestStatus") {
    theActionFlag = NoAction;
    string path = get_from_multimap(requestMap_, "Path");
    infoExtractor_->readStatusMessage((*mui_p), path, out);
  } else if (requestID == "PlotAsModule") {
    theActionFlag = PlotSingleModuleHistos;    
  } else if (requestID == "PlotSingleHistogram") {
   theActionFlag = PlotSingleHistogram;
  } else if (requestID == "PlotTkMapHistogram") {
   theActionFlag = PlotTkMapHistogram;
//    cout << ACYellow << ACBold 
//         << "[SiPixelWebInterface::handleCustomRequest()]" 
// 	<< ACPlain
// 	<< " Requested PlotTkMapHistogram" 
// 	<< endl ;
  } else if (requestID == "UpdatePlot") {
   string theMEName = get_from_multimap(requestMap_, "MEName");
   out->getHTTPResponseHeader().addHeader("Content-Type", "image/png");
   out->getHTTPResponseHeader().addHeader("Pragma", "no-cache");   
   out->getHTTPResponseHeader().addHeader("Cache-Control", "no-store, no-cache, must-revalidate,max-age=0");
   out->getHTTPResponseHeader().addHeader("Expires","Mon, 26 Jul 1997 05:00:00 GMT");
//    cout << ACYellow << ACBold 
//         << "[SiPixelWebInterface::handleCustomRequest()]" 
// 	<< ACPlain
// 	<< " Shipping plot back to web client" 
// 	<< endl ;
   *out << infoExtractor_->getNamedImage(theMEName).str();
    theActionFlag = NoAction;    
  } else if (requestID == "GetMEList") {
    theActionFlag = NoAction;
    infoExtractor_->readModuleAndHistoList((*mui_p), out, actionExecutor_->getCollationFlag() );    
//     cout << ACYellow << ACBold 
//          << "[SiPixelWebInterface::handleCustomRequest()] Gotcha MEList!" << ACPlain
// 	 << endl ;
  } else if (requestID == "periodicTrackerMapUpdate") {
   theActionFlag = NoAction;
   periodicTkMapUpdate(out) ;
  }
  configureCustomRequest(in, out);
//  cout<<"leaving handleCustomRequest"<<endl;
}
//
// -- Schedule Custom Action
//
void SiPixelWebInterface::configureCustomRequest(xgi::Input * in, xgi::Output * out) throw (xgi::exception::Exception){
  seal::Callback action(seal::CreateCallback(this, 
                      &SiPixelWebInterface::performAction));
  (*mui_p)->addCallback(action);
}
//
// -- Setup Quality Tests
// 
void SiPixelWebInterface::setupQTests() {
  actionExecutor_->setupQTests((*mui_p));
}
//
// -- Read Configurations 
//
void SiPixelWebInterface::readConfiguration(int& tkmap_freq, int& sum_barrel_freq, int& sum_endcap_freq){
  if (actionExecutor_)  {
    if (actionExecutor_->readConfiguration(tkmap_freq,sum_barrel_freq,sum_endcap_freq));
  } else {
    tkmap_freq = -1;
    sum_barrel_freq   = -1;
    sum_endcap_freq   = -1;
  }
}
//
// -- Perform action
//
void SiPixelWebInterface::performAction() {
//cout<<"entering performAction..."<<endl;
  switch (theActionFlag) {
  case SiPixelWebInterface::SubscribeAll :
    {
//      cout << " SiPixelWebInterface::subscribeAll " << endl;
//      (*mui_p)->subscribe("Collector/FU0/Tracker/PixelBarrel/Layer_1/Ladder_01/*");
      (*mui_p)->subscribe("Collector/*");
      break;
    } 
  case SiPixelWebInterface::Collate :
    {
//      cout << " SiPixelWebInterface::Collate " << endl;
      actionExecutor_->createCollation((*mui_p));
      break;
    }
  case SiPixelWebInterface::CreateTkMap :
    {
     if (createTkMap()) {
       tkMapCreated = true;
       theOut->getHTTPResponseHeader().addHeader("Content-Type", "text/xml");
      *theOut << "<?xml version=\"1.0\" ?>"	     << endl;
      *theOut << "<TkMap>"			     << endl;
      *theOut << " <Response>Successfull</Response>" << endl;
      *theOut << "</TkMap>"			     << endl;
     }
      break;
    }
  case SiPixelWebInterface::Summary :
    {
//      cout << " SiPixelWebInterface::Summary " << endl;
      actionExecutor_->createSummary((*mui_p));
      break;
    }
  case SiPixelWebInterface::setupQTest :
    {
//      cout << " SiPixelWebInterface::setupQTests " << endl;
      actionExecutor_->setupQTests((*mui_p));
      break;
    }
  case SiPixelWebInterface::QTestResult :
    {
//      cout << " SiPixelWebInterface::QTestResult " << endl;
      actionExecutor_->checkQTestResults((*mui_p));
      break;
    }
  case SiPixelWebInterface::SaveData :
    {
//      cout << " Saving Monitoring Elements " << endl;
      actionExecutor_->saveMEs((*mui_p), "SiPixelWebClient.root");
      break;
    }
  case SiPixelWebInterface::PlotSingleModuleHistos :
    {
//      cout << " SiPixelWebInterface::PlotSingleModuleHistos " << endl;
      infoExtractor_->plotSingleModuleHistos((*mui_p), requestMap_);
      break;
    }
  case SiPixelWebInterface::PlotTkMapHistogram :
    {
/* --> old tkmap plot
      vector<string> mes;
      int nval = actionExecutor_->getTkMapMENames(mes);
      if (nval == 0) break;
      for  (vector<string>::iterator it = mes.begin();
	    it != mes.end(); it++) {
	requestMap_.insert(pair<string,string>("histo",(*it)));  
      }
      string sName  = get_from_multimap(requestMap_, "MEName");
      cout << ACYellow << ACBold 
           << "[SiPixelWebInterface::PlotTkMapHistogram()]"
           << ACPlain
           << " ... further processing request for "
	   <<  sName
           << endl ;
      infoExtractor_->plotTkMapHistos((*mui_p), requestMap_, sName);
*/
// --> new tkmap plot
      string theMEName = get_from_multimap(requestMap_, "MEName");
      string theModId  = get_from_multimap(requestMap_, "ModId");
//       cout << ACYellow << ACBold 
//            << "[SiPixelWebInterface::PlotTkMapHistogram()]"
//            << ACPlain
//            << " further processing request for "
// 	   <<  theMEName
// 	   << " (" << theModId << ")"
//            << endl ;
      infoExtractor_->plotTkMapHisto((*mui_p), theModId, theMEName);
      break;
    }
  case SiPixelWebInterface::PlotSingleHistogram :
    {
//      cout << " SiPixelWebInterface::PlotSingleHistogram " << endl;
      infoExtractor_->plotSingleHistogram((*mui_p), requestMap_);
      break;
    }
  case SiPixelWebInterface::periodicTrackerMapUpdate :
    {
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

void SiPixelWebInterface::returnReplyXml(xgi::Output * out, const std::string& name, const std::string& comment){
   out->getHTTPResponseHeader().addHeader("Content-Type", "text/xml");
  *out << "<?xml version=\"1.0\" ?>" << std::endl;
  *out << "<TkMap>" << endl;
  *out << " <Response>" << comment << "</Response>" << endl;
  *out << "</TkMap>" << endl;
//  cout <<  "<?xml version=\"1.0\" ?>" << std::endl;
//  cout << "<TkMap>" << endl;
//  cout << " <Response>" << comment << "</Response>" << endl;
//  cout << "</TkMap>" << endl;

}

bool SiPixelWebInterface::createTkMap() {
  if (theActionFlag == SiPixelWebInterface::CreateTkMap) {
    string sname = get_from_multimap(requestMap_, "MEName");
    actionExecutor_->createTkMap((*mui_p), sname);
    return true;
  } else {
    return false;
  }
}


void SiPixelWebInterface::periodicTkMapUpdate(xgi::Output * out)
{
  string sname = get_from_multimap(requestMap_, "MEName");
  infoExtractor_->sendTkUpdatedStatus((*mui_p), out, sname) ;
}
