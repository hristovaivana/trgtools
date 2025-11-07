#ifndef TRGTOOLS_APPHELPER_CPP_
#define TRGTOOLS_APPHELPER_CPP_

#include "trgtools/AppHelper.hpp"


namespace dunedaq::trgtools
{

RootPlotter::RootPlotter() {
  //gStyle->SetCanvasPreferGL(kTRUE);
  gStyle->SetCanvasPreferGL();
  gStyle->SetPalette(kDeepSea);
  gStyle->SetPalette(kDarkRainbow);

  h1_tp->SetCanExtend(TH1::kAllAxes); 
  //h1_tp->GetXaxis()->SetCanExtend(kTRUE);
  //h1_tp->GetYaxis()->SetCanExtend(kTRUE);

  h2_tp->SetCanExtend(TH2::kAllAxes);

};


void RootPlotter::book(dunedaq::trgtools::AppHelper* ah) {

  mgrs.clear();
  cmgs.clear();
  csts.clear();
  hss.clear();
  m_canvas_titles.clear();

  
      
  TString port = TString(ah->m_helper->m_opts.port);
  //m_serv = std::make_unique<THttpServer>("http:"+port);

  m_tp_titles = {
    "Channel Number", "Start Time", "Samples Over Threshold", 
    "Samples To Peak", "ADC Peak", "ADC Integral"
  };
  m_tp_subs = {"_ch", "_ts", "_sot", "_peaks", "_peaka", "_sum"};
  //std::vector<TH1F*> h1s = {h1_ch, h1_tp, h1_tp, h1_tp, h1_tp, h1_tp};

  std::vector<TH2D*> h2s = {h2_tp, h2_tp, h2_tp};
  std::vector<TString> subs2 = {"_ch_vs_ts", "_ch_vs_sum", "_sum_vs_ts"};

  
  fmt::print("DETAIL book {} \n", ah->m_input_records_slices.size());
  for (const auto& it : ah->m_input_records_slices) {
    fmt::print("DETAIL record/slice {}: {}, {} \n", it.size(), it[0], it[1]);

    std::string runnum = std::to_string(m_current_runnum);
    std::string recnum = std::to_string(it[0]);
    std::string tsnum = std::to_string(it[1]);
    std::string folder = runnum + "_" + recnum + "_" + tsnum;
    fmt::print("DBG plot tps {}, {}, {}\n", runnum, recnum, folder);

    m_canvas_titles[recnum] = "Run Number " + runnum + ", Trigger Record " + recnum + ", Timeslice " + tsnum;  

    std::vector<TString> current_subs;
    std::vector<TString> current_subs2;
    for (auto& ss: m_tp_subs) { current_subs.push_back("_" + recnum + ss); }
    for (auto& ss: subs2) { current_subs2.push_back("_" + recnum + ss); }

    for (size_t idx = 0; idx < h2s.size(); idx++) {
    fmt::print("DETAIL subs2 {}: {} \n", idx, current_subs2[idx]);
      mgrs[recnum].push_back(new TMultiGraph());
      cmgs[recnum].push_back(new TCanvas("cmg"+current_subs2[idx], "title"));
    }

    for (size_t idx = 0; idx < m_tp_subs.size(); idx++) {
      fmt::print("DETAIL subs {}: {} \n", idx, current_subs[idx]);
      csts[recnum].push_back(new TCanvas("cst"+current_subs[idx], "stacked hists"));
      hss[recnum].push_back(new THStack("hs"+current_subs[idx], m_tp_titles[idx]));
    }

  }

}

void RootPlotter::fill_helper_2(TH1F* h1i, TH1F* h1, const tpg_type& type, const TString* info) {
  switch(type) {
  case RECORD:
    h1i->SetName(h1->GetName()+TString("_r")+info[0]);
    h1i->SetTitle(info[1]);
    h1i->SetLineColor(kBlack);
    h1i->SetFillColorAlpha(kBlack, 0.35);
    break;
  case SLICE:
    h1i->SetName(h1->GetName()+TString("_s")+info[0]);
    h1i->SetTitle(info[1]);
    h1i->SetLineColor(kRed);
    h1i->SetFillColorAlpha(kRed, 0.35);
    break;
  case TPGEMU:
    h1i->SetName(h1->GetName()+TString("_e")+info[0]);
    h1i->SetTitle(info[1]);
    h1i->SetLineColor(kBlue);
    h1i->SetFillColorAlpha(kBlue, 0.35);
    break;
  case TPGNAIVE:
    h1i->SetName(h1->GetName()+TString("_n")+info[0]);
    h1i->SetTitle(info[1]);
    h1i->SetLineColor(kMagenta);
    h1i->SetFillColorAlpha(kMagenta, 0.35);
    break;
  case TPGSIM:
    h1i->SetName(h1->GetName()+TString("_m")+info[0]);
    h1i->SetTitle(info[1]);
    h1i->SetLineColor(kRed);
    h1i->SetFillColorAlpha(kMagenta, 0.35);
    break;
  default: 
    ;;
  }


  
}
TGraph* RootPlotter::fill_helper_1(const tpg_type& type) {
  TGraph* gr = new TGraph();

  switch(type) {
  case RECORD:
    gr->SetMarkerStyle(25);
    gr->SetMarkerColor(kBlack);
    gr->SetMarkerSize(0.85);
    break;
  case SLICE:
    gr->SetMarkerStyle(24);
    gr->SetMarkerColor(kRed);
    gr->SetMarkerSize(0.55);
    break;
  case TPGEMU:
    gr->SetMarkerStyle(20);
    gr->SetMarkerColor(kBlack);
    gr->SetMarkerSize(0.10);
    break;
  case TPGNAIVE:
    gr->SetMarkerStyle(28);
    gr->SetMarkerColor(kMagenta);
    gr->SetMarkerSize(0.70);
    break;
  case TPGSIM:
    gr->SetMarkerStyle(28);
    gr->SetMarkerColor(kRed);
    gr->SetMarkerSize(0.70);
    break;  
  default:
    ;;
  }
  return gr;
}
void RootPlotter::fill(const std::vector<trgdataformats::TriggerPrimitive>& tp_buffer, const tpg_type& type) {
 
  std::vector<uint32_t> bad_channels = {293, 295, 955, 1482, 2657, 2705, 2727, 4242, 4243, 5471, 5797, 6089};

  std::string recnum = std::to_string(m_current_recnum);

  //TGraph* gr = new TGraph();
  TGraph* gr0 = fill_helper_1(type);
  TGraph* gr1 = fill_helper_1(type);
  TGraph* gr2 = fill_helper_1(type);
  Int_t tpid = 0;
  for (auto& tp: tp_buffer) {
    if (std::find(std::begin(bad_channels), std::end(bad_channels), tp.channel) != std::end(bad_channels)) {continue;}
    if (tp.samples_over_threshold > 256) continue;
    uint64_t ts = tp.time_start + tp.samples_to_peak * 32;  	    
    gr0->SetPoint(tpid, ts, tp.channel);
    gr1->SetPoint(tpid, tp.adc_integral, tp.channel);
    gr2->SetPoint(tpid, ts, tp.adc_integral); 
    tpid++;
  }
  mgrs[recnum][0]->Add(gr0);
  mgrs[recnum][1]->Add(gr1);
  mgrs[recnum][2]->Add(gr2);
  m_2d_ytitles = {"Channel Number", "Channel Number", "ADC Integral"};
  m_2d_xtitles = {"Timestamp", "ADC Integral", "Timestamp"};

  std::vector<TH1F*> h1s = {h1_ch, h1_tp, h1_tp, h1_tp, h1_tp, h1_tp};
  std::vector<TH1F*> his;
  for (size_t idx = 0; idx < csts[recnum].size(); idx++) {
    TH1F* h1i = new TH1F(*h1s[idx]);
    TString info[2] = {m_tp_subs[idx], m_canvas_titles[recnum]};
    fill_helper_2(h1i, h1s[idx], type, info);
    h1i->GetXaxis()->SetTitle(m_tp_titles[idx]);
    h1i->GetYaxis()->SetTitle("Number of Events");
    h1i->Print();
    h1s[idx]->Print();
    fmt::print("DBG info {}, {}, {}, {}\n", info[0], info[1], recnum, csts[recnum][idx]->GetName());
    his.push_back(h1i);
  };
  for (auto& tp: tp_buffer) {
    if (std::find(std::begin(bad_channels), std::end(bad_channels), tp.channel) != std::end(bad_channels)) {continue;}
    if (tp.samples_over_threshold > 256) continue;
    his[0]->Fill(tp.channel); 
    his[1]->Fill(tp.time_start);
    his[2]->Fill(tp.samples_over_threshold);
    his[3]->Fill(tp.samples_to_peak);
    his[4]->Fill(tp.adc_peak);
    his[5]->Fill(tp.adc_integral);
  }
  TString dir = "hists/"+TString(std::to_string(type))+"/"+recnum;
  m_f->mkdir(dir);
  m_f->cd(dir);
  for (size_t idx = 0; idx < his.size(); idx++) {
    hss[recnum][idx]->Add(his[idx]);
    //m_serv->Register("/hists/"+TString(std::to_string(type))+"/"+recnum, his[idx]);
    his[idx]->Write();
  }

}

void RootPlotter::show(dunedaq::trgtools::AppHelper* ah) {
  //THttpServer *serv = new THttpServer("http:8081");
  //TString port = TString(ah->m_helper->m_opts.port);
  ////m_serv = std::make_unique<THttpServer>("http:"+port);

  fmt::print("DETAIL loop {} \n", ah->m_input_records_slices.size());
  for (const auto& it : ah->m_input_records_slices) {
    fmt::print("DETAIL 123 {}: {}, {} \n", it.size(), it[0], it[1]);

    std::string runnum = std::to_string(m_current_runnum);
    std::string recnum = std::to_string(it[0]);
    TString folder = TString(runnum + "_" + std::to_string(it[0]) + "_" + std::to_string(it[1]));
    fmt::print("DBG plot tps {}, {}, {}\n", runnum, recnum, folder);
    //for (auto& ss: subs) { ss = "_" + recnum + ss; }
    //for (auto& ss: subs2) { ss = "_" + recnum + ss; }

    TString dir = "graphs/"+runnum+"/"+folder;
    m_f->mkdir(dir);
    m_f->cd(dir);
    int idx = 0;
    for (auto& cmg: cmgs[recnum]) {
      cmg->cd();
      cmg->SetSupportGL(true);
      mgrs[recnum][idx]->SetTitle(m_canvas_titles[recnum]);
      mgrs[recnum][idx]->Draw("ap");
      mgrs[recnum][idx]->GetXaxis()->SetTitle(m_2d_xtitles[idx]);
      mgrs[recnum][idx]->GetYaxis()->SetTitle(m_2d_ytitles[idx]);
      cmg->Modified();
      cmg->Update();
      cmg->Write();
      //m_serv->Register("/graphs/"+runnum+"/"+folder, cmg);
      idx++;
    }

  
    dir = "canvases/"+runnum+"/"+folder;
    m_f->mkdir(dir);
    m_f->cd(dir);
    idx = 0;
    for (auto& cst : csts[recnum]) {
      cst->cd();
      cst->SetSupportGL(true);
      gPad->SetGrid();
      hss[recnum][idx]->SetTitle(m_canvas_titles[recnum]);
      hss[recnum][idx]->Draw("nostack"); // custom colours 
      hss[recnum][idx]->GetXaxis()->SetTitle(m_tp_titles[idx]);
      hss[recnum][idx]->GetYaxis()->SetTitle("Number of Events");
      cst->Modified();
      cst->Update();
      cst->Write();
      //m_serv->Register("/canvases/"+runnum+"/"+folder, cst);
      idx++;
    }

  }

  //m_serv
  //gStopProcessing = false; 
  //while (!gSystem->ProcessEvents() && !gStopProcessing) {
  //}


}


void RootPlotter::plot_tps(const std::vector<trgdataformats::TriggerPrimitive>& tp_buffer, const std::string& type, bool pause) {
  // 3072 * 4 = 12284
  //int max_ch = 12284;
  //TH1F *h1_ch = new TH1F("h1_ch", "This is the channel distribution", max_ch, 0, max_ch);

  // folder label
  TString runnum = TString(std::to_string(m_current_runnum));
  TString recnum = TString(std::to_string(m_current_recnum));
  TString folder = TString(m_current_folder);



  // Create two histograms
   TH1F *hpx = new TH1F("hpx", "This is the px distribution", 100, -4, 4);
   TH2F *hpxpy = new TH2F("hpxpy", "py vs px", 40, -4, 4, 40, -4, 4);

   // http server with port 8080, use jobname as top-folder name
   THttpServer *serv = new THttpServer("http:8081");
   //THttpServer *serv = new THttpServer("http:8082");
   //if (!serv) serv = new THttpServer("http:8081");
   
   //serv->Hide("/Objects/");
   //serv->Hide("/Objects");
   //serv->SetTopName("ProtoDUNE2");

   // Fill histograms randomly
   TRandom3 random;
   Float_t px, py;

    // Set a timer to call TimerFunction after a delay
    //gSystem->AddTimer(30000, Timer); // Stops after 5 seconds

  TString c1_opt = "";
  //bool pause = false;
  //TH1* h1_ch_i = new TH1F(*h1_ch);
  //TH1* h1_ts_i = new TH1F(*h1_ts);

  std::vector<TString> titles = {
    "Channel Number", "Start Time", "Samples Over Threshold", 
    "Samples To Peak", "ADC Peak", "ADC Integral"
  };
  std::vector<TString> subs = {"_ch", "_ts", "_sot", "_peaks", "_peaka", "_sum"};
  std::vector<TH1F*> h1s = {h1_ch, h1_tp, h1_tp, h1_tp, h1_tp, h1_tp};

  std::vector<TH2D*> h2s = {h2_tp};
  std::vector<TString> subs2 = {"_ch_vs_ts"};

  fmt::print("DBG plot tps {}, {}, {}\n", runnum, recnum, folder);

  for (auto& ss: subs) { ss = "_" + recnum + ss; }
  for (auto& ss: subs2) { ss = "_" + recnum + ss; }

  if (type == "record") {
    mgrs[recnum].push_back(new TMultiGraph());
  }

  //std::vector<TString> subs = {"_ts"};
  //std::vector<TH1F*> h1s = {h1_ts};
  int idx=0;
  for (auto& h1 : h1s) {
    //THStack& hsi = new THStack(*hs);
    //hsi->SetName(hs->GetName()+subss[idx]);
    //hss.push_back(hsi);

    TH1* h1i = new TH1F(*h1);
    if (type == "record") {
      h1i->SetName(h1->GetName()+TString("_r")+subs[idx]);
      h1i->SetTitle(titles[idx]);
      h1i->SetLineColor(kBlack);
      h1i->SetFillColorAlpha(kBlack, 0.35);
      //h1i->SetFillColor(TColor::GetColor((Float_t) 0., 0., 1., 0.35));

      //csts.push_back(new TCanvas("cst"+subs[idx], "stacked hists"));
      //hss.push_back(new THStack("hs"+subs[idx], titles[idx]));
      csts[recnum].push_back(new TCanvas("cst"+subs[idx], "stacked hists"));
      hss[recnum].push_back(new THStack("hs"+subs[idx], titles[idx]));

      //TCanvas* ctmp = new TCanvas("c1"+subs[idx], "Overlay Histograms");;
      //crse.push_back(ctmp);
      //crse[idx]->cd(); h1i->Draw(c1_opt); crse[idx]->Modified(); crse[idx]->Update();
      //pause = true;
    } else if (type == "slice") {
      h1i->SetName(h1->GetName()+TString("_s")+subs[idx]);
      h1i->SetTitle(titles[idx]);
      h1i->SetLineColor(kRed);
      h1i->SetFillColorAlpha(kRed, 0.35);
      //h1i->SetFillColor(TColor::GetColor((Float_t) 1., 0., 0., 0.35));
        //c1_opt = "SAME";
        //pause = true;
        //crse[idx]->cd(); h1i->Draw(c1_opt); crse[idx]->Modified(); crse[idx]->Update();
    } else if (type == "tpgemu") {
      h1i->SetName(h1->GetName()+TString("_e")+subs[idx]);
      h1i->SetTitle(titles[idx]);
      h1i->SetLineColor(kBlue);
      h1i->SetFillColorAlpha(kBlue, 0.35);
      //h1i->SetFillColor(TColor::GetColor((Float_t) 1., 0., 0., 0.35));
        //c1_opt = "SAME";
        //crse[idx]->cd(); h1i->Draw(c1_opt); crse[idx]->Modified(); crse[idx]->Update();
        //pause = true;
    } else if (type == "tpgnaive") {
      h1i->SetName(h1->GetName()+TString("_n")+subs[idx]);
      h1i->SetTitle(titles[idx]);
      h1i->SetLineColor(kMagenta);
      h1i->SetFillColorAlpha(kMagenta, 0.35);
      //pause = true;
    } else if (type == "tpgsim") {
      h1i->SetName(h1->GetName()+TString("_m")+subs[idx]);
      h1i->SetTitle(titles[idx]);
      h1i->SetLineColor(kRed);
      h1i->SetFillColorAlpha(kMagenta, 0.35);
      //pause = true;
    }


    for (auto& tp: tp_buffer) {	
      if (idx==0) h1i->Fill(tp.channel);
      if (idx==1) h1i->Fill(tp.time_start);
      if (idx==2) h1i->Fill(tp.samples_over_threshold);
      if (idx==3) h1i->Fill(tp.samples_to_peak);
      if (idx==4) h1i->Fill(tp.adc_peak);
      if (idx==5) h1i->Fill(tp.adc_integral);
    }
    //hs->Add((TH1*)h1_ch->Clone());
    serv->Register("/hists/"+TString(type), h1i);
    //hss[idx]->Add(h1i);
    hss[recnum][idx]->Add(h1i);
    idx++;
  }


 

  idx=0;
  //fmt::print("DBG multi graph size {}: \n", mgrs.size());
  fmt::print("DBG multi graph size {}, {}: \n", mgrs.size(), mgrs[recnum].size());
  //for (auto& mgr : mgrs) {
  ///for (auto& mgr : mgrs[recnum]) {

    //TH2* h2i = new TH2D(*h2);
    TGraph* gr = new TGraph();
    if (type == "record") {
      gr->SetMarkerStyle(25);
      gr->SetMarkerColor(kBlack);
      gr->SetMarkerSize(0.85);
      //cmgs.push_back(new TCanvas("cmg"+subs2[idx], "title"));
      //mgrs[recnum].push_back(new TMultiGraph());
      cmgs[recnum].push_back(new TCanvas("cmg"+subs2[idx], "title"));
    } else if (type == "slice") {
      gr->SetMarkerStyle(24);
      gr->SetMarkerColor(kRed);
      gr->SetMarkerSize(0.55);
        //pause = true;
    } else if (type == "tpgemu") {
      gr->SetMarkerStyle(20);
      gr->SetMarkerColor(kBlack);
      gr->SetMarkerSize(0.10);
      //pause = true;
    } else if (type == "tpgnaive") {
      gr->SetMarkerStyle(28);
      gr->SetMarkerColor(kMagenta);
      gr->SetMarkerSize(0.70);
      //pause = true;
    } else if (type == "tpgsim") {
      gr->SetMarkerStyle(28);
      gr->SetMarkerColor(kRed);
      gr->SetMarkerSize(0.70);
      //pause = true;
    }

    if (idx==0) { 
      Int_t tpid = 0;
      for (auto& tp: tp_buffer) {
        uint64_t ts = tp.time_start + tp.samples_to_peak * 32;  	    
	gr->SetPoint(tpid, ts, tp.channel);
        tpid++;
      }
      mgrs[recnum][idx]->Add(gr);
    }
    //mgr->Add(gr);
  //  idx++;
  //}
  fmt::print("DBG multi graph canvas {}, {}: \n", cmgs.size(), cmgs[recnum].size());

  


  if (pause) {

  gStopProcessing = false; 
  fmt::print("DBG signal {}     {}, {}, {}\n", gStopProcessing, runnum, recnum, folder);
  signal(SIGINT, signal_callback_handler);
  // press Ctrl-C to stop macro
    c1->cd();
    random.Rannor(px,py);
    hpx->Fill(px);
    hpxpy->Fill(px,py);
      
    //c1->cd();
    //h1_ch_i->Draw(c1_opt);
    //c1->Modified();
    //c1->Update();
   
    
    idx=0;
    //for (auto& cst : csts) {
    for (auto& cst : csts[recnum]) {
      cst->cd();
      cst->SetSupportGL(true);
      gPad->SetGrid();
      //hs->Draw("nostack,e1p");
      //hss[idx]->Draw("nostack"); // custom colours 
      hss[recnum][idx]->Draw("nostack"); // custom colours 
      //hss[idx]->Draw("nostack PFC");
      //hss[idx]->Draw("nostack PLC");
      //hss[idx]->Draw("nostack PFC HIST");
      //hss[idx]->Draw("nostack PLC HIST"); // palette colours
      //hss[idx]->Draw("nostack HIST");
      //hss[idx]->Draw("nostack AF"); // transparent
      //hs->Draw("");
      cst->Modified();
      cst->Update();
      if (pause == true) serv->Register("/canvases/"+runnum+"/"+folder, cst);
      idx++;
    }
  

   
    idx=0;
    //for (auto& cmg: cmgs) {
    for (auto& cmg: cmgs[recnum]) {
      cmg->cd();
      cmg->SetSupportGL(true);
      //mgrs[idx]->Draw("ap");
      mgrs[recnum][idx]->Draw("ap");
      cmg->Modified();
      cmg->Update();
      if (pause == true) serv->Register("/graphs/"+runnum+"/"+folder, cmg);
      //idx++;
    }



    // test graphs
    /* 
    for (auto& gr : grs) {
      c1->cd();
      gr->Draw("ap");
      c1->Modified();
      c1->Update();
    }
    */

    // test plot
      /*
      c1->cd();
      h1_ch->Draw(c1_opt);
      c1->Modified();
      c1->Update();
      */

 
  //c1->SaveAs("out.png");
  //cst->Print("c"+TString(cst->GetName())+".png");


  TGraph* gr = new TGraph(10);
  gr->SetName("gr1"); 
  serv->Register("graphs/subfolder", gr);
  //serv->Register("/graphs/subfolder", gr);
  //serv->SetItemField("/Graphs/subfolder","_monitoring","100");
  //serv->SetItemField("/Graphs/subfolder","_layout","simple");
  //serv->SetItemField("/Graphs/subfolder","_drawitem","graph");
  //serv->SetItemField("/Graphs/subfolder","_drawopt","AL");


   //serv->ProcessRequests();
   //serv->SetTimer(0, kTRUE);



  //std::cout << "Press Enter to continue..." << std::endl;
  //std::cin.get();

  //serv->Hide("/Objects/");
  //serv->Unregister(c1);
  } // pause 


  while (pause && !gSystem->ProcessEvents() && !gStopProcessing) {
  }

}


AppHelper::AppHelper() {

  //m_rp = new RootPlotter();
  //m_te = std::make_unique<TPGEmulator>();

};
//AppHelper::~AppHelper() {
  //delete m_rh;
//}


void AppHelper::app_test_trts_info(const std::shared_ptr<hdf5libs::HDF5RawDataFile>& input_file) {
  std::vector<std::string> str{
	    "application_name"
	  , "closing_timestamp"
	  , "creation_timestamp"
	  , "record_type"
	  , "operational_environment"
	  , "offline_data_stream"
	  , "run_was_for_test_purposes"
	  , "filelayout_params"    // json
	  , "source_id_geo_id_map" // json
  };
  std::vector<std::string> num{
	    "file_index"
	  , "recorded_size"
	  , "filelayout_version"
	  , "run_number"
  };
  std::vector<std::string> attrs = input_file->get_attribute_names();
  for (const auto& a : attrs) {
    //fmt::print("INFO attr {} : \n", a);
    if (std::find(std::begin(str), std::end(str), a) != std::end(str)) {
      fmt::print("INFO attr {} : {}\n", a, input_file->get_attribute_if_exists<std::string>(a, "N/A"));
    }
    if (std::find(std::begin(num), std::end(num), a) != std::end(num)) {
      fmt::print("INFO attr {} : {}\n", a, input_file->get_attribute<size_t>(a));
    }
  }
}

std::string AppHelper::dts_to_datetime(uint64_t& ts, std::string s) {
  std::time_t sec = ts*16/1000000000;
  int ns = ts*16%1000000000;
  //std::cout << s << ns << " " << std::asctime(std::localtime(&sec));
  return s + std::to_string(ns) + " " + std::asctime(std::localtime(&sec));

}

trgdataformats::TriggerPrimitive AppHelper::convert_tp(auto& tp) {
  //std::cout << "DBG TP type: "<<  typeid(tp).name() << " vs " << typeid(trgdataformats::TriggerPrimitive).name() << "\n";
  TriggerPrimitive_v4 tp_v4 = std::any_cast<TriggerPrimitive_v4>(tp);
  trgdataformats::TriggerPrimitive tp_v5;
  tp_v5.channel = tp_v4.channel;
  tp_v5.time_start = tp_v4.time_start;
  tp_v5.samples_over_threshold = (tp_v4.time_over_threshold) / SAMPLE_TICK;
  tp_v5.samples_to_peak = (tp_v4.time_peak - tp_v4.time_start) / SAMPLE_TICK;
  tp_v5.adc_peak = tp_v4.adc_peak;
  tp_v5.adc_integral = tp_v4.adc_integral;  
  return tp_v5;
}

// trgdataformats::TriggerPrimitive
template <typename T>
void  AppHelper::get_tps(const std::vector<std::unique_ptr<dunedaq::daqdataformats::Fragment>>& fragments, const std::vector<uint64_t>& rw, const T& TriggerPrimitive) {

  //std::vector<trgdataformats::TriggerPrimitive> tp_buffer;	

  // can be reused for raw files too 
  int n_frags = 0;
  for (const auto& fragment : fragments) {
    daqdataformats::SourceID sid = fragment->get_element_id();
    if (std::find(m_valid_tp_sourceids.begin(), m_valid_tp_sourceids.end(), sid) == m_valid_tp_sourceids.end()) {
      continue;
    }

    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kTriggerPrimitive) {
      continue;
    } else {
      //if (n_frags > 0) break; // TEMP test
      BYTES_PER_TP = sizeof(T);

      uint64_t begin = fragment->get_header().window_begin;
      uint64_t end = fragment->get_header().window_end;
      uint64_t eid = fragment->get_element_id().id;
      auto seqnum = fragment->get_sequence_number();
      fmt::print("DETAIL -- window/duration/id [ {} : {} ] / {} / {}, {} \n", begin, end, end-begin, eid, seqnum);
      std::cout << dts_to_datetime(begin, std::string("(beg)."));
      std::cout << dts_to_datetime(end, std::string("(end)."));
      int num_tps = fragment->get_data_size() / BYTES_PER_TP;
      if (num_tps == 0) continue;
      fmt::print("DETAIL -- tp size {}\n", BYTES_PER_TP);
      fmt::print("DETAIL -- tps {}\n", num_tps);

      T* tp_array = static_cast<T*>(fragment->get_data());
      for(size_t tpid(0); tpid<num_tps; ++tpid) {
          //fmt::print("DETAIL -- this tp id {}\n", tpid);
          auto& tp = tp_array[tpid];
          //if v4 convert v4 to v5
	  //if (typeid(tp) == typeid(TriggerPrimitive_v4)) {
	  //  auto& tp_v5 = convert_tp(tp);
	  //}

      //for (int itp = 0; itp < num_tps; itp++) {
      //  auto tp = reinterpret_cast<dunedaq::trgdataformats::TriggerPrimitive*>(
      //    static_cast<char*>(fragment->get_data()) + itp * BYTES_PER_TP);

	if (tpid < 10) {
	    uint64_t ts = tp.time_start;  
            int channel = (int)tp.channel;
            int detid = (int)tp.detid;
	    std::cout << "TP INFO " << channel << " (" << detid << ") " << dts_to_datetime(ts, "(ts).");
        }
     
	
	if (rw.size() > 0) {
	  if (tp.time_start >= rw.at(0) && tp.time_start <= rw.at(1)) {
	    uint64_t ts = tp.time_start;
            //fmt::print("DETAIL -- FOUND TP in window {}\n", dts_to_datetime(ts, "(dbg)."));
	    m_slice_tp_buffer_any.push_back(tp);
            m_trigger_numbers.insert(fragment->get_trigger_number());
	  }
	} else {
	  m_record_tp_buffer_any.push_back(tp);
	}
	
      }
    }
    n_frags++;
  }

