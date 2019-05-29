// CPP headers
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

// ROOT headers
#include <TH1F.h>
#include <TH2F.h>
#include <TROOT.h>
#include <TFile.h>
#include <TSystem.h>

// CMSSW headers
#include "DataFormats/JetReco/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/FWLite/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/FWLite/interface/FWLiteEnabler.h"
#include "PhysicsTools/FWLite/interface/TFileService.h"
#include "PhysicsTools/FWLite/interface/CommandLineParser.h"

// Headers from this package
#include "Analysis/Analysis_boostedNmssmHiggs/interface/Kinematics.h"
#include "Analysis/Analysis_boostedNmssmHiggs/interface/PlottingDoubleBTaggerEfficiencyStudies.h"

// PAT TUPLE FORMAT ONLY !!!

// preliminary running, compile with scram b and then
// $ DoubleBTaggerEfficiencyStudies inputfiles=XYZ outputfile=ABC orderedsecondaryfiles=0
// ($CMSSW_BASE/tmp/slc6_amd64_gcc530/src/Analysis/Analysis_boostedNmssmHiggs/bin/DoubleBTaggerEfficiencyStudies/DoubleBTaggerEfficiencyStudies)
// nb. if you are running it on DICE, include the word runOnDice at the end of the arguments of the executable

/* Notes on runOnDice mode
watchout...with this toggle the executable can now overwrite outputs!!!
hacks the original script abit so it can do some things slightly differently
it does so using the quantity: bool runOnDice
ALSO...make sure 'inputfiles' does not have a default value!!! It will screw it
*/


void CreateHistograms(std::map<std::string,TH1F*>&, std::map<std::string,TH2F*>&, std::vector<std::string>, std::vector<double>);
void FillHistograms(std::map<std::string,TH1F*>&, std::map<std::string,TH2F*>&, bool, bool, pat::Jet, pat::Jet, reco::GenParticle, double, double, std::vector<double>, std::vector<std::string>, std::vector<double>, std::vector<double>);
void WriteHistograms(std::map<std::string,TH1F*>&, std::map<std::string,TH2F*>&, std::string);
void WriteHistogramsDICE(std::map<std::string,TH1F*>&, std::map<std::string,TH2F*>&, std::string);
std::vector<reco::GenParticle> higgsBbGenParticles(edm::Handle<std::vector<reco::GenParticle>>, std::vector<double> &, std::vector<double> &, std::vector<std::vector<double>> &);
bool isThereAFatJetMatch(edm::Handle<std::vector<pat::Jet>>, reco::GenParticle, double, pat::Jet&);
bool isThereSoftDropMapping(edm::Handle<std::vector<pat::Jet>>, pat::Jet &, double, pat::Jet&);
std::string getOutputDirFromOutputFile(std::string);
double minPtFrac(const double&, const double&);

