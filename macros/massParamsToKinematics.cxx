#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include <TH1F.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TGraphAsymmErrors.h>
#include <TMultiGraph.h>
#include <TLegend.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TStyle.h>
#include <TROOT.h>
#include <TFile.h>
#include <TAxis.h>

// To use: 
// change the USER INPUT variables
// run with...
// $ root -l -b -q massParamsToKinematics.cxx
// (doesn't need compiling)


/*
We calculate kintematic properties of the following interaction:
pp -> squark + squark
looking at what happens to one arm of the squark
...where...
squark -> quark + NLSP;
NLSP -> LSP + higgs;
higgs -> bb;

interesting properties:

quark: energy/momentum, transverse momentum
NLSP: energy, momentum, transverse momentum

LSP:
in rest frame of NLSP - energy, momentum
in lab frame - energy, momentum, transverse momentum

Higgs:
in rest frame of NLSP - energy, momentum
in lab frame - energy, momentum, transverse momentum

Splitting angle of the bbar pair
*/

void createTex(std::vector<double>, std::vector<double>, std::string);
TStyle * TDRStyle();
void drawAndSave(TMultiGraph*, std::vector<TGraphAsymmErrors*>, TLegend*, std::string, std::string, double, double, double, double, double, double);
int SetColor(int, int);
void SetGraphOptions(TGraphAsymmErrors *, size_t, size_t, int);
void doCalculationsAndGraphs(double, double, double, double, std::vector<double>, std::string);


void massParamsToKinematics()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////
	// --- USER INPUT --- //
	////////////////////////

	// Setup the outputting
	std::string motherDir = "/users/jt15104/local_Analysis_boostedNmssmHiggs/output_massParamsToKinematics/v05_c4a4359_ZOOM/";

	// Setup the mass parameters
	std::vector<double> mass_squark_vec = {2000.0};
	std::vector<double> mass_higgs_vec = {70.0};
	double mass_ratio_beginPoint = 0.70; // mass_ratio = mass_higgs / mass_NLSP
	double mass_ratio_stepSize = 0.001; // interval in ratio between each calculation
	std::vector<double> mass_delta_vec = {1.0, 5.0, 10.0, 15.0, 20.0}; // mass_delta = mass_NLSP - mass_higgs - mass_LSP

	////////////////////////
	////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// make the output mother directory
	std::string forwardSlash = "/";
	if (motherDir[motherDir.size()-1] != forwardSlash[0]) motherDir = motherDir + "/";

	// bool makeDir = !(std::system(Form("mkdir %s",motherDir.c_str())));
	// if (makeDir == false){
	// 	std::cout << "The chosen output directory already exists, or the parent directory does not exist:" << std::endl;
	// 	std::cout << "Do not wish to overwrite files: Exiting..." << std::endl;
	// 	return;
	// }

	for (size_t i_squark=0; i_squark < mass_squark_vec.size(); ++i_squark){
		for (size_t i_higgs=0; i_higgs < mass_higgs_vec.size(); ++i_higgs){
		
			std::string outputDir = motherDir + Form("massSquark%.0f_massHiggs%.0f/", mass_squark_vec[i_squark], mass_higgs_vec[i_higgs]);
			std::system(Form("mkdir %s", outputDir.c_str()));
			doCalculationsAndGraphs(mass_squark_vec[i_squark], mass_higgs_vec[i_higgs], mass_ratio_beginPoint, mass_ratio_stepSize, mass_delta_vec, outputDir);

		} // closes loop through higgs entries
	} // closes loop through squark entries

	// create the .tex file
	createTex(mass_squark_vec, mass_higgs_vec, motherDir);

	// create educational pdfs
	// pt dist.
	TStyle * tdrStyle = TDRStyle();
	TCanvas* cPT = new TCanvas("cPT","cPT");
	TF1 * funcPT = new TF1("funcPT","x/sqrt(1-x*x)",0,0.9999999);
	funcPT->GetXaxis()->SetTitle("X");
	// funcPT->GetYaxis()->SetTitle("distribution");
	funcPT->SetTitle("");
	funcPT->SetLineColor(2);
	funcPT->SetLineWidth(2);
	funcPT->Draw();
	TLatex * latPT = new TLatex();
	latPT->SetNDC();
	latPT->SetTextFont(42);
	latPT->SetTextAlign(11); // align from left
	latPT->DrawLatex(0.20,0.80,"distribution of a particles p_{T}");
	latPT->DrawLatex(0.20,0.74,"with constant |p| and random direction");
	latPT->DrawLatex(0.50,0.62,"PDF = #frac{X}{#sqrt{1-X^{2}}}");
	latPT->DrawLatex(0.50,0.45,"X = #frac{p_{T}}{|p|}");	
	cPT->SaveAs(Form("%sexamplePtDist.pdf", motherDir.c_str()));
	cPT->Close();

	// dr dist.
	// TStyle * tdrStyle = TDRStyle();
	TCanvas* cDR = new TCanvas("cDR","cDR");
	double mass_squark_func = mass_squark_vec[int(mass_squark_vec.size()/2)];
	double pt_higgs_func = 5*mass_squark_func/12;
	double mass_higgs_func = mass_higgs_vec[int(mass_higgs_vec.size()/2)];
	double dRmin_func = 2 * mass_higgs_func / pt_higgs_func; 
	// TF1 * funcDRdum = new TF1("funcDR","0",0,2.0);
	// funcDRdum->Draw();
	TF1 * funcDR = new TF1("funcDR","(1/x)*(2*[0]/([1]*x))*(2*[0]/([1]*x))/sqrt(1-(2*[0]/([1]*x))*(2*[0]/([1]*x)))",dRmin_func,2.0);
	funcDR->SetParameter(0, mass_higgs_func);
	funcDR->SetParameter(1, pt_higgs_func);
	funcDR->GetXaxis()->SetTitle("dR bb");
	// funcDR->GetYaxis()->SetTitle("distribution");
	funcDR->SetTitle("");
	funcDR->SetLineColor(2);
	funcDR->SetLineWidth(2);
	funcDR->Draw();
	TLatex * latDR = new TLatex();
	latDR->SetNDC();
	latDR->SetTextFont(42);
	latDR->SetTextAlign(11); // align from left
	// latDR->DrawLatex(0.15,0.92,Form("Mass_Squark = %.0fGeV; Mass_Higgs =  %.0fGeV", mass_squark_func, mass_higgs_func));
	latDR->DrawLatex(0.20,0.80,"distribution in higgs->bb seperation, R");
	latDR->DrawLatex(0.20,0.74,"due to variation in higgs p_{T} for a given |p|");
	latDR->DrawLatex(0.50,0.62,"PDF = #frac{(R_{min}/R)^{2}}{R#sqrt{1-(R_{min}/R)^{2}}}");
	latDR->DrawLatex(0.50,0.45,"R_{min} = #frac{1.9m_{higgs}}{p_{higgs}}");	
	latDR->DrawLatex(0.15,0.92,"Described by 'Straight Line' Dists (R_{min} to R_{mean})");	
	cDR->SaveAs(Form("%sexampleDrDist.pdf", motherDir.c_str()));
	cDR->Close();

	// higgs |p| dist.
	TCanvas* cHP = new TCanvas("cHP","cHP");
	TF1 * funcHP = new TF1("funcHP","sin(x)",0,M_PI);
	funcHP->SetParameter(0, mass_higgs_func);
	funcHP->SetParameter(1, pt_higgs_func);
	funcHP->GetXaxis()->SetTitle("|p_{higgs}|");
	// funcHP->GetYaxis()->SetTitle("distribution");
	funcHP->SetTitle("");
	funcHP->SetLineColor(2);
	funcHP->SetLineWidth(2);
	funcHP->Draw();
	TLatex * latHP = new TLatex();
	latHP->SetNDC();
	latHP->SetTextFont(42);
	latHP->SetTextAlign(11); // align from left
	latHP->DrawLatex(0.30,0.30,"Note:");
	latHP->DrawLatex(0.30,0.25,"higgs |p| distribution is ~sinosoidal");
	latHP->DrawLatex(0.30,0.20,"between min and max");
	latHP->DrawLatex(0.15,0.92,"Described by 'Dotted' Dists (min to max)");
	cHP->SaveAs(Form("%sexampleHiggs3PDist.pdf", motherDir.c_str()));
	cHP->Close(); 

} // closes the function 'massParamsToKinematics'