  if (m_record_tp_buffer_any.size() > 0) {
    if ((m_record_tp_buffer_any.front()).type() == typeid(TriggerPrimitive_v4)) {

    std::transform(m_record_tp_buffer_any.begin(), m_record_tp_buffer_any.end(), std::back_inserter(m_record_tp_buffer), [this](std::any val) { return this->convert_tp(val); });
    } else {
      std::transform(m_record_tp_buffer_any.begin(), m_record_tp_buffer_any.end(), std::back_inserter(m_record_tp_buffer), [](std::any val) { return std::any_cast<trgdataformats::TriggerPrimitive>(val); });
    }
    m_record_tp_buffer_any.clear();
  }

  fmt::print("DETAIL -- slice tp buffer size {}\n", m_slice_tp_buffer_any.size());
  if (m_slice_tp_buffer_any.size() > 0) { 
    if ((m_slice_tp_buffer_any.front()).type() == typeid(TriggerPrimitive_v4)) {

    std::transform(m_slice_tp_buffer_any.begin(), m_slice_tp_buffer_any.end(), std::back_inserter(m_slice_tp_buffer), [this](std::any val) { return this->convert_tp(val); });
    } else { 
    std::transform(m_slice_tp_buffer_any.begin(), m_slice_tp_buffer_any.end(), std::back_inserter(m_slice_tp_buffer), [](std::any val) { return std::any_cast<trgdataformats::TriggerPrimitive>(val); });
    }
    m_slice_tp_buffer_any.clear();
  }