int main(int argc, char* argv[]) 
{
	gSystem->Load("libFWCoreFWLite.so");
	FWLiteEnabler::enable();

	////////////////////
	////////////////////
	// Set parameters //
	std::vector<double> doubleBtagWP = {0.3, 0.6, 0.8, 0.9}; // these WP vectors must correspond to one-another
	std::vector<std::string> doubleBtagWPname = {"loose", "medium", "tight_1", "tight_2"};
	double dRMaxMatch = 0.8; // max dR between higgs boson and fatJet to claim a match
	double dRMaxMatchJets = 0.4; // max dR between fatJet and fatJetSoftDrop to claim a match
	// std::vector<double> etaBinning = {0.00, 0.80, 1.60, 2.40};
	std::vector<double> etaBinning = {0.00, 2.40};
	/////////////////////
	/////////////////////

	// see if it is in runOnDice mode
	bool runOnDice = false;
    for (int i = 0; i < argc; ++i) {
        std::string argvString(argv[i]);
        if (argvString == "runOnDice") runOnDice = true;
    }

	// Create histograms, they are accessed by eg: h_["fatJetMass_loose"]->Fill(125.0);
	std::map<std::string, TH1F*> h_;
	std::map<std::string, TH2F*> h2_;
	CreateHistograms(h_, h2_, doubleBtagWPname, etaBinning);

	// Initialize command line parser
	optutl::CommandLineParser parser ("Analyze DoubleBTagger Efficiencies");

	//////////////////
	// Set defaults //
	parser.integerValue ("maxevents"      ) = -1; // -1 for all events
	parser.integerValue ("outputevery"    ) =   100;
	// parser.stringVector ("inputfiles"    ) = {"/hdfs/user/jt15104/Analysis_boostedNmssmHiggs/patTuples/CMSSW_8_0_20/signalSamples/nmssmSignalCascadeV05_13TeV_mH70p0_mSusy1800p0_ratio0p99_splitting0p5/nmssmSignalCascadeV05_13TeV_patTupleAddBTag_ed12MJI_mH70p0_mSusy1800p0_ratio0p99_splitting0p5/bTagPatTuple_888.root"};
	// parser.stringValue  ("outputfile"     ) = "output_DoubleBTaggerEfficiencyStudies/histos.root";
	parser.boolValue    ("orderedsecondaryfiles") = false;
	//////////////////

	// Parse arguments_
	if (runOnDice) parser.parseArguments (argc-1, argv);
	else parser.parseArguments (argc, argv);
	int maxEvents_ = parser.integerValue("maxevents");
	unsigned int outputEvery_ = parser.integerValue("outputevery");
	std::vector<std::string> inputFiles_ = parser.stringVector("inputfiles");
	std::string outputFile_ = parser.stringValue("outputfile");
	bool justDoPlotting_ = parser.boolValue("orderedsecondaryfiles");

	if (runOnDice==false){ // don't do this if on DICE as it cannot do this command locally

		std::string outputDirectory_ = getOutputDirFromOutputFile(outputFile_);
		bool makeDir = !(std::system(Form("mkdir %s",outputDirectory_.c_str())));
		if (justDoPlotting_ == false && makeDir == false){
			std::cout << "The chosen output directory already exists, or the parent directory does not exist:" << std::endl;
			std::cout << "Do not wish to overwrite ROOT file: Exiting..." << std::endl;
			return 1;
		}
		
		// copy the code used to make the histogram ROOT file into the same directory (this could be out of sync if you edit after compilation)
		// also copy the parser values that were used to a .txt file (ie the input data used)
		if (justDoPlotting_ == false){
			
			std::system(Form("cp $CMSSW_BASE/src/Analysis/Analysis_boostedNmssmHiggs/bin/DoubleBTaggerEfficiencyStudies.cpp %s",outputDirectory_.c_str()));
			
			std::ofstream parserRecord;
			parserRecord.open(Form("%soptionsUsed.txt",outputDirectory_.c_str()));
			parserRecord << "MaxEvents: " << maxEvents_ << "\n";
			parserRecord << "InputFiles: " << "\n";
			for(unsigned int iFile=0; iFile<inputFiles_.size(); ++iFile){
				parserRecord << inputFiles_[iFile] << "\n";
			}
			parserRecord.close();
		}
	} // closes 'if' runOnDice


	// Loop through the input files
	int ievt=0;
	// but we are just doing the plotting goto the end
	if (justDoPlotting_) goto plottingLabel;

	for(unsigned int iFile=0; iFile<inputFiles_.size(); ++iFile){
	    
	    // Open input file
	    TFile* inFile = TFile::Open(inputFiles_[iFile].c_str());
	    if( inFile ){
		
			fwlite::Event ev(inFile);
			// Loop through the events for this file
			for(ev.toBegin(); !ev.atEnd(); ++ev, ++ievt){
				edm::EventBase const & event = ev;

				// break loop if maximal number of events is reached 
				if(maxEvents_>0 ? ievt+1>maxEvents_ : false) break;
				
				// event counter
				if(outputEvery_!=0 ? (ievt>0 && ievt%outputEvery_==0) : false){
					std::cout << "   File " << iFile+1 << " of " << inputFiles_.size() << ":";
					std::cout << "  processing event: " << ievt << std::endl;
				}

				// Handle to the fatJet collection
				edm::Handle<std::vector<pat::Jet>> fatJets;
				event.getByLabel(std::string("selectedPatJetsAK8PFCHS"), fatJets);

				// Handle to the groomedFatJet collection
				edm::Handle<std::vector<pat::Jet>> softDropFatJets;
				event.getByLabel(std::string("selectedPatJetsAK8PFCHSSoftDrop"), softDropFatJets);

				// Handle to the genParticle collection
				edm::Handle<std::vector<reco::GenParticle>> genParticles;
				event.getByLabel(std::string("genParticles"), genParticles);		 



				// A N A L Y S I S
				// ------------------------------------------------------------------------------------------------------------//
				// Find the higgs to bb particles, store the objects in a vector
				std::vector<double> dRbbVec; // gets updated by 'higgsBbGenParticles' function
				std::vector<double> minPtFracVec; // gets updated by 'higgsBbGenParticles' function
				
				// this is a debugging vector containing the higgs' bb position info
				// an element for each higgs, within that
				// the four elements hold b0_phi,b0_eta,b1_phi,b1_eta 
				std::vector<std::vector<double>> debugBbPositionsVec;
				
				std::vector<reco::GenParticle> higgsBbParticles = higgsBbGenParticles(genParticles, dRbbVec, minPtFracVec, debugBbPositionsVec); 
				// Loop through the higgsBb particles
				for (size_t iH = 0; iH < higgsBbParticles.size(); ++iH){

					const reco::GenParticle higgsBbParticle = higgsBbParticles[iH];

					// See if there is a fatJet that matches to the higgsBb (closest in dR, must have dR<dRMaxMatch)
					pat::Jet fatJetMatch; // if there is a matching fatJet, this object will contain it
					bool isMatch = isThereAFatJetMatch(fatJets, higgsBbParticle, dRMaxMatch, fatJetMatch);

					pat::Jet softDropFatJetMap; // if there is a softDropFatJet that maps onto the fatJet, this object will contain it
					bool isJetMapping = false;
					if (isMatch) isJetMapping = isThereSoftDropMapping(softDropFatJets, fatJetMatch, dRMaxMatchJets, softDropFatJetMap);

					FillHistograms(h_, h2_, isMatch, isJetMapping, fatJetMatch, softDropFatJetMap, higgsBbParticle, dRbbVec[iH], minPtFracVec[iH], debugBbPositionsVec[iH], doubleBtagWPname, doubleBtagWP, etaBinning);

				} // closes loop through higgsBb Particles
				// ------------------------------------------------------------------------------------------------------------//

			} // closes loop through events for this file
			inFile->Close();
	    } // closes 'if' the file exists

	    // break loop if maximal number of events is reached:
	    // this has to be done twice to stop the file loop as well
	    if(maxEvents_>0 ? ievt+1>maxEvents_ : false) break;

	} // closes loop through files


	// don't do the .pdf plotting on DICE (we will want to hadd outputs first)
	if (runOnDice==true){ 
		WriteHistogramsDICE(h_, h2_, outputFile_.c_str());
		return 0;
	}  

	WriteHistograms(h_, h2_, outputFile_.c_str());
	plottingLabel:
	if (std::system(Form("test -e %s",outputFile_.c_str())) == 0) // check the file exists
		PlottingDoubleBTaggerEfficiencyStudies createPlots(outputFile_.c_str(), doubleBtagWPname, etaBinning);
	else{
		std::cout << "the following output root file does not exist to make plots from:" << std::endl;
		std::cout << outputFile_ << std::endl;
	}

return 0;
} // closes the 'main' function






