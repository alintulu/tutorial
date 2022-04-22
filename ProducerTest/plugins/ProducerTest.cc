// -*- C++ -*-
//
// Package:    tutorial/ProducerTest
// Class:      ProducerTest
//
/**\class ProducerTest ProducerTest.cc tutorial/ProducerTest/plugins/ProducerTest.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Adelina Eleonora Lintuluoto
//         Created:  Thu, 21 Apr 2022 08:56:08 GMT
//
//

// system include files
#include <memory>

// standard C++ includes
#include <memory>
#include <vector>
#include <iostream>

// root include files
#include "TLorentzVector.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TVectorD.h"
#include "TMatrixD.h"
#include "TMatrixDSym.h"
#include "TMatrixDSymEigen.h"

// CMSSW data formats
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Math/interface/angle.h"
#include "DataFormats/Scouting/interface/Run3ScoutingParticle.h"
#include "DataFormats/Scouting/interface/Run3ScoutingVertex.h"
#include "DataFormats/JetReco/interface/BasicJet.h"
#include "DataFormats/JetReco/interface/BasicJetCollection.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "fastjet/ClusterSequence.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/contrib/Nsubjettiness.hh"
#include "fastjet/contrib/XConePlugin.hh"
#include "fastjet/contrib/SoftDrop.hh"
#include "fastjet/tools/Pruner.hh"
#include "fastjet/tools/Filter.hh"
#include "fastjet/contrib/RecursiveSoftDrop.hh"
#include "fastjet/contrib/EnergyCorrelator.hh"
#include "fastjet/JadePlugin.hh"
#include "fastjet/contrib/SoftKiller.hh"

#include "Math/GenVector/VectorUtil.h"

//
// class declaration
//
class ProducerTest : public edm::stream::EDProducer<>
{
public:
  explicit ProducerTest(const edm::ParameterSet &);
  ~ProducerTest();

  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

private:
  void beginStream(edm::StreamID) override;
  void produce(edm::Event &, const edm::EventSetup &) override;
  void endStream() override;

  const edm::EDGetTokenT<std::vector<Run3ScoutingParticle>> pfcands_;
  const edm::EDGetTokenT<std::vector<Run3ScoutingParticle>> vertices_;
};

ProducerTest::ProducerTest(const edm::ParameterSet &iConfig) : pfcands_(consumes<Run3ScoutingParticleCollection>(iConfig.getParameter<edm::InputTag>("pfcands"))),
                                                               vertices_(consumes<Run3ScoutingParticleCollection>(iConfig.getParameter<edm::InputTag>("vertices")))
{
  produces<reco::BasicJetCollection>("recoJet").setBranchAlias("recoJets");
}

ProducerTest::~ProducerTest() = default;

// ------------ method called to produce the data  ------------
void ProducerTest::produce(edm::Event &iEvent, const edm::EventSetup &iSetup)
{
  using namespace edm;
  using namespace std;

  Handle<Run3ScoutingParticleCollection> pfcands;
  iEvent.getByToken(pfcands_, pfcands);

  Handle<Run3ScoutingVertexCollection> vertices;
  iEvent.getByToken(vertices_, vertices);

  // create vector of fast jets
  vector<fastjet::PseudoJet> fj_particles;
  int pfcand_i = 0;
  for (auto pfcand = pfcands->begin(); pfcand != pfcands->end(); ++pfcand)
  {
    math::PtEtaPhiMLorentzVector p4(pfcand->pt(), pfcand->eta(), pfcand->phi(), pfcand->m());
    fj_particles.emplace_back(p4.px(), p4.py(), p4.pz(), p4.energy());
    fj_particles.back().set_user_index(pfcand_i);
    pfcand_i++;
  }

  fastjet::JetDefinition ak4_def = fastjet::JetDefinition(fastjet::antikt_algorithm, 0.4);
  fastjet::GhostedAreaSpec area_spec(5.0, 1, 0.01);
  fastjet::AreaDefinition area_def(fastjet::active_area, area_spec);
  fastjet::ClusterSequenceArea ak4_cs(fj_particles, ak4_def, area_def);
  vector<fastjet::PseudoJet> fj_jets = fastjet::sorted_by_pt(ak4_cs.inclusive_jets(15.0));

  // allocate fj_jets.size() in reco::Jet vector
  auto jets = std::make_unique<std::vector<reco::BasicJet>>(fj_jets.size());

  for (unsigned int ijet = 0; ijet < fj_jets.size(); ++ijet)
  {
    auto &jet = (*jets)[ijet];

    // get the fastjet jet
    const fastjet::PseudoJet &fj_jet = fj_jets[ijet];

    // get the constituents from fastjet
    std::vector<fastjet::PseudoJet> const &fj_constituents = fastjet::sorted_by_pt(fj_jet.constituents());

    auto vertex_ = reco::Jet::Point(0, 0, 0);
    jet = reco::BasicJet(reco::Particle::LorentzVector(fj_jet.px(), fj_jet.py(), fj_jet.pz(), fj_jet.E()), vertex_);

    // // convert constituents to CandidatePtr vector
    // vector<reco::CandidatePtr> result;
    // result.reserve(fj_constituents.size() / 2);
    // for (unsigned int i = 0; i < fj_constituents.size(); i++)
    // {
    //   auto index = fj_constituents[i].user_index();
    //   if (index >= 0 && static_cast<unsigned int>(index) < fj_particles.size())
    //   {
    //     result.emplace_back(&pfcands->at(index));
    //   }
    // }
  }

  iEvent.put(std::move(jets), "recoJets");
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void ProducerTest::beginStream(edm::StreamID)
{
  // please remove this method if not needed
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void ProducerTest::endStream()
{
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void ProducerTest::fillDescriptions(edm::ConfigurationDescriptions &descriptions)
{
  // The following says we do not know what parameters are allowed so do no validation
  //  Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("pfcands", edm::InputTag("hltScoutingPFPacker"));
  desc.add<edm::InputTag>("vertices", edm::InputTag("hltScoutingPFPacker"));
  descriptions.addDefault(desc);
}

// define this as a plug-in
DEFINE_FWK_MODULE(ProducerTest);