  fmt::print("DETAIL -- number of TP fragments {}\n", n_frags);
  fmt::print("DETAIL -- TP record window {}\n", rw.size());
  fmt::print("DETAIL -- TP sizee {}\n", BYTES_PER_TP);
  fmt::print("DETAIL -- number of TPs found record {}\n", m_record_tp_buffer.size());
  fmt::print("DETAIL -- number of TPs found slice {}\n", m_slice_tp_buffer.size());
  fmt::print("DETAIL -- number of trn {}\n", m_trigger_numbers.size());
  for (const auto& trn: m_trigger_numbers) {
    fmt::print("DETAIL -- trigger number trn {}\n", trn);
  }

  fmt::print("DETAIL -- number of TPs found record ANY {}\n", m_record_tp_buffer_any.size());
  fmt::print("DETAIL -- number of TPs found slice ANY {}\n", m_slice_tp_buffer_any.size());

//std::vector<NewType> newVec; std::transform(oldVec.begin(), oldVec.end(), std::back_inserter(newVec), [](OldType val) { return static_cast<NewType>(val); });

}

void AppHelper::get_tps_wrapper(const std::vector<std::unique_ptr<dunedaq::daqdataformats::Fragment>>& fragments, const std::vector<uint64_t>& rw) {
if (m_helper->m_opts.tp_format == "v4") {
          TriggerPrimitive_v4 tp;
          get_tps(fragments, rw, tp);
        } else {
          trgdataformats::TriggerPrimitive tp;
          get_tps(fragments, rw, tp);
        }
}