void doCalculationsAndGraphs(double mass_squark, double mass_higgs, double mass_ratio_beginPoint, double mass_ratio_stepSize, std::vector<double> mass_delta_vec, std::string outputDir)
{
	TMultiGraph * mg_mass_NLSP = new TMultiGraph();
	TMultiGraph * mg_mass_LSP = new TMultiGraph();
	TMultiGraph * mg_energy_quark = new TMultiGraph();
	TMultiGraph * mg_momentum_quark = new TMultiGraph();
	TMultiGraph * mg_energy_NLSP = new TMultiGraph();
	TMultiGraph * mg_momentum_NLSP = new TMultiGraph();
	TMultiGraph * mg_energy_LSP = new TMultiGraph();
	TMultiGraph * mg_momentum_LSP = new TMultiGraph();
	TMultiGraph * mg_momentum_LSP_noError = new TMultiGraph();
	TMultiGraph * mg_energy_higgs = new TMultiGraph();
	TMultiGraph * mg_momentum_higgs = new TMultiGraph();
	TMultiGraph * mg_seperation_bb_min_different3p = new TMultiGraph();
	TMultiGraph * mg_seperation_bb_minToMean = new TMultiGraph();
	std::vector<TGraphAsymmErrors*> reverse_momentumHiggsEntry;
	std::vector<TGraphAsymmErrors*> reverse_momentumLSPEntry;
	std::vector<TGraphAsymmErrors*> reverse_momentumLSPEntry_noError;
	std::vector<TGraphAsymmErrors*> reverse_seperation_bb_min_different3p;
	TLegend * legend = new TLegend();
	legend->SetHeader("Mass Gap (#Delta_{M})");
	TCanvas* c=new TCanvas("c","c");

	for (size_t i_delta = 0; i_delta < mass_delta_vec.size(); ++i_delta){
		double mass_delta = mass_delta_vec[i_delta];

		// WHAT TO DO ABOUT TRANVERSE MOMENTUM???
		std::vector<double> mass_ratio_vec;
		std::vector<double> null_vec;
		std::vector<double> mass_NLSP_vec;
		std::vector<double> mass_LSP_vec;
		std::vector<double> energy_quark_vec;
		std::vector<double> momentum_quark_vec;
		std::vector<double> energy_NLSP_vec;
		std::vector<double> momentum_NLSP_vec;
		std::vector<double> energy_LSP_mean_vec;
		std::vector<double> energy_LSP_dev_vec;
		std::vector<double> momentum_LSP_mean_vec;
		std::vector<double> momentum_LSP_devUp_vec;
		std::vector<double> momentum_LSP_devDown_vec;
		std::vector<double> energy_higgs_mean_vec;
		std::vector<double> energy_higgs_dev_vec;
		std::vector<double> momentum_higgs_mean_vec;
		std::vector<double> momentum_higgs_devUp_vec;
		std::vector<double> momentum_higgs_devDown_vec;
		std::vector<double> seperation_bb_min_higgsPtMean_vec;
		std::vector<double> seperation_bb_meanMinDev_higgsPtMean_vec;
		std::vector<double> seperation_bb_minDev_higgsPtMax_vec;
		std::vector<double> seperation_bb_minDev_higgsPtMin_vec;

		for (double mass_ratio = mass_ratio_beginPoint; mass_ratio < mass_higgs / (mass_delta + mass_higgs); mass_ratio += mass_ratio_stepSize){  
			mass_ratio_vec.push_back(mass_ratio);
			null_vec.push_back(0);  
			
			//////////////////
			// ------------ // 
			// CALCULATIONS //
			// ------------ //
			//////////////////

			// calculate the LSP and NLSP masses
			double mass_NLSP = mass_higgs / mass_ratio;
			double mass_LSP = mass_NLSP - mass_higgs - mass_delta;

			// squark -> quark + NLSP
			double energy_quark = (mass_squark * mass_squark - mass_NLSP * mass_NLSP) / (2 * mass_squark);
			double momentum_quark = energy_quark;
			double energy_NLSP = mass_squark - energy_quark;
			double momentum_NLSP = momentum_quark;

			// NLSP -> LSP + higgs
			double boost_gamma = energy_NLSP / mass_NLSP;
			double boost_betaGamma = momentum_NLSP / mass_NLSP;

			// LSP: rest frame
			double energy_LSP_restFrame = (mass_NLSP * mass_NLSP + mass_LSP * mass_LSP - mass_higgs * mass_higgs) / (2 * mass_NLSP);
			double momentum_LSP_restFrame = sqrt(energy_LSP_restFrame * energy_LSP_restFrame - mass_LSP * mass_LSP);
			// higgs: rest frame
			double energy_higgs_restFrame = mass_NLSP - energy_LSP_restFrame;
			double momentum_higgs_restFrame = momentum_LSP_restFrame;
			// LSP: lab frame
			double energy_LSP_mean = boost_gamma * energy_LSP_restFrame;
			double energy_LSP_dev = boost_betaGamma * momentum_LSP_restFrame; // energy extremums can be +/- this value depending on boost angle
			double momentum_LSP_mean = sqrt(energy_LSP_mean * energy_LSP_mean - mass_LSP * mass_LSP);
			double momentum_LSP_devUp = sqrt( (energy_LSP_mean+energy_LSP_dev) * (energy_LSP_mean+energy_LSP_dev) - mass_LSP * mass_LSP ) - momentum_LSP_mean;
			double momentum_LSP_devDown = -1 * sqrt( (energy_LSP_mean-energy_LSP_dev) * (energy_LSP_mean-energy_LSP_dev) - mass_LSP * mass_LSP ) + momentum_LSP_mean;
			// higgs: lab frame
			double energy_higgs_mean = boost_gamma * energy_higgs_restFrame;
			double energy_higgs_dev = boost_betaGamma * momentum_higgs_restFrame; // energy extremums can be +/- this value depending on boost angle
			double momentum_higgs_mean = sqrt(energy_higgs_mean * energy_higgs_mean - mass_higgs * mass_higgs);
			double momentum_higgs_devUp = sqrt( (energy_higgs_mean+energy_higgs_dev) * (energy_higgs_mean+energy_higgs_dev) - mass_higgs * mass_higgs ) - momentum_higgs_mean;
			double momentum_higgs_devDown = -1 * sqrt( (energy_higgs_mean-energy_higgs_dev) * (energy_higgs_mean-energy_higgs_dev) - mass_higgs * mass_higgs ) + momentum_higgs_mean;

			// higgs -> bbar
			double seperation_bb_min_higgsPtMean = 1.9 * mass_higgs / momentum_higgs_mean;
			double seperation_bb_meanMinDev_higgsPtMean = ((M_PI/2) - 1) * seperation_bb_min_higgsPtMean;
			double seperation_bb_minDev_higgsPtMax = seperation_bb_min_higgsPtMean - 1.9 * mass_higgs / (momentum_higgs_mean+momentum_higgs_devUp); // smearing below
			double seperation_bb_minDev_higgsPtMin = 1.9 * mass_higgs / (momentum_higgs_mean-momentum_higgs_devDown) - seperation_bb_min_higgsPtMean; // smearing above
			//////////////////
			// -----END---- // 
			// CALCULATIONS //
			// ------------ //
			//////////////////

			mass_NLSP_vec.push_back(mass_NLSP);
			mass_LSP_vec.push_back(mass_LSP);
			energy_quark_vec.push_back(energy_quark);
			momentum_quark_vec.push_back(momentum_quark);
			energy_NLSP_vec.push_back(energy_NLSP);
			momentum_NLSP_vec.push_back(momentum_NLSP);
			energy_LSP_mean_vec.push_back(energy_LSP_mean);
			energy_LSP_dev_vec.push_back(energy_LSP_dev);
			momentum_LSP_mean_vec.push_back(momentum_LSP_mean);
			momentum_LSP_devUp_vec.push_back(momentum_LSP_devUp);
			momentum_LSP_devDown_vec.push_back(momentum_LSP_devDown);
			energy_higgs_mean_vec.push_back(energy_higgs_mean);
			energy_higgs_dev_vec.push_back(energy_higgs_dev);
			momentum_higgs_mean_vec.push_back(momentum_higgs_mean);
			momentum_higgs_devUp_vec.push_back(momentum_higgs_devUp);
			momentum_higgs_devDown_vec.push_back(momentum_higgs_devDown);
			seperation_bb_min_higgsPtMean_vec.push_back(seperation_bb_min_higgsPtMean);
			seperation_bb_meanMinDev_higgsPtMean_vec.push_back(seperation_bb_meanMinDev_higgsPtMean);
			seperation_bb_minDev_higgsPtMax_vec.push_back(seperation_bb_minDev_higgsPtMax);
			seperation_bb_minDev_higgsPtMin_vec.push_back(seperation_bb_minDev_higgsPtMin); 

		} // closes loop through the mass_ratio values

		TGraphAsymmErrors * gr_mass_NLSP = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(mass_NLSP_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]));
		TGraphAsymmErrors * gr_mass_LSP = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(mass_LSP_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]));
		TGraphAsymmErrors * gr_energy_quark = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(energy_quark_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]));
		TGraphAsymmErrors * gr_momentum_quark = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(momentum_quark_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]));
		TGraphAsymmErrors * gr_energy_NLSP = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(energy_NLSP_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]));
		TGraphAsymmErrors * gr_momentum_NLSP = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(momentum_NLSP_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]));
		TGraphAsymmErrors * gr_energy_LSP = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(energy_LSP_mean_vec[0]), &(null_vec[0]), &(null_vec[0]), &(energy_LSP_dev_vec[0]), &(energy_LSP_dev_vec[0]));
		
		TGraphAsymmErrors * gr_momentum_LSP = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(momentum_LSP_mean_vec[0]), &(null_vec[0]), &(null_vec[0]), &(momentum_LSP_devDown_vec[0]), &(momentum_LSP_devUp_vec[0]));
		TGraphAsymmErrors * gr_momentum_LSP_noError = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(momentum_LSP_mean_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]));
		

		TGraphAsymmErrors * gr_energy_higgs = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(energy_higgs_mean_vec[0]), &(null_vec[0]), &(null_vec[0]), &(energy_higgs_dev_vec[0]), &(energy_higgs_dev_vec[0]));
		TGraphAsymmErrors * gr_momentum_higgs = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(momentum_higgs_mean_vec[0]), &(null_vec[0]), &(null_vec[0]), &(momentum_higgs_devDown_vec[0]), &(momentum_higgs_devUp_vec[0]));
		TGraphAsymmErrors * gr_seperation_bb_min_different3p = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(seperation_bb_min_higgsPtMean_vec[0]), &(null_vec[0]), &(null_vec[0]), &(seperation_bb_minDev_higgsPtMax_vec[0]), &(seperation_bb_minDev_higgsPtMin_vec[0]));
		TGraphAsymmErrors * gr_seperation_bb_minToMean = new TGraphAsymmErrors(mass_ratio_vec.size(), &(mass_ratio_vec[0]), &(seperation_bb_min_higgsPtMean_vec[0]), &(null_vec[0]), &(null_vec[0]), &(null_vec[0]), &(seperation_bb_meanMinDev_higgsPtMean_vec[0]));

		SetGraphOptions(gr_mass_NLSP, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_mass_LSP, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_energy_quark, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_momentum_quark, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_energy_NLSP, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_momentum_NLSP, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_energy_LSP, i_delta, mass_delta_vec.size(), 3002);
		
		SetGraphOptions(gr_momentum_LSP, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_momentum_LSP_noError, i_delta, mass_delta_vec.size(), 3002);
		
		SetGraphOptions(gr_energy_higgs, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_momentum_higgs, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_seperation_bb_min_different3p, i_delta, mass_delta_vec.size(), 3002);
		SetGraphOptions(gr_seperation_bb_minToMean, i_delta, mass_delta_vec.size(), 3395); // 3007

		TH1F * h_dummy = new TH1F("h_dummy", "", 100, 0, 100);
		h_dummy->SetLineColor(SetColor(i_delta, mass_delta_vec.size()));
		h_dummy->SetLineWidth(3);
		legend->AddEntry(h_dummy, Form("%.1f GeV", mass_delta_vec[i_delta]), "L");
	    legend->SetLineColor(0);

		// add the TGraphs for this mass_delta bin to the TMultiGraphs
		mg_mass_NLSP->Add(gr_mass_NLSP);
		mg_mass_LSP->Add(gr_mass_LSP);
		mg_energy_quark->Add(gr_energy_quark);
		mg_momentum_quark->Add(gr_momentum_quark);
		mg_energy_NLSP->Add(gr_energy_NLSP);
		mg_momentum_NLSP->Add(gr_momentum_NLSP);
		mg_energy_LSP->Add(gr_energy_LSP);
		// mg_momentum_LSP->Add(gr_momentum_LSP);
		mg_energy_higgs->Add(gr_energy_higgs);
		// mg_momentum_higgs->Add(gr_momentum_higgs);
		// mg_seperation_bb_min_different3p->Add(gr_seperation_bb_min_different3p);
		mg_seperation_bb_minToMean->Add(gr_seperation_bb_minToMean);

		// hack to reverse the order of the higgs and lsp momentum entries into the multigraph 		
		reverse_momentumLSPEntry.push_back(gr_momentum_LSP);
		reverse_momentumLSPEntry_noError.push_back(gr_momentum_LSP_noError);

		reverse_momentumHiggsEntry.push_back(gr_momentum_higgs);
		reverse_seperation_bb_min_different3p.push_back(gr_seperation_bb_min_different3p);

	} // closes loop through the mass_delta entries

	// hack to reverse the order of the higgs and lsp momentum entries into the multigraph 
	for (size_t revCount = mass_delta_vec.size(); revCount > 0; --revCount){
		mg_momentum_LSP->Add(reverse_momentumLSPEntry[revCount-1]);
		// mg_momentum_LSP_noError->Add(reverse_momentumLSPEntry_noError[revCount-1]);
		mg_momentum_higgs->Add(reverse_momentumHiggsEntry[revCount-1]);
		mg_seperation_bb_min_different3p->Add(reverse_seperation_bb_min_different3p[revCount-1]);
	}

	// drawAndSave(mg_mass_NLSP, legend, Form("%sNLSP_mass.pdf", outputDir.c_str()), "mass_NLSP (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.65, 0.86);
	// drawAndSave(mg_mass_LSP, legend, Form("%sLSP_mass.pdf", outputDir.c_str()), "mass_LSP (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.65, 0.86);
	// // drawAndSave(mg_energy_quark, legend, Form("%squark_energy.pdf", outputDir.c_str()), "energy_quark (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.65, 0.86);
	// drawAndSave(mg_momentum_quark, legend, Form("%squark_momentum.pdf", outputDir.c_str()), "momentum_quark (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.25, 0.46);
	// drawAndSave(mg_energy_NLSP, legend, Form("%sNLSP_energy.pdf", outputDir.c_str()), "energy_NLSP (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.65, 0.86);
	// drawAndSave(mg_momentum_NLSP, legend, Form("%sNLSP_momentum.pdf", outputDir.c_str()), "momentum_NLSP (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.25, 0.46);
	// drawAndSave(mg_energy_LSP, legend, Form("%sLSP_energy.pdf", outputDir.c_str()), "energy_LSP (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.65, 0.86);
	drawAndSave(mg_momentum_LSP, reverse_momentumLSPEntry_noError, legend, Form("%sLSP_momentum.pdf", outputDir.c_str()), "LSP momentum (GeV)", mass_squark, mass_higgs, 0.67, 0.88, 0.58, 0.86);
	// drawAndSave(mg_energy_higgs, legend, Form("%shiggs_energy.pdf", outputDir.c_str()), "energy_higgs (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.65, 0.86);
	// drawAndSave(mg_momentum_higgs, legend, Form("%shiggs_momentum.pdf", outputDir.c_str()), "momentum_higgs (GeV)", mass_squark, mass_higgs, 0.70, 0.88, 0.25, 0.46);
	// drawAndSave(mg_seperation_bb_min_different3p, legend, Form("%sseperation_bb_min_difference3p.pdf", outputDir.c_str()), "dR_{min/mode} bb", mass_squark, mass_higgs, 0.76, 0.88, 0.65, 0.86);
	// drawAndSave(mg_seperation_bb_minToMean, legend, Form("%sseperation_bb_minToMean.pdf", outputDir.c_str()), "dR bb", mass_squark, mass_higgs, 0.76, 0.88, 0.65, 0.86);
} // closes the function 'doCalculationsAndGraphs'






