import FWCore.ParameterSet.Config as cms

process = cms.Process("test")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(5) )

process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
       'file:/eos/cms/store/group/ml/Tagging4ScoutingHackathon/Adelina/DeepNtuples/12_3_0/ScoutingAK4-v00/BulkGraviton_hh_GF_HH_14TeV_TuneCP5_pythia8/DeepNtuplesAK4-v00/220420_102614/0000/mini_1.root'
    )
)

process.recoJets = cms.EDProducer("ProducerTest",
    pfcands  = cms.InputTag("hltScoutingPFPacker"),
    vertices = cms.InputTag("hltScoutingPFPacker")
)

process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('output.root')
)

process.p = cms.Path(process.recoJets)

process.e = cms.EndPath(process.out)