//void AppHelper::get_tps(auto& fragments, TH1F *h1_ch, TString c1_opt) {
void AppHelper::get_tps_old(auto& fragments, const std::vector<uint64_t>& rw) {

  //std::vector<trgdataformats::TriggerPrimitive> tp_buffer;	

  // can be reused for raw files too 
  int n_frags = 0;
  std::set<uint64_t> trigger_numbers;
  for (const auto& fragment : fragments) {
    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kTriggerPrimitive) {
      continue;
    } else {
      if (n_frags > 0) break; // test
      BYTES_PER_TP = sizeof(trgdataformats::TriggerPrimitive);

      uint64_t begin = fragment->get_header().window_begin;
      uint64_t end = fragment->get_header().window_end;
      fmt::print("DETAIL -- window [ {} : {} ], {}\n", begin, end);
      int num_tps = fragment->get_data_size() / BYTES_PER_TP;
      if (num_tps == 0) continue;
      fmt::print("DETAIL -- tp size {}\n", BYTES_PER_TP);
      fmt::print("DETAIL -- tps {}\n", num_tps);

      trgdataformats::TriggerPrimitive* tp_array = static_cast<trgdataformats::TriggerPrimitive*>(fragment->get_data());
      for(size_t tpid(0); tpid<num_tps; ++tpid) {
          auto& tp = tp_array[tpid];

      //for (int itp = 0; itp < num_tps; itp++) {
      //  auto tp = reinterpret_cast<dunedaq::trgdataformats::TriggerPrimitive*>(
      //    static_cast<char*>(fragment->get_data()) + itp * BYTES_PER_TP);

	if (tpid < 10) {
	    uint64_t ts = tp.time_start;  
            int channel = (int)tp.channel;
	    std::cout << "TP INFO " << channel << ", " << dts_to_datetime(ts, "(ts).");
        }
      
	if (rw.size() > 0) {
	  if (tp.time_start >= rw.at(1) && tp.time_start <= rw.at(2)) {
	    uint64_t ts = tp.time_start;
            fmt::print("DETAIL -- FOUND TP in window {}\n", dts_to_datetime(ts, "(dbg)."));
	    m_slice_tp_buffer.push_back(tp);
            trigger_numbers.insert(fragment->get_trigger_number());
	  }
	} else {
	  m_record_tp_buffer.push_back(tp);
	}
      }
    }
    n_frags++;
  }
  fmt::print("DETAIL -- number of TP fragments {}\n", n_frags);
  fmt::print("DETAIL -- TP record window {}\n", rw.size());
  fmt::print("DETAIL -- number of TPs found record {}\n", m_record_tp_buffer.size());
  fmt::print("DETAIL -- number of TPs found slice {}\n", m_slice_tp_buffer.size());
  fmt::print("DETAIL -- number of trn {}\n", trigger_numbers.size());
  for(const auto& trn: trigger_numbers) {
    fmt::print("DETAIL -- trigger number trn {}\n", trn);
  }

}

void AppHelper::get_valid_tp_sourceids(auto& fragments, const std::string& type) {
  
  std::vector<daqdataformats::SourceID> ret;
  for (const auto& fragment : fragments) {
    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kTriggerPrimitive) {
      continue;
    }

    daqdataformats::SourceID sourceid = fragment->get_element_id();

    if (type == "Timeslice") {
      m_valid_tp_sourceids.push_back(sourceid);
    }
    ret.push_back(sourceid);
  }
  fmt::print("INFO {} sourceids: ", type);
  for (auto& it: ret) {
    //fmt::print("{}, {}, {}  ", it.id, it.subsystem, it.version);
    fmt::print("{} ", it.id);
  }
  fmt::print("\n");
}

void AppHelper::get_valid_sourceids(auto& fragments) {

  std::vector<daqdataformats::SourceID> ret;
  for (const auto& fragment : fragments) {
    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kTriggerPrimitive && fragment->get_fragment_type() != daqdataformats::FragmentType::kWIBEth) {
      continue;
    }

    daqdataformats::SourceID sourceid = fragment->get_element_id();

    ret.push_back(sourceid);
  }

  //uint64_t trn = record->get_header_ref().get_trigger_number();
  //m_record_sourceids[trn].reserve(m_record_sourceids[trn].size() + ret.size());
  //m_record_sourceids[trn].insert(m_record_sourceids[trn].end(), std::make_move_iterator(ret.begin()), std::make_move_iterator(ret.end()));

  fmt::print("INFO sourceids \n");
  for (auto& it: ret) {
    fmt::print("{}  ", it.id);
  }
  fmt::print("\n");
}
void AppHelper::app_test_trts(const std::shared_ptr<hdf5libs::HDF5RawDataFile>& input_file, bool a, bool v, bool vv) {

  if (vv) app_test_trts_info(input_file);

  auto records = input_file->get_all_record_ids();
  auto first_rec = *(records.begin());
  auto all_rh_paths = input_file->get_record_header_dataset_paths();

  int rec_first = 0, rec_last = 0, rec_second = 0, rec_gap = 1;
  if (input_file->is_trigger_record_type()) {
    auto trh_ptr = input_file->get_trh_ptr(first_rec);
    rec_first = trh_ptr->get_header().trigger_number;
    rec_last = input_file->get_trh_ptr(all_rh_paths.back())->get_header().trigger_number;
    // gap
    for (const auto& record : records) {
      rec_second++;
      if (rec_second == 2) {
        auto trh_ptr = input_file->get_trh_ptr(record);
        rec_second = trh_ptr->get_header().trigger_number;
        rec_gap = rec_second - rec_first;
        break;
      }
    }
  } else if (input_file->is_timeslice_type()) {
    auto tsh_ptr = input_file->get_tsh_ptr(first_rec);
    rec_first = (*tsh_ptr).timeslice_number;
    rec_last = (*(input_file->get_tsh_ptr(all_rh_paths.back()))).timeslice_number;
    // gap - if needed
  }

  size_t num_records = (rec_last - rec_first)/rec_gap + 1;
  if (v) {
    fmt::print("Number of records {} vs {}: [ {} : {} ], gap: {}\n", num_records, all_rh_paths.size(), rec_first, rec_last, rec_gap);
  }
  if (input_file->is_trigger_record_type()) {
    m_num_records += num_records;
  } else if (input_file->is_timeslice_type()) {
    m_num_slices += num_records;
  }


  if (a) {
    assert(num_records == all_rh_paths.size() && "checking number of records");
  }

}


void AppHelper::parse_app(CLI::App& _app, Options& _opts)
{
  _app.add_option("-i,--input-files", _opts.input_files, "List of input files (required)")
    ->required()
    ->check(CLI::ExistingFile); // Validate that each file exists

  _app.add_option("-j,--json-config", _opts.config_name, "TPGenerator config JSON to use (required)")
    ->required()
    ->check(CLI::ExistingFile);


  _app.add_option("-r,--input-records", _opts.input_records, "List of input trigger records to process");
  _app.add_option("-s,--input-slices", _opts.input_slices, "List of input time slices to process");
  _app.add_option("-f,--tp-format", _opts.tp_format, "Trigger Primitive format: either v4 or v5. Default: v5");

  _app.add_option("-e, --helper", _opts.helper, "Select helper, e.g. 0, 1, ...");
  _app.add_option("--ee", _opts.helpers, "Select helpers, e.g. 0 1 ...");
  _app.add_option("-p, --port", _opts.port, "Port number for THttpServer.");

  _app.add_flag("-a, --assert", _opts.assert, "Don't crash on assert failure.");
  _app.add_flag("-v, --verbose", _opts.verbose, "Don't print outputs.");
  _app.add_flag("--vv", _opts.verbose2, "Don't print more outputs.");
}  

