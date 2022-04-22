# Test repository

The goal is to find the jet flavour of Run3ScoutingPFJets by performing the [ghost association](https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuideBTagMCTools#Hadron_parton_based_jet_flavour) method.
The method requires reco::Jets as input, so in this Github repository I am trying to convert Run3ScoutingParticles to reco::Jets.

The ghost association method requires the following information from the jets:

```
getJetConstituents()
rapidity()
phi()
pt()
```

Therefore I think it is enough to create reco::BasicJets.