void drawAndSave(TMultiGraph * mg, std::vector<TGraphAsymmErrors*> mgNOERROR, TLegend * leg, std::string saveName, std::string yAxisTitle, double mass_squark, double mass_higgs, double legXmin, double legXmax, double legYmin, double legYmax)
{
	TStyle * tdrStyle = TDRStyle();
	TCanvas* cPlot=new TCanvas("cPlot","cPlot");
	mg->Draw("a3L");
	mg->GetXaxis()->SetTitle("R_{M}");
	mg->GetXaxis()->SetTitleSize(0.05);
	mg->GetYaxis()->SetTitle(yAxisTitle.c_str());
	mg->GetYaxis()->SetTitleSize(0.05);
	mg->GetYaxis()->SetTitleOffset(1.15);
	mg->Draw("a3L");

	for (auto i=0; i<mgNOERROR.size(); i++)
		mgNOERROR[i]->Draw("same L");
	// mgNOERROR->Draw("A L same");
	
	leg->SetX1NDC(legXmin);
	leg->SetX2NDC(legXmax);
	leg->SetY1NDC(legYmin);
	leg->SetY2NDC(legYmax);
	leg->Draw("same");
	TLatex * latex = new TLatex();
	latex->SetNDC();
	latex->SetTextFont(42);
	latex->SetTextAlign(11); // align from left
	latex->DrawLatex(0.15,0.93,Form("Momentum_NLSP = %.0f GeV; M_{H} =  %.0f GeV", 0.5*mass_squark, mass_higgs));
	if (yAxisTitle=="dR_{min} bb") latex->DrawLatex(0.30,0.83,"dist. due to different higgs |p|");
	if (yAxisTitle=="dR bb") latex->DrawLatex(0.30,0.83,"dist. due to higgs p_{T} variation");
	// mg->Draw("a3L");
	cPlot->SaveAs(saveName.c_str());
	cPlot->Close();
}