void AppHelper::config_app(Options& _opts) {
 
  m_input_records = _opts.input_records;
  m_input_slices = _opts.input_slices;

  //m_rp = new RootPlotter();
  m_rp = std::make_unique<RootPlotter>();
  m_te = std::make_unique<TPGEmulator>();

  if (_opts.verbose) {

    fmt::print("{:-<77}\n", "");
    fmt::print("{}\n", _opts);
    //std::cout << _opts;

    fmt::print("{:-<77}\n", "");
    fmt::print("Input files to process:\n");
    for (const std::string& file : _opts.input_files) {
      fmt::print("- {}\n", file);
    }
    fmt::print("Number of input files: {}\n", _opts.input_files.size());
    fmt::print("{:-<77}\n", "");
  }

  for (auto& input_file : _opts.input_files) {
    m_input_files.push_back(std::make_shared<hdf5libs::HDF5RawDataFile>(input_file));
  }

  for (const auto& input_file : m_input_files) {
    // if -c --pedantic 
    //app_test_trts(input_file, _opts.assert, _opts.verbose, _opts.verbose2);
    if (input_file->is_trigger_record_type()) {
      m_input_files_raw.push_back(input_file);
    } else if (input_file->is_timeslice_type()) {
      m_input_files_tp.push_back(input_file);
    } else {
      fmt::print("ERROR File has unknown record type.\n"); 
    }
  }


  // sourceids
  auto records = m_input_files_raw.front()->get_all_record_ids();
  auto first_rec = *(records.begin());
  daqdataformats::TriggerRecord record = m_input_files_raw.front()->get_trigger_record(first_rec);
  auto& fragments = record.get_fragments_ref();
  get_valid_tp_sourceids(fragments, "Trigger Record");

  auto records_tp = m_input_files_tp.front()->get_all_record_ids();
  auto first_rec_tp = *(records_tp.begin());
  daqdataformats::TimeSlice timeslice = m_input_files_tp.front()->get_timeslice(first_rec_tp);
  auto& fragments_tp = timeslice.get_fragments_ref();
  get_valid_tp_sourceids(fragments_tp, "Timeslice");

  // json 
  // Get the configuration file
  std::ifstream config_stream(_opts.config_name);
  nlohmann::json config = nlohmann::json::parse(config_stream);
  m_te->configure(config);
  // Get the configuration file


  //helpers[0] = &AppHelper::helper_0;

  //helper_0();
  m_helper = std::make_unique<Helper>(_opts);

  make_tpg_configs();

  //(this->*helpers[_opts.helper])();
  for (const auto& h : _opts.helpers) {
    m_helper->start_message(h);
    (this->*helpers[h])();
  }
}
// -------------------------------------------------------------------------
void AppHelper::make_tpg_configs() {
  std::map<std::string, RootPlotter::tpg_type> type = {
    {"AVX", RootPlotter::tpg_type::TPGEMU}, 
    {"Naive", RootPlotter::tpg_type::TPGNAIVE}, 
    {"Sim", RootPlotter::tpg_type::TPGSIM}, 
  };

  std::ifstream config_stream(m_helper->m_opts.config_name);
  nlohmann::json config = nlohmann::json::parse(config_stream);
  //m_tes->configure(config);

  std::vector<std::string> tats = config["tpg_algorithm_types"];
  fmt::print("Using number of algorithm types: {}, {}\n", tats.size(), tats[0]);

  for (auto tat : tats) {	
    fmt::print("Using alg type: {}\n", tat);

    nlohmann::json tpg_config_json = config["tpg_config"][0];
    //std::vector<std::pair<std::string, nlohmann::json>> m_tpg_configs = tpg_config_json;
    nlohmann::json new_config = config;
    nlohmann::json j = new_config["tpg_config"][0];

    for (auto& it : tpg_config_json.items()) {
      auto itr = j.find(it.key()); // try catch this, handle case when key is not found
      std::string newKey = tat + it.key();
      std::swap(j[newKey], itr.value());
      j.erase(itr);
    }
    new_config["tpg_config"][0] = j;
    m_tpg_configs.push_back(new_config);

    fmt::print("DETAIL config key \n");
    for (auto& it : tpg_config_json.items()) {
      std::cout << it.key() << "\n";
    }
    fmt::print("DETAIL final key \n");
    for (auto& it : j.items()) {
      std::cout << it.key() << "\n";
    }

  }

}
// -------------------------------------------------------------------------
void AppHelper::helper_1_0(const auto& fragment, uint64_t& first_ts, uint64_t& last_ts, size_t& n_frames) {
  assert(fragment->get_fragment_type() == daqdataformats::FragmentType::kWIBEth && "WIBEth fragment check");

  n_frames = fragment->get_data_size()/SIZE_WIB_FRAME;
  //if (m_helper->m_opts.verbose) {
  //  fmt::print("  WIBEth fragment size: {}\n", fragment->get_data_size());
  //  fmt::print("  Num WIB frames: {}\n", n_frames);
  //}
  fddetdataformats::WIBEthFrame* wib_array = static_cast<fddetdataformats::WIBEthFrame*>(fragment->get_data());
  first_ts = wib_array[0].get_timestamp();
  last_ts = wib_array[n_frames-1].get_timestamp();
  uint64_t ts_diff = last_ts - first_ts;
  uint64_t last_ts_expected = first_ts + SAMPLE_TICK * TICKS_PER_FRAME * (n_frames - 1); 
  uint64_t ts_diff_expected = last_ts_expected - first_ts;
  //if (m_helper->m_opts.verbose) {
  //  fmt::print("INFO timestamp check: {} vs {}, diff = 0 vs {}\n", ts_diff, ts_diff_expected, ts_diff-ts_diff_expected);
  //}
  assert(ts_diff-ts_diff_expected == 0 && "timestamp check");
}
void AppHelper::helper_1_1(auto& fragments, uint64_t& first_ts, uint64_t& last_ts, size_t& n_frames) {
  if (m_helper->m_opts.verbose) fmt::print("DETAIL -- number of fragments {}\n", fragments.size());

  for (const auto& fragment : fragments) {
    if (fragment->get_fragment_type() != daqdataformats::FragmentType::kWIBEth) {
      continue;
    } else {
        helper_1_0(fragment, first_ts, last_ts, n_frames);
        break;
    }
  }
}
void AppHelper::helper_1_2(auto& record, uint64_t& first_ts, uint64_t& last_ts, uint64_t& n_frames) {

    last_ts += SAMPLE_TICK * TICKS_PER_FRAME;
    uint64_t last_ts_expected = first_ts + n_frames * SAMPLE_TICK * TICKS_PER_FRAME;
    if (m_helper->m_opts.verbose) {
      //fmt::print("INFO n_frags: {} vs {}\n", n_frames, fragments.size());
      fmt::print("INFO check last timestamp: {} vs {}\n", last_ts, last_ts_expected);
    }
    assert(last_ts == last_ts_expected && "quick timestamp check");


    uint64_t ts_window = last_ts - first_ts;
    if (m_helper->m_opts.verbose) {
      fmt::print("INFO trigger record window: {}\n", ts_window); 
      fmt::print("INFO first timestamp: {}\n", first_ts); 
      fmt::print("INFO last timestamp: {}\n", last_ts); 
    }
 
    std::vector<uint64_t> v2 = {first_ts, last_ts};
    m_record_window_map.insert(std::make_pair(record->get_header_ref().get_trigger_number(), v2));
    //m_record_window_map.insert(std::make_pair(record.get_header_ref().get_trigger_number(), v2));
    
    //std::vector<uint64_t> v = {record->get_header_ref().get_trigger_number(), first_ts, last_ts};
    //m_record_window.push_back(v);

    //uint64_t tt = record->get_header_ref().get_trigger_timestamp();
    //assert(tt >= first_ts && tt <= last_ts && "check record header timestamp");

}
// -------------------------------------------------------------------------
void AppHelper::helper_0_1_test() {
  fmt::print("DBG get input records {} \n", -1);
  for (auto& input_file: m_input_files_raw) {

    m_trigger_numbers.clear();
    m_rp->m_current_runnum = input_file->get_attribute<size_t>("run_number");

    auto records = input_file->get_all_record_ids();
    fmt::print("INFO records: {}\n", records.size());
    fmt::print("INFO records to process: {}\n", m_helper->m_opts.input_records.size());
    for (const auto& record : records) {
      //fmt::print("INFO record path {}\n", input_file->get_trigger_record_header_dataset_path(record));
      //for (auto t: input_file->get_fragment_dataset_paths(record)) {
      //  fmt::print("INFO record fragment path {}\n", t);
      //}

      //m_record_tp_buffer.clear();

      if (m_helper->m_opts.input_records.size() > 0) { 	    
        if (std::find(m_helper->m_opts.input_records.begin(), m_helper->m_opts.input_records.end(), record.first) == m_helper->m_opts.input_records.end()) {
          continue;
        }
      }

      if (m_helper->m_opts.verbose) fmt::print("DETAIL trigger record number {}\n", record.first);

      m_record_buffer.push_back(std::make_unique<daqdataformats::TriggerRecord>(input_file->get_trigger_record(record)));

      if (std::find(m_input_records.begin(), m_input_records.end(), record.first) == m_input_records.end()) {
        m_input_records.push_back(record.first);
      }

    }

  }

}
// -------------------------------------------------------------------------
void AppHelper::helper_0_test() {


  fmt::print("DBG input records/buffer {}/{} \n", m_input_records.size(), m_record_buffer.size());
  //if (m_input_records.size() == 0) {
    helper_0_1_test();
    //return;
  //}
  fmt::print("DBG input records/buffer {}/{} \n", m_input_records.size(), m_record_buffer.size());
  if (m_input_slices.size() == 0) {
    m_input_records.clear();
    helper_2();
  } else {
    for (std::size_t i = 0; i < std::min(m_input_records.size(), m_input_slices.size() ); i++) {
      m_input_records_slices.push_back({m_input_records[i], m_input_slices[i]});
    }
  }
  for (std::size_t i = 0; i < std::min(m_input_records.size(), m_input_slices.size() ); i++) {
    fmt::print("DBG record--slice {}--{} \n", m_input_records[i], m_input_slices[i]);
  }
  m_rp->book(this);

  for(auto iter = m_record_buffer.begin(); iter != m_record_buffer.end(); ) {    
    auto& record = *iter;
    uint64_t n = record->get_header_ref().get_trigger_number();
    if (std::find(std::begin(m_input_records), std::end(m_input_records), n) == std::end(m_input_records)) {
      iter = m_record_buffer.erase(iter);
    } else {
      ++iter;
    }
 
  }

  fmt::print("DBG records--buffer--slices {}--{}--{} \n", m_input_records.size(), m_record_buffer.size(), m_input_slices.size()); 

  //auto records = m_input_files.front()->get_all_record_ids();
  //std::vector<daqdataformats::TriggerRecord> record_buffer;
  //std::vector<std::unique_ptr<daqdataformats::TriggerRecord>> record_buffer;
  // eventually separate into m_input_files_raw and m_input_files_tp
  //for (auto& input_file: m_input_files) {
  //for (auto& input_file: m_input_files_raw) {

  //std::vector<std::unique_ptr<daqdataformats::TriggerRecord>> tmp;
  for (auto& record : m_record_buffer) {

    m_trigger_numbers.clear();

    //auto records = input_file->get_all_record_ids();
    //fmt::print("INFO records: {}\n", records.size());
    //fmt::print("INFO records to process: {}\n", m_helper->m_opts.input_records.size());
    //for (const auto& record : records) {
      //fmt::print("INFO record path {}\n", input_file->get_trigger_record_header_dataset_path(record));
      //for (auto t: input_file->get_fragment_dataset_paths(record)) {
      //  fmt::print("INFO record fragment path {}\n", t);
      //}

      m_record_tp_buffer.clear();

      //if (m_helper->m_opts.input_records.size() > 0) { 	    
      //  if (std::find(m_helper->m_opts.input_records.begin(), m_helper->m_opts.input_records.end(), record.first) == m_helper->m_opts.input_records.end()) {
      //    continue;
      //  }
      //}

      uint64_t trigger_number = record->get_header_ref().get_trigger_number();
      //uint64_t trigger_number = record.get_header_ref().get_trigger_number();
      //if (std::find(m_input_records.begin(), m_input_records.end(), trigger_number) == m_input_records.end()) { 
	//continue;
      //}

      //if (m_helper->m_opts.verbose) fmt::print("DETAIL trigger record number {}\n", record.first);

      //m_record_buffer.push_back(std::make_unique<daqdataformats::TriggerRecord>(input_file->get_trigger_record(record)));
 
      //std::cout << "TMP record first second " << record.first << ", " << record.second << "\n";
      //m_rp->m_current_recnum = record.get_header_data().trigger_number;
      //auto trh_ptr = input_file->get_trh_ptr(record);
      //rec_second = trh_ptr->get_header().trigger_number;


      //daqdataformats::TriggerRecord trigger_record = input_file->get_trigger_record(record);      
      auto& fragments = record->get_fragments_ref(); 
      //auto& fragments = record.get_fragments_ref(); 
      //get_tps_old(fragments, std::vector<uint64_t>{});
      //trgdataformats::TriggerPrimitive tp;
      //if (m_helper->m_opts.tp_format == "v4") {
      //  TriggerPrimitive_v4 tp;
      //  get_tps(fragments, std::vector<uint64_t>{}, tp);
      //} else {
      //  trgdataformats::TriggerPrimitive tp;
      //  get_tps(fragments, std::vector<uint64_t>{}, tp);
      //}
      get_tps_wrapper(fragments, std::vector<uint64_t>{});
      //m_rp->plot_tps(m_record_tp_buffer, "record");
      m_rp->m_current_recnum = trigger_number;
      fmt::print("DETAIL -- plot for record/tps {}/{}\n", m_rp->m_current_recnum, m_record_tp_buffer.size());
      m_rp->fill(m_record_tp_buffer, RootPlotter::tpg_type::RECORD);
      if (m_helper->m_opts.verbose) { 
        fmt::print("DETAIL -- number of fragments/tps {}/{}\n", fragments.size(), m_record_tp_buffer.size());
	//get_valid_sourceids(fragments);
      }

    //}
    //tmp.push_back(std::move(record));

  }

  //m_rp->show(this);
  //m_record_buffer.clear();
  //m_record_buffer.assign(tmp.begin(), tmp.end());

}

// -------------------------------------------------------------------------
void AppHelper::helper_0_1(auto records, auto input_file) {
    for (const auto& record : records) {
      //fmt::print("INFO record path {}\n", input_file->get_trigger_record_header_dataset_path(record));
      //for (auto t: input_file->get_fragment_dataset_paths(record)) {
      //  fmt::print("INFO record fragment path {}\n", t);
      //}

      m_record_tp_buffer.clear();

      if (m_helper->m_opts.input_records.size() > 0) { 	    
        if (std::find(m_helper->m_opts.input_records.begin(), m_helper->m_opts.input_records.end(), record.first) == m_helper->m_opts.input_records.end()) {
          continue;
        }
      }

      if (m_helper->m_opts.verbose) fmt::print("DETAIL trigger record number {}\n", record.first);

      m_record_buffer.push_back(std::make_unique<daqdataformats::TriggerRecord>(input_file->get_trigger_record(record)));

      if (std::find(m_input_records.begin(), m_input_records.end(), record.first) == m_input_records.end()) {
        m_input_records.push_back(record.first);
      }

    }

}