void CreateHistograms(std::map<std::string,TH1F*> & h_, std::map<std::string,TH2F*> & h2_, std::vector<std::string> doubleBtagWPnameD, const std::vector<double> etaBinningD)
{
	// set the binning for histograms
    std::vector<double> ptBinning;
    // for(double binLowerEdge=  0.0; binLowerEdge< 100.0; binLowerEdge+= 50.0) ptBinning.push_back(binLowerEdge);
    // for(double binLowerEdge=  100.0; binLowerEdge< 150.0; binLowerEdge+= 50.0) ptBinning.push_back(binLowerEdge);
    // for(double binLowerEdge=  150.0; binLowerEdge< 350.0; binLowerEdge+= 25.0) ptBinning.push_back(binLowerEdge);
    for(double binLowerEdge=  0.0; binLowerEdge< 1000.0; binLowerEdge+= 50.0) ptBinning.push_back(binLowerEdge);    	
    for(double binLowerEdge=  1000.0; binLowerEdge< 2000.1; binLowerEdge+= 100.0) ptBinning.push_back(binLowerEdge);
    // for(double binLowerEdge=  600.0; binLowerEdge< 800.1; binLowerEdge+= 200.0) ptBinning.push_back(binLowerEdge);

    std::vector<double> ptBinningSoftDrop;
    // for(double binLowerEdge=  0.0; binLowerEdge< 100.0; binLowerEdge+= 50.0) ptBinningSoftDrop.push_back(binLowerEdge);
    // for(double binLowerEdge=  100.0; binLowerEdge< 150.0; binLowerEdge+= 50.0) ptBinningSoftDrop.push_back(binLowerEdge);
    // for(double binLowerEdge=  150.0; binLowerEdge< 350.0; binLowerEdge+= 25.0) ptBinningSoftDrop.push_back(binLowerEdge);
    for(double binLowerEdge=  0.0; binLowerEdge< 400.1; binLowerEdge+= 10.0) ptBinningSoftDrop.push_back(binLowerEdge);    	
    // for(double binLowerEdge=  1000.0; binLowerEdge< 2000.1; binLowerEdge+= 100.0) ptBinningSoftDrop.push_back(binLowerEdge);
    // for(double binLowerEdge=  600.0; binLowerEdge< 800.1; binLowerEdge+= 200.0) ptBinningSoftDrop.push_back(binLowerEdge);

    std::vector<double> ptScatXBinning;
    for(double binLowerEdge=  0.0; binLowerEdge< 2000.1; binLowerEdge+= 5.0) ptScatXBinning.push_back(binLowerEdge);

    std::vector<double> ptScatYBinning = ptScatXBinning;
    // for(double binLowerEdge=  0.0; binLowerEdge< 800.1; binLowerEdge+= 20.0) ptScatYBinning.push_back(binLowerEdge);	

    std::vector<double> massBinning;
    for(double binLowerEdge=  0.0; binLowerEdge< 200.1; binLowerEdge+= 2.5) massBinning.push_back(binLowerEdge);

    std::vector<double> etaDistBinning;
    for(double binLowerEdge=  -4.00; binLowerEdge< 4.0001; binLowerEdge+= 0.10) etaDistBinning.push_back(binLowerEdge);    

    std::vector<double> matchDeltaRDistBinning;
    for(double binLowerEdge=  0.0; binLowerEdge< 0.8001; binLowerEdge+= 0.01) matchDeltaRDistBinning.push_back(binLowerEdge); 

    std::vector<double> bbDeltaRDistBinning;
    for(double binLowerEdge=  0.0; binLowerEdge< 2.501; binLowerEdge+= 0.025) bbDeltaRDistBinning.push_back(binLowerEdge); 

    std::vector<double> dREffBinning;
    for(double binLowerEdge=  0.0; binLowerEdge< 0.40; binLowerEdge+= 0.10) dREffBinning.push_back(binLowerEdge);
    for(double binLowerEdge=  0.40; binLowerEdge< 0.70; binLowerEdge+= 0.10) dREffBinning.push_back(binLowerEdge);
    for(double binLowerEdge=  0.70; binLowerEdge< 1.00; binLowerEdge+= 0.10) dREffBinning.push_back(binLowerEdge);
    for(double binLowerEdge=  1.00; binLowerEdge< 1.10; binLowerEdge+= 0.10) dREffBinning.push_back(binLowerEdge);
    for(double binLowerEdge=  1.10; binLowerEdge< 1.70; binLowerEdge+= 0.20) dREffBinning.push_back(binLowerEdge);
    for(double binLowerEdge=  1.70; binLowerEdge< 2.501; binLowerEdge+= 0.40) dREffBinning.push_back(binLowerEdge);

	// create the debugging histograms
	h_["DEBUG_higgsBbDRpreMatching"] = new TH1F("DEBUG_higgsBbDRpreMatching", ";MC dR_bb;a.u.", 100, 0, 2.50);

	h_["softDropJetMass_drLessThan0p8"] = new TH1F("softDropJetMass_drLessThan0p8", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drMoreThan0p8"] = new TH1F("softDropJetMass_drMoreThan0p8", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drBetween0p0and0p1"] = new TH1F("softDropJetMass_drBetween0p0and0p1", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drBetween0p1and0p2"] = new TH1F("softDropJetMass_drBetween0p1and0p2", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drBetween0p2and0p3"] = new TH1F("softDropJetMass_drBetween0p2and0p3", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drBetween0p3and0p4"] = new TH1F("softDropJetMass_drBetween0p3and0p4", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drBetween0p4and0p5"] = new TH1F("softDropJetMass_drBetween0p4and0p5", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drBetween0p5and0p6"] = new TH1F("softDropJetMass_drBetween0p5and0p6", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drBetween0p6and0p7"] = new TH1F("softDropJetMass_drBetween0p6and0p7", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h_["softDropJetMass_drBetween0p7and0p8"] = new TH1F("softDropJetMass_drBetween0p7and0p8", ";fatJet SoftDropMass (GeV);a.u.", 80, 0, 160);
	h2_["DBT_pt"] = new TH2F("DBT_pt",";fatJet doubleBtagDiscriminator; higgs p_{T} (GeV)",400,-1,1,400,0,2000);
	h2_["DBT_drBB"] = new TH2F("DBT_drBB",";fatJet doubleBtagDiscriminator; dR(bb)",400,-1,1,400,0,2.5);
	
	h2_["softDropJetMass_dRbb"] = new TH2F("softDropJetMass_dRbb",";fatJet SoftDropMass (GeV); dR(bb)",400,0,160,400,0,2.0);
	h2_["softDropJetMass_minPtFrac"] = new TH2F("softDropJetMass_minPtFrac",";fatJet SoftDropMass (GeV); z_{min}",400,0,160,400,0,0.5);
	h2_["DBT_minPtFrac"] = new TH2F("DBT_minPtFrac",";fatJet doubleBtagDiscriminator; z_{min}",400,-1.0,1.0,400,0,0.5);

	h_["dRmatchNom"] = new TH1F("dRmatchNom", ";dR;a.u.", 50, 0, 1.50);

    // create the histograms (includes histograms with a fat jet mass cut)
	for (std::vector<std::string>::size_type iWP=0; iWP<doubleBtagWPnameD.size(); ++iWP){

		h2_[Form("ptScatter_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH2F(
			Form("ptScatter_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC Higgs p_{T} (GeV); doubleBTagJet p_{T} (GeV)",
			ptScatXBinning.size()-1, &(ptScatXBinning)[0], ptScatYBinning.size()-1, &(ptScatYBinning)[0]);

		h2_[Form("ptScatterSoftDrop_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH2F(
			Form("ptScatterSoftDrop_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC Higgs p_{T} (GeV); softDropFatJet p_{T} (GeV)",
			ptScatXBinning.size()-1, &(ptScatXBinning)[0], ptScatYBinning.size()-1, &(ptScatYBinning)[0]);

		h2_[Form("softDropJetMassVsBbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH2F(
			Form("softDropJetMassVsBbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC dR_bb; mass_softDropFatJet (GeV)",400,0,2.5,400,0,200);

		h2_[Form("softDropJetMassVsHiggsPt_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH2F(
			Form("softDropJetMassVsHiggsPt_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC Higgs p_{T} (GeV); mass_softDropFatJet (GeV)",400,0,2000,400,0,200);

		h2_[Form("softDropJetMassVsHiggsEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH2F(
			Form("softDropJetMassVsHiggsEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC Higgs #eta; mass_softDropFatJet (GeV)",400,-4,4,400,0,200);

		h2_[Form("fatJetMassVsBbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH2F(
			Form("fatJetMassVsBbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC dR_bb; mass_doubleBTagJet (GeV)",400,0,2.5,400,0,200);

		h2_[Form("fatJetMassVsHiggsPt_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH2F(
			Form("fatJetMassVsHiggsPt_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC Higgs p_{T} (GeV); mass_doubleBTagJet (GeV)",400,0,2000,400,0,200);

		h2_[Form("fatJetMassVsHiggsEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH2F(
			Form("fatJetMassVsHiggsEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC Higgs #eta; mass_doubleBTagJet (GeV)",400,-4,4,400,0,200);

		h_[Form("fatJetMass_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH1F(
		   Form("fatJetMass_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";mass_doubleBTagJet (GeV); a.u.", massBinning.size()-1, &(massBinning)[0]);

		h_[Form("fatJetEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH1F(
		   Form("fatJetEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";doubleBTagJet #eta; a.u.", etaDistBinning.size()-1, &(etaDistBinning)[0]);

		h_[Form("matchDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH1F(
		   Form("matchDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";dR(higgs,doubleBTagJet); a.u.", matchDeltaRDistBinning.size()-1, &(matchDeltaRDistBinning)[0]);

		h_[Form("bbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH1F(
		   Form("bbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";MC dR_bb; a.u.", bbDeltaRDistBinning.size()-1, &(bbDeltaRDistBinning)[0]);

		h_[Form("softDropJetEffDenominator_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH1F(
		   Form("softDropJetEffDenominator_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";doubleBTagJet p_{T} (GeV); totalCount", ptBinningSoftDrop.size()-1, &(ptBinningSoftDrop)[0]);

		h_[Form("softDropJetEffNumerator_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH1F(
		   Form("softDropJetEffNumerator_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";doubleBTagJet p_{T} (GeV); matchCount", ptBinningSoftDrop.size()-1, &(ptBinningSoftDrop)[0]);

		h_[Form("softDropJetMass_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH1F(
		   Form("softDropJetMass_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";mass_softDropFatJet (GeV); a.u.", massBinning.size()-1, &(massBinning)[0]);

		h_[Form("softDropJetDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())] = new TH1F(
		   Form("softDropJetDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str()), ";dR(doubleBTagJet,SoftDropFatJet); a.u.", 80, 0, 0.4);

   		for (std::vector<double>::size_type iEtaBin=0; iEtaBin<etaBinningD.size()-1; ++iEtaBin){

   			h_[Form("effNumerator_%sDoubleBTagWP_eta%.2f-%.2f", doubleBtagWPnameD[iWP].c_str(), etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] )] = new TH1F(
   			   Form("effNumerator_%sDoubleBTagWP_eta%.2f-%.2f", doubleBtagWPnameD[iWP].c_str(), etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] ),
   			   ";MC Higgs p_{T} (GeV); matchCount", ptBinning.size()-1, &(ptBinning)[0]);


   			if (iWP==0) // so we only create the denominators once
   			h_[Form("effDenominator_eta%.2f-%.2f", etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] )] = new TH1F(
   			   Form("effDenominator_eta%.2f-%.2f", etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] ),
   			   ";MC Higgs p_{T} (GeV); totalCount", ptBinning.size()-1, &(ptBinning)[0]); 

   			// same as above but as a function of dr rather than pt 
   			h_[Form("effNumerator_%sDoubleBTagWP_eta%.2f-%.2f_fcnDR", doubleBtagWPnameD[iWP].c_str(), etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] )] = new TH1F(
   			   Form("effNumerator_%sDoubleBTagWP_eta%.2f-%.2f_fcnDR", doubleBtagWPnameD[iWP].c_str(), etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] ),
   			   ";MC dR_bb; matchCount", dREffBinning.size()-1, &(dREffBinning)[0]);

   			if (iWP==0) // so we only create the denominators once
   			h_[Form("effDenominator_eta%.2f-%.2f_fcnDR", etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] )] = new TH1F(
   			   Form("effDenominator_eta%.2f-%.2f_fcnDR", etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] ),
   			   ";MC dR_bb; totalCount", dREffBinning.size()-1, &(dREffBinning)[0]); 

   		} // closes loop through etaBins
	} // closes loop through Btag WPs 
} //closes the function 'CreateHistograms'







void FillHistograms(std::map<std::string,TH1F*> & h_, std::map<std::string,TH2F*> & h2_, bool isMatch, bool isJetMapping, pat::Jet fatJetMatchD, pat::Jet softDropFatJetMapD, reco::GenParticle higssBbGenParticleD, double dRbb, double minPtFracDouble, std::vector<double> debugBbPositions, std::vector<std::string> doubleBtagWPnameD, std::vector<double> doubleBtagWPD, std::vector<double> etaBinningD)
{

	// fill the efficiency denominators
	for (std::vector<double>::size_type iEtaBin=0; iEtaBin<etaBinningD.size()-1; ++iEtaBin){

		if( fabs(higssBbGenParticleD.eta()) >= etaBinningD[iEtaBin] && fabs(higssBbGenParticleD.eta()) < etaBinningD[iEtaBin+1]){
			h_[Form("effDenominator_eta%.2f-%.2f", etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] )]->Fill(higssBbGenParticleD.pt());
			h_[Form("effDenominator_eta%.2f-%.2f_fcnDR", etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] )]->Fill(dRbb);

		} // closes 'if' eta within the set bin
	}

	h_["DEBUG_higgsBbDRpreMatching"]->Fill(dRbb);

	// fill other histograms if there is a match
	if (isMatch){



		double dRmatchDUMMY = delR( delPhi( fatJetMatchD.phi(),higssBbGenParticleD.phi() ), delEta( fatJetMatchD.eta(),higssBbGenParticleD.eta() ) );
		
		h_["dRmatchNom"]->Fill(dRmatchDUMMY);

		if (isJetMapping && dRbb<0.8 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drLessThan0p8"]->Fill(softDropFatJetMapD.mass());
		if (isJetMapping && dRbb>0.8 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drMoreThan0p8"]->Fill(softDropFatJetMapD.mass());
		
		if (isJetMapping && dRbb>0.0 && dRbb<0.1 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drBetween0p0and0p1"]->Fill(softDropFatJetMapD.mass());
		if (isJetMapping && dRbb>0.1 && dRbb<0.2 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drBetween0p1and0p2"]->Fill(softDropFatJetMapD.mass());
		if (isJetMapping && dRbb>0.2 && dRbb<0.3 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drBetween0p2and0p3"]->Fill(softDropFatJetMapD.mass());
		if (isJetMapping && dRbb>0.3 && dRbb<0.4 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drBetween0p3and0p4"]->Fill(softDropFatJetMapD.mass());
		if (isJetMapping && dRbb>0.4 && dRbb<0.5 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drBetween0p4and0p5"]->Fill(softDropFatJetMapD.mass());
		if (isJetMapping && dRbb>0.5 && dRbb<0.6 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drBetween0p5and0p6"]->Fill(softDropFatJetMapD.mass());
		if (isJetMapping && dRbb>0.6 && dRbb<0.7 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drBetween0p6and0p7"]->Fill(softDropFatJetMapD.mass());
		if (isJetMapping && dRbb>0.7 && dRbb<0.8 && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h_["softDropJetMass_drBetween0p7and0p8"]->Fill(softDropFatJetMapD.mass());
		
		if (isJetMapping && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h2_["softDropJetMass_dRbb"]->Fill(softDropFatJetMapD.mass(), dRbb);
		if (isJetMapping && dRmatchDUMMY<0.3 && dRbb<0.8 && softDropFatJetMapD.pt()>170.0) h2_["softDropJetMass_minPtFrac"]->Fill(softDropFatJetMapD.mass(), minPtFracDouble);
		if (isJetMapping && dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h2_["DBT_minPtFrac"]->Fill(fatJetMatchD.bDiscriminator("pfBoostedDoubleSecondaryVertexAK8BJetTags"), minPtFracDouble);
		
		if (dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h2_["DBT_pt"]->Fill(fatJetMatchD.bDiscriminator("pfBoostedDoubleSecondaryVertexAK8BJetTags"), higssBbGenParticleD.pt());
		if (dRmatchDUMMY<0.3 && softDropFatJetMapD.pt()>170.0) h2_["DBT_drBB"]->Fill(fatJetMatchD.bDiscriminator("pfBoostedDoubleSecondaryVertexAK8BJetTags"), dRbb);

		for (std::vector<std::string>::size_type iWP=0; iWP<doubleBtagWPnameD.size(); ++iWP){
			if (fatJetMatchD.bDiscriminator("pfBoostedDoubleSecondaryVertexAK8BJetTags") > doubleBtagWPD[iWP]){

				h2_[Form("ptScatter_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(higssBbGenParticleD.pt(), fatJetMatchD.pt());

				h_[Form("fatJetMass_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(fatJetMatchD.mass());
				h_[Form("fatJetEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(fatJetMatchD.eta());
				h_[Form("bbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(dRbb);
				double dRmatch = delR( delPhi( fatJetMatchD.phi(),higssBbGenParticleD.phi() ), delEta( fatJetMatchD.eta(),higssBbGenParticleD.eta() ) );
				h_[Form("matchDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(dRmatch);				
				h_[Form("softDropJetEffDenominator_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(fatJetMatchD.pt());
				h2_[Form("fatJetMassVsBbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(dRbb,fatJetMatchD.mass());
				h2_[Form("fatJetMassVsHiggsPt_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(higssBbGenParticleD.pt(),fatJetMatchD.mass());
				h2_[Form("fatJetMassVsHiggsEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(higssBbGenParticleD.eta(),fatJetMatchD.mass());

				if (isJetMapping){
					h2_[Form("ptScatterSoftDrop_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(higssBbGenParticleD.pt(), softDropFatJetMapD.pt());
					h_[Form("softDropJetEffNumerator_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(fatJetMatchD.pt());
					h_[Form("softDropJetMass_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(softDropFatJetMapD.mass());
					double dRmatchJet = delR( delPhi( fatJetMatchD.phi(),softDropFatJetMapD.phi() ), delEta( fatJetMatchD.eta(),softDropFatJetMapD.eta() ) );
					h_[Form("softDropJetDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(dRmatchJet);
					h2_[Form("softDropJetMassVsBbDeltaR_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(dRbb,softDropFatJetMapD.mass());
					h2_[Form("softDropJetMassVsHiggsPt_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(higssBbGenParticleD.pt(),softDropFatJetMapD.mass());
					h2_[Form("softDropJetMassVsHiggsEta_%sDoubleBTagWP", doubleBtagWPnameD[iWP].c_str())]->Fill(higssBbGenParticleD.eta(),softDropFatJetMapD.mass());

				} // closes 'if' there is a softDropJet map

		   		for (std::vector<double>::size_type iEtaBin=0; iEtaBin<etaBinningD.size()-1; ++iEtaBin){

		   			if( fabs(higssBbGenParticleD.eta()) >= etaBinningD[iEtaBin] && fabs(higssBbGenParticleD.eta()) < etaBinningD[iEtaBin+1]){ //this is where the eta binning is used
						h_[Form("effNumerator_%sDoubleBTagWP_eta%.2f-%.2f", doubleBtagWPnameD[iWP].c_str(), etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] )]->Fill(higssBbGenParticleD.pt());
						h_[Form("effNumerator_%sDoubleBTagWP_eta%.2f-%.2f_fcnDR", doubleBtagWPnameD[iWP].c_str(), etaBinningD[iEtaBin], etaBinningD[iEtaBin+1] )]->Fill(dRbb);
					} // closes 'if' the eta within the set bin
				} // closes loop through etaBins

	   		} // closes 'if' Btag discriminator greater than WP 
		} // closes loop through Btag WPs
	} // closes 'if' there is a matching fatJet

} // closes the function 'FillHistograms'







void WriteHistograms(std::map<std::string,TH1F*> & h_, std::map<std::string,TH2F*> & h2_, std::string outputFileD)
{
	TFile * outFile = new TFile(outputFileD.c_str(),"RECREATE");
	for ( auto & h : h_ ){
		h.second->Write();
	}
	for ( auto & h : h2_ ){
		h.second->Write();
	}
	outFile -> Close();
	delete outFile;
}







void WriteHistogramsDICE(std::map<std::string,TH1F*> & h_, std::map<std::string,TH2F*> & h2_, std::string outputFileD)
{
	TFile outFile(outputFileD.c_str(),"RECREATE");
	for ( auto & h : h_ ){
		h.second->Write();
	}
	for ( auto & h : h2_ ){
		h.second->Write();
	}
	outFile.Close();
	std::system("cp	*.root	../../.	"); // get the output back on soolin
}





double minPtFrac(const double& ptA, const double& ptB){

	if (ptA > ptB) return ptB / (ptA + ptB);
	if (ptB > ptA) return ptA / (ptA + ptB);
	else return 99.99;
}





std::vector<reco::GenParticle> higgsBbGenParticles(edm::Handle<std::vector<reco::GenParticle>> genParticles, std::vector<double> & dRbbVec, std::vector<double> & minPtFracVec, std::vector<std::vector<double>> & debugBbPositionsVec)
{
	// this is a vector containing the h->bb higgs'
	std::vector<reco::GenParticle> hBbGenParticles;

	for (size_t iGen = 0; iGen < genParticles->size(); ++iGen){

		const reco::GenParticle & genParticle = (*genParticles)[iGen];

			if (genParticle.pdgId()==35){ // particle is a higgs						
				if (genParticle.numberOfDaughters()==2 && abs(genParticle.daughter(0)->pdgId())==5 && abs(genParticle.daughter(1)->pdgId())==5){ // higgs decays to two b-quarks

					hBbGenParticles.push_back(genParticle);
					dRbbVec.push_back( delR( delPhi( genParticle.daughter(0)->phi(),genParticle.daughter(1)->phi() ), delEta( genParticle.daughter(0)->eta(),genParticle.daughter(1)->eta() ) ) );
					
					minPtFracVec.push_back( minPtFrac(genParticle.daughter(0)->pt(), genParticle.daughter(1)->pt()) );

					std::vector<double> temp = {genParticle.daughter(0)->phi(), genParticle.daughter(0)->eta(), genParticle.daughter(1)->phi(), genParticle.daughter(1)->eta()};
					debugBbPositionsVec.push_back(temp);
				} // closes 'if' higgs decays to two b-quarks			
			} // closes 'if' genParticle is a higgs
		} // closes loop through genParticles
	return hBbGenParticles;
}	






bool isThereAFatJetMatch(edm::Handle<std::vector<pat::Jet>> fatJetsD, reco::GenParticle higssBbGenParticleD, double dRMaxMatchD, pat::Jet & fatJetMatch)
{
	size_t closestFatJetIndex = 999;
	double dRMin = 9999.99;
							
	for (size_t iFJ = 0; iFJ < fatJetsD->size(); ++iFJ){
		const pat::Jet & fatJet = (*fatJetsD)[iFJ];

		double dR = delR( delPhi( fatJet.phi(),higssBbGenParticleD.phi() ), delEta( fatJet.eta(),higssBbGenParticleD.eta() ) );
		if (dR < dRMin){
			dRMin = dR;
			closestFatJetIndex = iFJ; 
		}
	} // closes loop through fatJets entries for the event 

	// if there is a fatJet that matches, update fatJetMatch with this object and return 'true' 
	if (dRMin < dRMaxMatchD){
		fatJetMatch = (*fatJetsD)[closestFatJetIndex]; 
		return true; 
	} 

	else return false;
} // closes the function 'isThereAFatJetMatch'






bool isThereSoftDropMapping(edm::Handle<std::vector<pat::Jet>> softDropFatJetsD, pat::Jet & fatJetMatchD, double dRMaxMatchJetsD, pat::Jet & softDropFatJetMap)
{
	size_t closestSoftDropFatJetIndex = 999;
	double dRMin = 9999.99;

	for (size_t iFJSD = 0; iFJSD < softDropFatJetsD->size(); ++iFJSD){
		const pat::Jet & softDropFatJet = (*softDropFatJetsD)[iFJSD];
		
		double dR = delR( delPhi( fatJetMatchD.phi(),softDropFatJet.phi() ), delEta( fatJetMatchD.eta(),softDropFatJet.eta() ) );
		if (dR < dRMin){
			dRMin = dR;
			closestSoftDropFatJetIndex = iFJSD;
		}
	} // closes loop through soft drop fatJets

	// if there is a softDropFatJet that matches, update softDropFatJetMap with this object and return 'true' 
	if (dRMin < dRMaxMatchJetsD){ 
		softDropFatJetMap = (*softDropFatJetsD)[closestSoftDropFatJetIndex];
		return true; 
	}

	else return false;
} // closes the function 'isThereSoftDropMapping'







std::string getOutputDirFromOutputFile(std::string outputFile)
{
	std::string forwardSlash = "/";
	std::string outputDirectory = outputFile;
	// strip the directory from the outputfile name
	for (size_t c = outputFile.size()-1; c > 0; --c){
		if (outputFile[c] == forwardSlash[0]){
			outputDirectory = outputFile.substr(0, c+1);
			break;
		}
	}
	return outputDirectory;
}