int SetColor(int posititon, int maxColors)
{	
	gStyle->SetPalette(55); // sets what sort of colours we will use (this number could be changed)
	// modifier is an offset in the colour spectrum
	double modifier = 0.00;
    double colorIndex;
    int colour(1);
    double fraction = (double)(posititon)/(double)(maxColors);
    if( posititon > maxColors || posititon < 0 || maxColors < 0 ) colour = 1;
    else
    {
        colorIndex = (fraction + modifier) * gStyle->GetNumberOfColors();
        colour = gStyle->GetColorPalette(colorIndex);
    }
    return colour;
}





void SetGraphOptions(TGraphAsymmErrors * gr, size_t index, size_t numberOfBins, int fillStyle){
	gr->SetLineWidth(3);
	gr->SetLineColor(SetColor(index, numberOfBins));
	gr->SetFillColorAlpha(SetColor(index, numberOfBins), 0.40);
	gStyle->SetHatchesLineWidth(2);
	// gr->SetFillStyle(3001);

}





void createTex(std::vector<double> mass_squark_vec, std::vector<double> mass_higgs_vec, std::string motherDir)
{
	// create .tex strings to use in presentations
	std::string tex_sep = "\\documentclass{beamer}\n";
	tex_sep += "\\begin{document}\n\n";
	std::string tex_LSP = "\\documentclass{beamer}\n";
	tex_LSP += "\\begin{document}\n\n";
	std::string tex_NLSP = "\\documentclass{beamer}\n";
	tex_NLSP += "\\begin{document}\n\n";
	std::string tex_higgs = "\\documentclass{beamer}\n";
	tex_higgs += "\\begin{document}\n\n";
	std::string tex_sepRev = "\\documentclass{beamer}\n";
	tex_sepRev += "\\begin{document}\n\n";
	std::string tex_LSPRev = "\\documentclass{beamer}\n";
	tex_LSPRev += "\\begin{document}\n\n";
	std::string tex_NLSPRev = "\\documentclass{beamer}\n";
	tex_NLSPRev += "\\begin{document}\n\n";
	std::string tex_higgsRev = "\\documentclass{beamer}\n";
	tex_higgsRev += "\\begin{document}\n\n";

	// add the intro pics
	tex_sep += "\\begin{frame}";
	tex_sep += "\\begin{figure}[!htbp]\n";
	tex_sep += "\\centering\n";
	tex_sep += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{exampleHiggs3PDist.pdf}\n";
	tex_sep += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{exampleDrDist.pdf}\n";
	tex_sep += "\\end{figure}\n";
	tex_sep += "\\end{frame}\n\n";

	tex_LSP += "\\begin{frame}";
	tex_LSP += "\\begin{figure}[!htbp]\n";
	tex_LSP += "\\centering\n";
	tex_LSP += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{examplePtDist.pdf}\n";
	tex_LSP += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{exampleHiggs3PDist.pdf}\n";
	tex_LSP += "\\end{figure}\n";
	tex_LSP += "\\end{frame}\n\n";

	tex_NLSP += "\\begin{frame}";
	tex_NLSP += "\\begin{figure}[!htbp]\n";
	tex_NLSP += "\\centering\n";
	tex_NLSP += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{examplePtDist.pdf}\n";
	tex_NLSP += "\\end{figure}\n";
	tex_NLSP += "\\end{frame}\n\n";

	tex_higgs += "\\begin{frame}";
	tex_higgs += "\\begin{figure}[!htbp]\n";
	tex_higgs += "\\centering\n";
	tex_higgs += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{exampleHiggs3PDist.pdf}\n";
	tex_higgs += "\\end{figure}\n";
	tex_higgs += "\\end{frame}\n\n";

	tex_sepRev += "\\begin{frame}";
	tex_sepRev += "\\begin{figure}[!htbp]\n";
	tex_sepRev += "\\centering\n";
	tex_sepRev += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{exampleHiggs3PDist.pdf}\n";
	tex_sepRev += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{exampleDrDist.pdf}\n";
	tex_sepRev += "\\end{figure}\n";
	tex_sepRev += "\\end{frame}\n\n";

	tex_LSPRev += "\\begin{frame}";
	tex_LSPRev += "\\begin{figure}[!htbp]\n";
	tex_LSPRev += "\\centering\n";
	tex_LSPRev += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{examplePtDist.pdf}\n";
	tex_LSPRev += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{exampleHiggs3PDist.pdf}\n";
	tex_LSPRev += "\\end{figure}\n";
	tex_LSPRev += "\\end{frame}\n\n";

	tex_NLSPRev += "\\begin{frame}";
	tex_NLSPRev += "\\begin{figure}[!htbp]\n";
	tex_NLSPRev += "\\centering\n";
	tex_NLSPRev += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{examplePtDist.pdf}\n";
	tex_NLSPRev += "\\end{figure}\n";
	tex_NLSPRev += "\\end{frame}\n\n";

	tex_higgsRev += "\\begin{frame}";
	tex_higgsRev += "\\begin{figure}[!htbp]\n";
	tex_higgsRev += "\\centering\n";
	tex_higgsRev += "\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{exampleHiggs3PDist.pdf}\n";
	tex_higgsRev += "\\end{figure}\n";
	tex_higgsRev += "\\end{frame}\n\n";

	for (size_t i_squark=0; i_squark < mass_squark_vec.size(); ++i_squark){
		for (size_t i_higgs=0; i_higgs < mass_higgs_vec.size(); ++i_higgs){
		
			// add images to .tex presentations
		    std::string dirForSlide = Form("massSquark%.0f_massHiggs%.0f/", mass_squark_vec[i_squark], mass_higgs_vec[i_higgs]);
		    
		    tex_sep += "\\begin{frame}";
		    tex_sep += "\\begin{figure}[!htbp]\n";
		    tex_sep += "\\centering\n";
	        // tex_sep += Form("\\footnotesize\n \\textbf{Mass_Squark = %.0fGeV; Mass_Higgs =  %.0fGeV} \\hspace{0.15\\textwidth}", mass_squark_vec[i_squark], mass_higgs_vec[i_higgs]);
		    tex_sep += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sseperation_bb_min_difference3p.pdf}\n", dirForSlide.c_str());
		    // tex_sep += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sseperation_bb_minToMean.pdf}\n", dirForSlide.c_str());
		    tex_sep += "\\end{figure}\n";
		    tex_sep += "\\end{frame}\n\n";

		    tex_LSP += "\\begin{frame}";
		    tex_LSP += "\\begin{figure}[!htbp]\n";
		    tex_LSP += "\\centering\n";
	        // tex_LSP += Form("\\footnotesize\n \\textbf{Mass_Squark = %.0fGeV; Mass_Higgs =  %.0fGeV} \\hspace{0.15\\textwidth}", mass_squark_vec[i_squark], mass_higgs_vec[i_higgs]);
		    tex_LSP += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sLSP_mass.pdf}\n", dirForSlide.c_str());
		    tex_LSP += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sLSP_momentum.pdf}\n", dirForSlide.c_str());
		    tex_LSP += "\\end{figure}\n";
		    tex_LSP += "\\end{frame}\n\n";

		    tex_NLSP += "\\begin{frame}";
		    tex_NLSP += "\\begin{figure}[!htbp]\n";
		    tex_NLSP += "\\centering\n";
	        // tex_NLSP += Form("\\footnotesize\n \\textbf{Mass_Squark = %.0fGeV; Mass_Higgs =  %.0fGeV} \\hspace{0.15\\textwidth}", mass_squark_vec[i_squark], mass_higgs_vec[i_higgs]);
		    tex_NLSP += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sNLSP_mass.pdf}\n", dirForSlide.c_str());
		    tex_NLSP += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sNLSP_momentum.pdf}\n", dirForSlide.c_str());
		    tex_NLSP += "\\end{figure}\n";
		    tex_NLSP += "\\end{frame}\n\n";

		    tex_higgs += "\\begin{frame}";
		    tex_higgs += "\\begin{figure}[!htbp]\n";
		    tex_higgs += "\\centering\n";
	        // tex_higgs += Form("\\footnotesize\n \\textbf{Mass_Squark = %.0fGeV; Mass_Higgs =  %.0fGeV} \\hspace{0.15\\textwidth}", mass_squark_vec[i_squark], mass_higgs_vec[i_higgs]);
		    tex_higgs += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%shiggs_momentum.pdf}\n", dirForSlide.c_str());
		    tex_higgs += "\\end{figure}\n";
		    tex_higgs += "\\end{frame}\n\n";

		} // closes loop through higgs entries
	} // closes loop through squark entries

	for (size_t i_higgs=0; i_higgs < mass_higgs_vec.size(); ++i_higgs){
		for (size_t i_squark=0; i_squark < mass_squark_vec.size(); ++i_squark){
		
			// add images to .tex presentations
		    std::string dirForSlide = Form("massSquark%.0f_massHiggs%.0f/", mass_squark_vec[i_squark], mass_higgs_vec[i_higgs]);
		    
		    tex_sepRev += "\\begin{frame}";
		    tex_sepRev += "\\begin{figure}[!htbp]\n";
		    tex_sepRev += "\\centering\n";
		    tex_sepRev += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sseperation_bb_min_difference3p.pdf}\n", dirForSlide.c_str());
		    tex_sepRev += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sseperation_bb_minToMean.pdf}\n", dirForSlide.c_str());
		    tex_sepRev += "\\end{figure}\n";
		    tex_sepRev += "\\end{frame}\n\n";

		    tex_LSPRev += "\\begin{frame}";
		    tex_LSPRev += "\\begin{figure}[!htbp]\n";
		    tex_LSPRev += "\\centering\n";
		    tex_LSPRev += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sLSP_mass.pdf}\n", dirForSlide.c_str());
		    tex_LSPRev += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sLSP_momentum.pdf}\n", dirForSlide.c_str());
		    tex_LSPRev += "\\end{figure}\n";
		    tex_LSPRev += "\\end{frame}\n\n";

		    tex_NLSPRev += "\\begin{frame}";
		    tex_NLSPRev += "\\begin{figure}[!htbp]\n";
		    tex_NLSPRev += "\\centering\n";
		    tex_NLSPRev += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sNLSP_mass.pdf}\n", dirForSlide.c_str());
		    tex_NLSPRev += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%sNLSP_momentum.pdf}\n", dirForSlide.c_str());
		    tex_NLSPRev += "\\end{figure}\n";
		    tex_NLSPRev += "\\end{frame}\n\n";

		    tex_higgsRev += "\\begin{frame}";
		    tex_higgsRev += "\\begin{figure}[!htbp]\n";
		    tex_higgsRev += "\\centering\n";
		    tex_higgsRev += Form("\\includegraphics[trim={0 0 0 0cm},clip,width=0.49\\textwidth]{%shiggs_momentum.pdf}\n", dirForSlide.c_str());
		    tex_higgsRev += "\\end{figure}\n";
		    tex_higgsRev += "\\end{frame}\n\n";

		} // closes loop through squark entries
	} // closes loop through higgs entries

	tex_sep += "\\end{document}";
	tex_LSP += "\\end{document}";
	tex_NLSP += "\\end{document}";
	tex_higgs += "\\end{document}";
	tex_sepRev += "\\end{document}";
	tex_LSPRev += "\\end{document}";
	tex_NLSPRev += "\\end{document}";
	tex_higgsRev += "\\end{document}";

	// write these strings to .tex files
	std::ofstream file_sep;
	file_sep.open(Form("%ssep.tex", motherDir.c_str()));
	file_sep << tex_sep;
	file_sep.close();

	std::ofstream file_LSP;
	file_LSP.open(Form("%sLSP.tex", motherDir.c_str()));
	file_LSP << tex_LSP;
	file_LSP.close();

	std::ofstream file_NLSP;
	file_NLSP.open(Form("%sNLSP.tex", motherDir.c_str()));
	file_NLSP << tex_NLSP;
	file_NLSP.close();

	std::ofstream file_higgs;
	file_higgs.open(Form("%shiggs.tex", motherDir.c_str()));
	file_higgs << tex_higgs;
	file_higgs.close();

	std::ofstream file_sepRev;
	file_sepRev.open(Form("%ssepRev.tex", motherDir.c_str()));
	file_sepRev << tex_sepRev;
	file_sepRev.close();

	std::ofstream file_LSPRev;
	file_LSPRev.open(Form("%sLSPRev.tex", motherDir.c_str()));
	file_LSPRev << tex_LSPRev;
	file_LSPRev.close();

	std::ofstream file_NLSPRev;
	file_NLSPRev.open(Form("%sNLSPRev.tex", motherDir.c_str()));
	file_NLSPRev << tex_NLSPRev;
	file_NLSPRev.close();

	std::ofstream file_higgsRev;
	file_higgsRev.open(Form("%shiggsRev.tex", motherDir.c_str()));
	file_higgsRev << tex_higgsRev;
	file_higgsRev.close();

}