// -------------------------------------------------------------------------
void AppHelper::helper_0() {
  fmt::print("DBG get input records {} \n", -1);
  for (auto& input_file: m_input_files_raw) {

    m_trigger_numbers.clear();
    m_rp->m_current_runnum = input_file->get_attribute<size_t>("run_number");

    auto records = input_file->get_all_record_ids();
    fmt::print("INFO records: {}\n", records.size());
    fmt::print("INFO records to process: {}\n", m_helper->m_opts.input_records.size());
 
    //m_input_records.clear();
    //m_record_buffer.clear();
    //m_input_slices.clear();
    //m_input_records_slices.clear();

  m_record_buffer.clear();
  m_input_slices.clear();
  m_input_records_slices.clear();
  fmt::print("DBG input records/buffer/win/slices {}/{}/{}/{} \n", m_input_records.size(), m_record_buffer.size(), m_record_window_map.size(), m_input_slices.size());
  //if (m_input_records.size() == 0) {
    helper_0_1(records, input_file);
    //return;
  //}
  if (m_input_slices.size() == 0) {
    m_input_records.clear();
    helper_2();
  } else {
    for (std::size_t i = 0; i < std::min(m_input_records.size(), m_input_slices.size() ); i++) {
      m_input_records_slices.push_back({m_input_records[i], m_input_slices[i]});
    }
  }
  fmt::print("DBG input records/buffer/win/slices {}/{}/{}/{} \n", m_input_records.size(), m_record_buffer.size(), m_record_window_map.size(), m_input_slices.size());
  for (std::size_t i = 0; i < std::min(m_input_records.size(), m_input_slices.size() ); i++) {
    fmt::print("DBG record--slice {}--{} \n", m_input_records[i], m_input_slices[i]);
  }
  m_rp->book(this);

  for(auto iter = m_record_buffer.begin(); iter != m_record_buffer.end(); ) {    
    auto& record = *iter;
    uint64_t n = record->get_header_ref().get_trigger_number();
    if (std::find(std::begin(m_input_records), std::end(m_input_records), n) == std::end(m_input_records)) {
      iter = m_record_buffer.erase(iter);
      m_record_window_map.erase(n);
    } else {
      ++iter;
    } 
  }


  fmt::print("DBG records--buffer--slices {}--{}--{} \n", m_input_records.size(), m_record_buffer.size(), m_input_slices.size()); 

  for (auto& record : m_record_buffer) {

    m_trigger_numbers.clear();

      m_record_tp_buffer.clear();

      uint64_t trigger_number = record->get_header_ref().get_trigger_number();

      auto& fragments = record->get_fragments_ref(); 
      get_tps_wrapper(fragments, std::vector<uint64_t>{});
      m_rp->m_current_recnum = trigger_number;
      fmt::print("DETAIL -- plot for record/tps/type {}/{}/{}\n", m_rp->m_current_recnum, m_record_tp_buffer.size(), RootPlotter::tpg_type::RECORD);
      m_rp->fill(m_record_tp_buffer, RootPlotter::tpg_type::RECORD);
      if (m_helper->m_opts.verbose) { 
        fmt::print("DETAIL -- number of fragments/tps {}/{}\n", fragments.size(), m_record_tp_buffer.size());
      }

  }
  helper_9_tpgs();

  } // files

}



void AppHelper::helper_1() {
  // DETAILED TIMESTAMP CHECK - check each fragment separately 
  /* for each trigger record get
   * 1. start timestamp
   * 2. end timestamp
   * 3. check for missing timestamps
   */
  if (m_helper->m_opts.verbose) fmt::print("DETAIL number of records {}\n", m_record_buffer.size());
  for (const auto& record : m_record_buffer) {
    auto& fragments = record->get_fragments_ref();
    //auto& fragments = record.get_fragments_ref();
    int n_frags = fragments.size();
    size_t n_frames;
    if (m_helper->m_opts.verbose) fmt::print("DETAIL -- number of fragments {}\n", fragments.size());


    //const auto& first_fragment = fragments.front();
    //const auto& last_fragment = fragments.back();
    //helper_1_0(first_fragment);
    //helper_1_0(last_fragment);
    bool first = false;
    uint64_t first_ts;
    uint64_t last_ts;
    for (const auto& fragment : fragments) {
      if (fragment->get_fragment_type() != daqdataformats::FragmentType::kWIBEth) {
        continue;
      } else {
	if (!first) {
	  first = true;
	  helper_1_0(fragment, first_ts, last_ts, n_frames);
        }
	helper_1_0(fragment, first_ts, last_ts, n_frames);
      }
    }
    helper_1_2(record, first_ts, last_ts, n_frames);

  }

}



void AppHelper::helper_2() {
  // QUICK TIMESTAMP CHECK Don't check each fragment
  /* for each trigger record get
   * 1. start timestamp
   * 2. end timestamp
   * 3. check for missing timestamps
   * for each trigger record find the matching timeslice(s)
   */
  if (m_helper->m_opts.verbose) fmt::print("DETAIL number of records {}\n", m_record_buffer.size());
  m_record_window_map.clear();
  for (const auto& record : m_record_buffer) { 
    uint64_t first_ts;
    uint64_t last_ts;
    size_t n_frames;
    auto& fragments = record->get_fragments_ref();
    //auto& fragments = record.get_fragments_ref();
    helper_1_1(fragments, first_ts, last_ts, n_frames);


    std::ranges::reverse_view rv{fragments};
    helper_1_1(rv, first_ts, last_ts, n_frames);
    helper_1_2(record, first_ts, last_ts, n_frames);
  }
   
  // step 0
  fmt::print("DETAIL -- step0-0 tp files {}\n", m_input_files_tp.size());
  auto found = m_input_files_tp.begin();
  auto rec_begin = m_record_window_map.begin();
  for(auto iter = m_input_files_tp.begin(); iter != m_input_files_tp.end(); ) {
	
    auto& input_file = *iter;
    auto records = input_file->get_all_record_ids();

    daqdataformats::TimeSlice timeslice_first = input_file->get_timeslice(*(records.begin()));      
    daqdataformats::TimeSlice timeslice_last = input_file->get_timeslice(*(records.rbegin()));      
    uint64_t begin = timeslice_first.get_fragments_ref().front()->get_header().window_begin;
    uint64_t end = timeslice_last.get_fragments_ref().back()->get_header().window_end;
	   
    fmt::print("DETAIL -- step0-LOOK {}: {}-{}: {} \n", rec_begin->first, rec_begin->second[0], end, rec_begin->second[0] - end);
    if (rec_begin->second[0] > end) {
      //m_discard_files_tp.insert(input_file);
      //fmt::print("DETAIL -- step0-FOUND \n");
      //continue;
      iter = m_input_files_tp.erase(iter);
    } else {
      //++iter;
      break;
    }
    //++iter;
    //fmt::print("DETAIL -- step0-FOUND \n");
    //found = iter;
    //break;
  }
  //for(auto iter = m_input_files_tp.begin(); iter != found; ) {
  //  iter = m_input_files_tp.erase(iter);
  //}
  fmt::print("DETAIL -- step0-1 tp files {}\n", m_input_files_tp.size());


/*
  m_discard_files_tp.clear();
  fmt::print("DETAIL -- step0-0 tp files {}\n", m_input_files_tp.size());
  auto rec_begin = m_record_window_map.begin();
  for (auto& input_file: m_input_files_tp) { // assume time-pre-sorted 

    auto records = input_file->get_all_record_ids();

    daqdataformats::TimeSlice timeslice_first = input_file->get_timeslice(*(records.begin()));      
    daqdataformats::TimeSlice timeslice_last = input_file->get_timeslice(*(records.rbegin()));      
    uint64_t begin = timeslice_first.get_fragments_ref().front()->get_header().window_begin;
    uint64_t end = timeslice_last.get_fragments_ref().back()->get_header().window_end;
	   
    if (rec_begin->second[0] > end) {
      m_discard_files_tp.insert(input_file);
    }
  }
  fmt::print("DETAIL -- step0-1 tp files {}\n", m_discard_files_tp.size());
  m_input_files_tp.erase( remove_if( begin(m_input_files_tp),end(m_input_files_tp), [&](auto x){return find(begin(m_discard_files_tp),end(m_discard_files_tp),x)!=end(m_discard_files_tp);}), end(m_input_files_tp) );
  fmt::print("DETAIL -- step0-2 tp files {}\n", m_input_files_tp.size());
*/

  // step 1
  m_matched_files_tp.clear();
  fmt::print("DETAIL -- step1 tp files {}\n", m_input_files_tp.size());
  rec_begin = m_record_window_map.begin();
  auto rec_end = m_record_window_map.rbegin();
  for (auto& input_file: m_input_files_tp) { // assume time-pre-sorted 

    auto records = input_file->get_all_record_ids();

    daqdataformats::TimeSlice timeslice_first = input_file->get_timeslice(*(records.begin()));      
    daqdataformats::TimeSlice timeslice_last = input_file->get_timeslice(*(records.rbegin()));      
    uint64_t begin = timeslice_first.get_fragments_ref().front()->get_header().window_begin;
    uint64_t end = timeslice_last.get_fragments_ref().back()->get_header().window_end;
    
    if (begin > rec_end->second[0]) break;     

    for (auto it = rec_begin; it != m_record_window_map.end(); it++) {
      if (rec_begin->second[0] >= begin && rec_begin->second[1] <= end) {
        m_matched_files_tp.insert(input_file);
        rec_begin = it;
      }
    }
  }
  //m_input_files_tp.clear();

  //m_input_files_tp = std::vector<std::shared_ptr<hdf5libs::HDF5RawDataFile>>(m_matched_files_tp.begin(), m_matched_files_tp.end());
  //m_matched_files_tp.clear();
  fmt::print("DETAIL -- step1 tp files {}/{}\n", m_input_files_tp.size(), m_matched_files_tp.size());


  // step 2 
  fmt::print("DETAIL -- records {}\n", m_record_window_map.size());
  fmt::print("DETAIL -- num records {}\n", m_num_records);
  fmt::print("DETAIL -- num slices {}\n", m_num_slices);
  // resize by the found number of slices 
  // or convert filename time string to epoch and compare to skip files  
  m_slice_window_map.clear();
  for (auto& input_file: m_matched_files_tp) { // assume time-pre-sorted 

    auto records = input_file->get_all_record_ids();
    for (const auto& record : records) {

      daqdataformats::TimeSlice timeslice = input_file->get_timeslice(record);      
      auto& fragments = timeslice.get_fragments_ref();
      uint64_t begin = fragments.front()->get_header().window_begin;
      uint64_t end = fragments.front()->get_header().window_end;
	    
      std::vector<uint64_t> v2 = {begin, end};
      m_slice_window_map.insert(std::make_pair(record.first, v2));
      //m_slice_window_buffer.insert(std::make_pair(record, v2));
    }

  }

  //m_input_records_slices.clear();
  //m_input_records.clear();

  fmt::print("DETAIL -- slices {}\n", m_slice_window_map.size());
  fmt::print("DETAIL -- input slices {}\n", m_input_slices.size());
  if (m_input_slices.size() == 0) {
  for (auto& [rec, win]: m_record_window_map) {

    auto it_begin = m_slice_window_map.begin();
    auto findWithin = std::find_if(it_begin, m_slice_window_map.end(), [&win](const std::pair<uint64_t, std::vector<uint64_t>>& pair) {
      return win[0] >= pair.second[0] && win[1] <= pair.second[1];
    });
    if (findWithin != m_slice_window_map.end()) {
      fmt::print("INFO within  record -- slice {} -- {}\n", rec, findWithin->first); 
      m_input_slices.push_back(findWithin->first);
      m_input_records.push_back(rec);
      m_input_records_slices.push_back({rec, findWithin->first});
      m_input_slices_records.insert({findWithin->first, rec});
      it_begin = findWithin;
    }

    /*
    auto findBegin = std::find_if(m_slice_window_map.begin(), m_slice_window_map.end(), [&win](const std::pair<uint64_t, std::vector<uint64_t>>& pair) {
      return win[0] > pair.second[0] && win[0] < pair.second[1] && win[1] > pair.second[1];
    });
    if (findBegin != m_slice_window_map.end()) {
      fmt::print("INFO begin  record -- slice {} -- {}\n", rec, findBegin->first); 
      
    }

    auto findEnd = std::find_if(m_slice_window_map.begin(), m_slice_window_map.end(), [&win](const std::pair<uint64_t, std::vector<uint64_t>>& pair) {
      return win[0] < pair.second[0] && win[1] > pair.second[0] && win[1] < pair.second[1];
    });
    if (findEnd != m_slice_window_map.end()) {
      fmt::print("INFO end  record -- slice {} -- {}\n", rec, findEnd->first); 
    }
    */
  }
  }
  fmt::print("DETAIL -- input slices {}\n", m_input_slices.size());
  for (auto& it: m_input_slices) {
    fmt::print("{} ", it);
  }
  fmt::print("\n");




}


void AppHelper::helper_3() {

  for (auto& input_file: m_input_files_tp) {

    m_rp->m_current_runnum = input_file->get_attribute<size_t>("run_number");
    fmt::print("DETAIL -- current run {}, {}\n", m_rp->m_current_runnum, m_input_records_slices.size());

    auto records = input_file->get_all_record_ids();
    for (const auto& record : records) {

      m_trigger_numbers.clear();
      m_slice_tp_buffer.clear();

      if (m_input_records_slices.size() == 0) return; // speedup but cannot process slices only -- check -r option 

      if (m_helper->m_opts.input_slices.size() > 0) { 	    
        if (std::find(m_helper->m_opts.input_slices.begin(), m_helper->m_opts.input_slices.end(), record.first) == m_helper->m_opts.input_slices.end()) {
          continue;
        }
      } else {
        if (!m_input_slices_records.contains(record.first)) continue;	
      }

      if (m_helper->m_opts.verbose) fmt::print("DETAIL timeslice/record pair {}/{}\n", record.first, m_input_slices_records[record.first]);


 
      //m_slice_buffer.push_back(std::make_unique<daqdataformats::TimeSlice>(input_file->get_timeslice(record)));

      daqdataformats::TimeSlice timeslice = input_file->get_timeslice(record);      

      auto& fragments = timeslice.get_fragments_ref();
      if (m_helper->m_opts.verbose) { 
        fmt::print("DETAIL -- number of fragments {}\n", fragments.size());
	//get_valid_sourceids(fragments);
      }

      //} else {
        //std::vector<uint64_t> rw = {0, 0, std::numeric_limits<uint64_t>::max()};
        //get_tps_wrapper(fragments, rw);	
      //}

    // capture slice number(s) that match the trigger record
    // TODO make m_record_window sta::map<uint64_t, std::vecotr<uint64_t>>
    //for(const auto& trn: trigger_numbers) {
    //  m_record_window_map[m_rp->m_current_runnum].push_back(trn);
    //}


      fmt::print("DBG record windows {} ........................... \n", m_record_window_map.size());
      /*
      std::string temp = std::to_string(m_rp->m_current_runnum);
      temp += "_" + std::to_string(record.first);
      m_rp->m_current_folder = temp;
      fmt::print("DBG current folder, record {}, \n", temp, m_rp->m_current_recnum);
     
      if (m_trigger_numbers.size() > 0) {
        fmt::print("DBG current folder plot ############################ \n");
        m_rp->plot_tps(m_slice_tp_buffer, "slice");
      }
      */


      m_rp->m_current_recnum = m_input_slices_records[record.first];
      get_tps_wrapper(fragments, m_record_window_map[m_rp->m_current_recnum]);
      fmt::print("DETAIL -- TS plot for record/tps {}/{}\n", m_rp->m_current_recnum, m_slice_tp_buffer.size());
      m_rp->fill(m_slice_tp_buffer, RootPlotter::tpg_type::SLICE);
      if (m_helper->m_opts.verbose) {
        fmt::print("DETAIL -- TS number of fragments/tps {}/{}\n", fragments.size(), m_record_tp_buffer.size());
      }



    } // record

  } // file


}


void AppHelper::helper_3_old() {
  /* for each trigger record window get the corresponding timeslice window
   *
   */


  fmt::print("INFO trigger record: \n");
  fmt::print("INFO trigger record {}: \n", m_record_window.size());
  for (auto& i: m_record_window) {
    fmt::print("INFO trigger record {}: [ {} : {} ]\n", i.at(0), i.at(1), i.at(2));

    std::cout << dts_to_datetime(i.at(1), std::string("(beg)."));
    std::cout << dts_to_datetime(i.at(2), std::string("(end)."));

  }
  for (auto& [i, v2]: m_record_window_map) {
    fmt::print("INFO trigger record MAP {}: [ {} : {} ]\n", i, v2.at(0), v2.at(1));

    std::cout << dts_to_datetime(v2.at(0), std::string("(beg)."));
    std::cout << dts_to_datetime(v2.at(1), std::string("(end)."));

  }


  fmt::print("INFO timeslice {}: \n", m_input_files_tp.size());

  for (auto& input_file: m_input_files_tp) {

    m_rp->m_current_runnum = input_file->get_attribute<size_t>("run_number");
      fmt::print("DETAIL -- current run {}, {}\n", m_rp->m_current_runnum, m_record_window_map.size());

    auto records = input_file->get_all_record_ids();
    fmt::print("INFO slices: {}\n", records.size());
    fmt::print("INFO slices to process: {}\n", m_helper->m_opts.input_slices.size());
    for (const auto& record : records) {

      m_trigger_numbers.clear();
      m_slice_tp_buffer.clear();

      if (m_record_window_map.size() == 0) return; // speedup but cannot process slices only -- check -r option 

      if (m_helper->m_opts.input_slices.size() > 0) { 	    
        if (std::find(m_helper->m_opts.input_slices.begin(), m_helper->m_opts.input_slices.end(), record.first) == m_helper->m_opts.input_slices.end()) {
          continue;
        }
      }

      if (m_helper->m_opts.verbose) fmt::print("DETAIL timeslice number {}\n", record.first);

      //m_slice_buffer.push_back(std::make_unique<daqdataformats::TimeSlice>(input_file->get_timeslice(record)));

      daqdataformats::TimeSlice timeslice = input_file->get_timeslice(record);      

      auto& fragments = timeslice.get_fragments_ref();
      if (m_helper->m_opts.verbose) { 
        fmt::print("DETAIL -- number of fragments {}\n", fragments.size());
	//get_valid_sourceids(fragments);
      }


      if (m_record_window_map.size() > 0) {

      uint64_t begin = fragments.front()->get_header().window_begin;
      uint64_t end = fragments.front()->get_header().window_end;
       
      for (auto& [i, rw]: m_record_window_map) {

        //bool inWindow = (rw.at(0) > begin && rw.at(1) < end);
        //if (!inWindow) continue;
	
        // before
	if (end < rw.at(0)) continue; 
	// past
	if (begin > rw.at(1)) {
	  m_record_window_map.erase(m_record_window_map.begin());
	  //m_trigger_numbers.clear();
	  break;
	}
	// should not be needed 
        bool inWindow = ( (rw.at(0) > begin && rw.at(1) > end) || 
	     (rw.at(0) < begin && rw.at(1) < end) || 
	     (rw.at(0) > begin && rw.at(1) < end)
	   );
        if (!inWindow) continue;


        m_rp->m_current_recnum = i;
	//m_trigger_numbers.insert(i);
        fmt::print("DETAIL -- current run rw {}, {}\n", i, rw.size());

	//get_tps_old(fragments, rw);
        //trgdataformats::TriggerPrimitive tp;
        //TriggerPrimitive_v4 tp;
        //get_tps(fragments, rw, tp);
	//if (m_helper->m_opts.tp_format == "v4") {
        //  TriggerPrimitive_v4 tp;
        //  get_tps(fragments, rw, tp);
        //} else {
        //  trgdataformats::TriggerPrimitive tp;
        //  get_tps(fragments, rw, tp);
        //}
	get_tps_wrapper(fragments, rw);

      }
      } else {
        std::vector<uint64_t> rw = {0, 0, std::numeric_limits<uint64_t>::max()};
        get_tps_wrapper(fragments, rw);	
      }

    // capture slice number(s) that match the trigger record
    // TODO make m_record_window sta::map<uint64_t, std::vecotr<uint64_t>>
    //for(const auto& trn: trigger_numbers) {
    //  m_record_window_map[m_rp->m_current_runnum].push_back(trn);
    //}


    fmt::print("DBG record windows {} ........................... \n", m_record_window_map.size());
    std::string temp = std::to_string(m_rp->m_current_runnum);
    for (auto& it: m_trigger_numbers) {
        temp += "_" + std::to_string(it);
     }
     m_rp->m_current_folder = temp;
     fmt::print("DBG current folder, record {}, \n", temp, m_rp->m_current_recnum);
     
     if (m_trigger_numbers.size() > 0) {
       fmt::print("DBG current folder plot ############################ \n");
       m_rp->plot_tps(m_slice_tp_buffer, "slice");
     }


    } // record

  } // file
}



void AppHelper::dummy(auto& fragment) {
  std::unique_ptr<tpglibs::TPGenerator> tp_generator = m_te->get_tp_generator(fragment);


  int16_t num_frames = fragment->get_data_size() / sizeof(dunedaq::fddetdataformats::WIBEthFrame);

  std::cout << "DBG TPG NUM frames " << num_frames << "\n";
  for (int16_t ifr = 0; ifr < num_frames; ifr++) {
    //auto fp = reinterpret_cast<dunedaq::fdreadoutlibs::types::DUNEWIBEthTypeAdapter*>(
    //          static_cast<char*>(frag_ptr->get_data()) + ifr * sizeof(dunedaq::fddetdataformats::WIBEthFrame));
    auto fp = static_cast<fddetdataformats::WIBEthFrame*>(fragment->get_data() + ifr * sizeof(dunedaq::fddetdataformats::WIBEthFrame));


    auto wfptr = reinterpret_cast<dunedaq::fddetdataformats::WIBEthFrame*>((uint8_t*)fp); // NOLINT

    std::vector<trgdataformats::TriggerPrimitive> tps = (*tp_generator)(wfptr);

    fmt::print("DETAIL number of tps {}\n", tps.size());

  }

}