TStyle * TDRStyle()
{
	TStyle * tdrStyle = new TStyle("tdrStyle","");
	gROOT->SetStyle("tdrStyle"); 
	//tdrStyle->SetPalette(palette);

	// For the canvas:
	tdrStyle->SetCanvasBorderMode(0);
	tdrStyle->SetCanvasColor(kWhite);
	tdrStyle->SetCanvasDefH(600); //Height of canvas
	tdrStyle->SetCanvasDefW(800); //Width of canvas
	tdrStyle->SetCanvasDefX(0);   //POsition on screen
	tdrStyle->SetCanvasDefY(0);

	// For the Pad:
	tdrStyle->SetPadBorderMode(0);
	// tdrStyle->SetPadBorderSize(Width_t size = 1);
	tdrStyle->SetPadColor(kWhite);
	tdrStyle->SetPadGridX(false);
	tdrStyle->SetPadGridY(false);
	tdrStyle->SetGridColor(0);
	tdrStyle->SetGridStyle(3);
	tdrStyle->SetGridWidth(1);
	tdrStyle->SetPadGridX(false);
	tdrStyle->SetPadGridY(false);

	// For the frame:
	tdrStyle->SetFrameBorderMode(0);
	tdrStyle->SetFrameBorderSize(1);
	tdrStyle->SetFrameFillColor(0);
	tdrStyle->SetFrameFillStyle(0);
	tdrStyle->SetFrameLineColor(1);
	tdrStyle->SetFrameLineStyle(1);
	tdrStyle->SetFrameLineWidth(1);

	// For the histo:
	// tdrStyle->SetHistFillColor(1);
	// tdrStyle->SetHistFillStyle(0);
	tdrStyle->SetHistLineColor(1);
	tdrStyle->SetHistLineStyle(0);
	tdrStyle->SetHistLineWidth(1);
	// tdrStyle->SetLegoInnerR(Float_t rad = 0.5);
	// tdrStyle->SetNumberContours(Int_t number = 20);

	tdrStyle->SetEndErrorSize(2);
	//  tdrStyle->SetErrorMarker(20);
	tdrStyle->SetErrorX(0.);

	tdrStyle->SetMarkerStyle(1); // EDITFROM 20

	//For the legend
	tdrStyle->SetLegendBorderSize(0);
	tdrStyle->SetLegendFillColor(0);
	tdrStyle->SetLegendFont(42); // EDITFROM 42
	tdrStyle->SetLegendTextSize(0.05);

	//For the fit/function:
	tdrStyle->SetOptFit(0);
	tdrStyle->SetFitFormat("5.4g");
	tdrStyle->SetFuncColor(2);
	tdrStyle->SetFuncStyle(1);
	tdrStyle->SetFuncWidth(1);

	//For the date:
	tdrStyle->SetOptDate(0);
	// tdrStyle->SetDateX(Float_t x = 0.01);
	// tdrStyle->SetDateY(Float_t y = 0.01);

	// For the statistics box:
	tdrStyle->SetOptFile(0);
	tdrStyle->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
	tdrStyle->SetStatColor(kWhite);
	tdrStyle->SetStatFont(42);
	tdrStyle->SetStatFontSize(0.025);
	tdrStyle->SetStatTextColor(1);
	tdrStyle->SetStatFormat("6.4g");
	tdrStyle->SetStatBorderSize(1);
	tdrStyle->SetStatH(0.1);
	tdrStyle->SetStatW(0.15);
	// tdrStyle->SetStatStyle(Style_t style = 1001);
	// tdrStyle->SetStatX(Float_t x = 0);
	// tdrStyle->SetStatY(Float_t y = 0);

	// Margins:
	//tdrStyle->SetPadTopMargin(0.05);
	tdrStyle->SetPadTopMargin(0.10);
	tdrStyle->SetPadBottomMargin(0.13);
	// tdrStyle->SetPadLeftMargin(0.16);
	tdrStyle->SetPadLeftMargin(0.14);
	// tdrStyle->SetPadRightMargin(0.02);
	tdrStyle->SetPadRightMargin(0.07);
	// tdrStyle->SetPadRightMargin(0.10); // only really want to be changing this for the scatters

	// For the Global title:
	// tdrStyle->SetOptTitle(1); //EDITFROM 0
	tdrStyle->SetTitleFont(42);
	tdrStyle->SetTitleColor(1);
	tdrStyle->SetTitleTextColor(1);
	tdrStyle->SetTitleFillColor(10);
	tdrStyle->SetTitleFontSize(0.05);
	// tdrStyle->SetTitleH(0); // Set the height of the title box
	// tdrStyle->SetTitleW(0); // Set the width of the title box
	// tdrStyle->SetTitleX(0); // Set the position of the title box
	// tdrStyle->SetTitleY(0.985); // Set the position of the title box
	// tdrStyle->SetTitleStyle(Style_t style = 1001);
	// tdrStyle->SetTitleBorderSize(2);

	// For the axis titles:
	tdrStyle->SetTitleColor(1, "XYZ");
	tdrStyle->SetTitleFont(42, "XYZ");
	tdrStyle->SetTitleSize(0.045, "XYZ");
	// tdrStyle->SetTitleXSize(Float_t size = 0.02); // Another way to set the size?
	// tdrStyle->SetTitleYSize(Float_t size = 0.02);
	tdrStyle->SetTitleXOffset(0.95);//EDITFROM 0.9
	tdrStyle->SetTitleYOffset(1.35);//EDITFROM 1.25
	// tdrStyle->SetTitleOffset(1.1, "Y"); // Another way to set the Offset

	// For the axis labels:
	tdrStyle->SetLabelColor(1, "XYZ");
	tdrStyle->SetLabelFont(42, "XYZ");
	tdrStyle->SetLabelOffset(0.007, "XYZ");
	// tdrStyle->SetLabelSize(0.05, "XYZ");
	tdrStyle->SetLabelSize(0.045, "XYZ");

	// For the axis:
	tdrStyle->SetAxisColor(1, "XYZ");
	tdrStyle->SetStripDecimals(kTRUE);
	tdrStyle->SetTickLength(0.03, "XYZ");
	tdrStyle->SetNdivisions(510, "XYZ");
	tdrStyle->SetPadTickX(1);  // To get tick marks on the opposite side of the frame
	tdrStyle->SetPadTickY(1);

	// Change for log plots:
	tdrStyle->SetOptLogx(0);
	tdrStyle->SetOptLogy(0);
	tdrStyle->SetOptLogz(0);

	// Postscript options:
	tdrStyle->SetPaperSize(20.,20.);
	// tdrStyle->SetLineScalePS(Float_t scale = 3);
	// tdrStyle->SetLineStyleString(Int_t i, const char* text);
	// tdrStyle->SetHeaderPS(const char* header);
	// tdrStyle->SetTitlePS(const char* pstitle);

	// tdrStyle->SetBarOffset(Float_t baroff = 0.5);
	// tdrStyle->SetBarWidth(Float_t barwidth = 0.5);
	// tdrStyle->SetPaintTextFormat(const char* format = "g");
	// tdrStyle->SetPalette(Int_t ncolors = 0, Int_t* colors = 0);
	// tdrStyle->SetTimeOffset(Double_t toffset);
	// tdrStyle->SetHistMinimumZero(kTRUE);

	//tdrStyle->cd();
	return tdrStyle;
}