void AppHelper::helper_4() {
  // emulated tps
  /* for each trigger record get
   * 1. vecotor of tps 
   */
  if (m_helper->m_opts.verbose) fmt::print("DETAIL number of records {}\n", m_record_buffer.size());
  

  for (const auto& record : m_record_buffer) {

    std::vector<trgdataformats::TriggerPrimitive> output_buffer;
    std::vector<trgdataformats::TriggerPrimitive> temp_buffer;

    auto& fragments = record->get_fragments_ref();
    //auto& fragments = record.get_fragments_ref();

    for (const auto& fragment : fragments) {
      if (fragment->get_data_size() == 0) {
         continue;
      }
      if (fragment->get_fragment_type() != dunedaq::daqdataformats::FragmentType::kWIBEth) {
        continue;
      }
      //dummy(fragment);
      //temp_buffer = m_te->emulate_from(fragment);
      //std::unique_ptr<tpglibs::TPGenerator> tp_generator = m_te->get_tp_generator(fragment);
      //temp_buffer = m_te->emulate_from<tpglibs::TPGenerator>(std::move(tp_generator), std::move(fragment));
      temp_buffer = m_te->emulate_from(fragment);  

      if (temp_buffer.size() != 0) {
        output_buffer.insert(output_buffer.end(), temp_buffer.begin(), temp_buffer.end());
        temp_buffer.clear();
      }
    }

    fmt::print("DETAIL number of tps {}\n", output_buffer.size());
    int tpid = 0;
    for (auto& tp : output_buffer) {
      if (tpid < 10) {
        uint64_t ts = tp.time_start;
        int channel = (int)tp.channel;
        std::cout << "TP INFO " << channel << ", " << dts_to_datetime(ts, "(ts).");
        std::cout << "TP INFO " << tp.samples_over_threshold << ", " << tp.samples_to_peak << ", " << tp.adc_peak << ", " << tp.adc_integral << "\n";
      }
      tpid++;
    }
    m_rp->m_current_recnum = record->get_header_ref().get_trigger_number();
    //m_rp->m_current_recnum = record.get_header_ref().get_trigger_number();
    m_rp->plot_tps(output_buffer, "tpgemu");

    /*	  
    const auto& fragments = record->get_fragments_ref();

    for (const auto& fragment : fragments) {
      if (fragment->get_fragment_type() != daqdataformats::FragmentType::kWIBEth) {
        continue;
      } else {

        std::unique_ptr<tpglibs::TPGenerator> tp_generator = m_te->get_tp_generator(fragment);

	m_te->set_tp_generator(tp_generator);

	
        fddetdataformats::WIBEthFrame* wib_array = static_cast<fddetdataformats::WIBEthFrame*>(fragment->get_data());

        //std::vector<std::unique_ptr<fddetdataformats::WIBEthFrame*>> wib_buffer;
	size_t num_wibs = fragment->get_data_size() / sizeof(fddetdataformats::WIBEthFrame);
        //wib_buffer.reserve(num_wibs);
        
        for(size_t wid(0); wid<num_wibs; ++wid) {
	  auto wfptr = reinterpret_cast<dunedaq::fddetdataformats::WIBEthFrame*>((uint8_t*)&wib_array[wid]); // NOLINT
	  //std::unique_ptr<fddetdataformats::WIBEthFrame*> wib_ptr = std::make_unique<fddetdataformats::WIBEthFrame*>(&wib_array[wid]);
          //wib_buffer.push_back(std::move(wib_ptr));
	  //tps = 
	  (*tp_generator)(wfptr);
	}
        //fmt::print("DETAIL number of wib frames {}\n", wib_buffer.size());
	
	//std::vector<trgdataformats::TriggerPrimitive> tps = te.emulate_vector(std::move(wib_buffer));
	//std::vector<trgdataformats::TriggerPrimitive> tps = 
	//m_te->emulate_vector(wib_buffer);
        //fmt::print("DETAIL number of emulated tps {}\n", tps.size());

        //wib_buffer.clear();
    

      }
    }
    */

  } // record

}


void AppHelper::helper_5() {
  // simulated tps
  /* for each trigger record get
   * 1. vecotor of tps 
   */
  if (m_helper->m_opts.verbose) fmt::print("DETAIL number of records {}\n", m_record_buffer.size());
   
  auto m_tpg_configs = m_te->get_configs();

  //m_tpg_configs[0].first = "NaiveFrugalPedestalSubtractProcessor";
  //m_tpg_configs[1].first = "NaiveThresholdProcessor";
  //m_tpg_configs[0].first = "AVXFixedPedestalSubtractProcessor";
  //m_tpg_configs[1].first = "AVXThresholdProcessor";
  //m_te->set_configs(m_tpg_configs);

  m_tpg_configs = m_te->get_configs();
  for (auto& it : m_tpg_configs) {
    std::cout << "DBG simulation json config: " << it.first << ", " << it.second << "\n";
  }

  for (const auto& record : m_record_buffer) {

    std::vector<trgdataformats::TriggerPrimitive> output_buffer;
    std::vector<trgdataformats::TriggerPrimitive> temp_buffer;

    auto& fragments = record->get_fragments_ref();
    //auto& fragments = record.get_fragments_ref();

    int n_frags = 0;
    for (const auto& fragment : fragments) {
      //if (n_frags > 0) break; // TEMP test
      if (fragment->get_data_size() == 0) {
         continue;
      }
      if (fragment->get_fragment_type() != dunedaq::daqdataformats::FragmentType::kWIBEth) {
        continue;
      }
      //temp_buffer = m_te->emulate_from(fragment);
      //std::unique_ptr<tpglibs::NaiveTPGenerator> tp_generator = m_te->get_tp_generator_naive(fragment);
      //temp_buffer = m_te->emulate_from<tpglibs::NaiveTPGenerator>(std::move(tp_generator), std::move(fragment));
      temp_buffer = m_te->emulate_from_naive(fragment);

      if (temp_buffer.size() != 0) {
        output_buffer.insert(output_buffer.end(), temp_buffer.begin(), temp_buffer.end());
        temp_buffer.clear();
      }
      n_frags++;
    }

    fmt::print("DETAIL number of tps {}\n", output_buffer.size());
    int tpid = 0;
    for (auto& tp : output_buffer) {
      if (tpid < 10) {
        uint64_t ts = tp.time_start;
        int channel = (int)tp.channel;
        std::cout << "TP INFO " << channel << ", " << dts_to_datetime(ts, "(ts).");
        std::cout << "TP INFO " << tp.samples_over_threshold << ", " << tp.samples_to_peak << ", " << tp.adc_peak << ", " << tp.adc_integral << "\n";
      }
      tpid++;
    }
    m_rp->m_current_recnum = record->get_header_ref().get_trigger_number();;
    //m_rp->m_current_recnum = record.get_header_ref().get_trigger_number();;
    m_rp->plot_tps(output_buffer, "tpgnaive");

  } // record

}

void AppHelper::helper_6() {
  using generator_t = tpglibs::NaiveTPGenerator;
  std::unique_ptr<TPGEmulators<generator_t>> m_tes = std::make_unique<TPGEmulators<generator_t>>();

  std::ifstream config_stream(m_helper->m_opts.config_name);
  nlohmann::json config = nlohmann::json::parse(config_stream);
  m_tes->configure(config);

  std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> tps = m_tes->emulate_using(m_record_buffer);
  //std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> tps;


  int n_tps = 0;
  for (auto [rid, tp_vec] : tps) {
    m_rp->m_current_recnum = rid;
    m_rp->plot_tps(tp_vec, "tpgnaive");
    n_tps += tp_vec.size();
  }
  fmt::print("DBG found tps : {}, {}\n", tps.size(), n_tps);

}

void AppHelper::helper_7() {
  using generator_t = tpglibs::TPGenerator;
  std::unique_ptr<TPGEmulators<generator_t>> m_tes = std::make_unique<TPGEmulators<generator_t>>();

  std::ifstream config_stream(m_helper->m_opts.config_name);
  nlohmann::json config = nlohmann::json::parse(config_stream);
  m_tes->configure(config);

  std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> tps = m_tes->emulate_using(m_record_buffer);
  //std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> tps;

  int n_tps = 0;
  for (auto [rid, tp_vec] : tps) {
    m_rp->m_current_recnum = rid;
    m_rp->plot_tps(tp_vec, "tpgemu");
    n_tps += tp_vec.size();
  }
  fmt::print("DBG found tps : {}, {}\n", tps.size(), n_tps);

}

void AppHelper::helper_8() {
  using generator_t = tpglibs::TPSimulator;
  std::unique_ptr<TPGEmulators<generator_t>> m_tes = std::make_unique<TPGEmulators<generator_t>>();

  std::ifstream config_stream(m_helper->m_opts.config_name);
  nlohmann::json config = nlohmann::json::parse(config_stream);
  m_tes->configure(config);

  std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> tps = m_tes->emulate_using(m_record_buffer);
  //std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> tps;

  int n_tps = 0;
  for (auto [rid, tp_vec] : tps) {
    m_rp->m_current_recnum = rid;
    n_tps += tp_vec.size();
    fmt::print("DBG found tps : {}, {}\n", tps.size(), n_tps);
    m_rp->plot_tps(tp_vec, "tpgsim");
  }

}


void AppHelper::helper_9_tpgs() {

  std::vector<RootPlotter::tpg_type> tmp = {RootPlotter::tpg_type::RECORD, 
	  RootPlotter::tpg_type::SLICE, 
	  RootPlotter::tpg_type::TPGEMU, 
	  RootPlotter::tpg_type::TPGNAIVE, 
	  RootPlotter::tpg_type::TPGSIM
  };

  int idx = 2;
  for (auto& new_config : m_tpg_configs) {

    std::map<uint64_t, std::vector<trgdataformats::TriggerPrimitive>> tps;
    if (idx == 2) {
      tps = emulate_using<tpglibs::TPGenerator>(new_config, m_record_buffer);
    }
    if (idx == 3) {
      tps = emulate_using<tpglibs::NaiveTPGenerator>(new_config, m_record_buffer);
    }
    if (idx == 4) {
      tps = emulate_using<tpglibs::TPSimulator>(new_config, m_record_buffer);
    }
   

    int n_tps = 0;
    for (auto [rid, tp_vec] : tps) {
      n_tps += tp_vec.size();
      fmt::print("DBG found tps : {}/{}: {}, {}, {}, \n", rid, tp_vec.size(), idx, tps.size(), n_tps);

      m_rp->m_current_recnum = rid;
      m_rp->fill(tp_vec, tmp[idx]);
    }

    idx++;
  } 
  m_rp->show(this);
}
void AppHelper::helper_9() {
  m_rp->m_f->Close();
  delete m_rp->m_f;
}







// test 
void AppHelper::helper_10() {
 
  fmt::print("TP_v4 size {}\n", BYTES_PER_TP = sizeof(TriggerPrimitive_v4));


}



};

#endif
