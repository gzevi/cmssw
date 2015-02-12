#include "PhysicsTools/SelectorUtils/interface/CutApplicatorWithEventContentBase.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
#include "DataFormats/EgammaCandidates/interface/ConversionFwd.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "RecoEgamma/EgammaTools/interface/ConversionTools.h"


class GsfEleConversionVetoCut : public CutApplicatorWithEventContentBase {
public:
  GsfEleConversionVetoCut(const edm::ParameterSet& c);
  
  result_type operator()(const reco::GsfElectronPtr&) const override final;

  void setConsumes(edm::ConsumesCollector&) override final;
  void getEventContent(const edm::EventBase&) override final;

  CandidateType candidateType() const override final { 
    return ELECTRON; 
  }

private:  
  edm::Handle<reco::ConversionCollection> _convs;
  edm::Handle<reco::BeamSpot> _thebs;
};

DEFINE_EDM_PLUGIN(CutApplicatorFactory,
		  GsfEleConversionVetoCut,
		  "GsfEleConversionVetoCut");

GsfEleConversionVetoCut::GsfEleConversionVetoCut(const edm::ParameterSet& c) :
  CutApplicatorWithEventContentBase(c) {
  edm::InputTag conversiontag = c.getParameter<edm::InputTag>("conversionSrc");
  contentTags_.emplace("conversions",conversiontag);
  edm::InputTag conversiontagMA = c.getParameter<edm::InputTag>("conversionSrcMiniAOD");
  contentTags_.emplace("conversionsMA",conversiontagMA);
  edm::InputTag beamspottag = c.getParameter<edm::InputTag>("beamspotSrc");
  contentTags_.emplace("beamspot",beamspottag);
}

void GsfEleConversionVetoCut::setConsumes(edm::ConsumesCollector& cc) {
  auto convs = cc.mayConsume<reco::ConversionCollection>(contentTags_["conversions"]);
  auto convsMA = cc.mayConsume<reco::ConversionCollection>(contentTags_["conversionsMA"]);
  auto thebs = cc.consumes<reco::BeamSpot>(contentTags_["beamspot"]);
  contentTokens_.emplace("conversions",convs);
  contentTokens_.emplace("conversionsMA",convsMA);
  contentTokens_.emplace("beamspot",thebs);
}

void GsfEleConversionVetoCut::getEventContent(const edm::EventBase& ev) {    
  // First try AOD, then go to miniAOD. Use the same Handle since collection class is the same.
  ev.getByLabel(contentTags_["conversions"],_convs);
  if (!_convs.isValid())
     ev.getByLabel(contentTags_["conversionsMA"],_convs);
  ev.getByLabel(contentTags_["beamspot"],_thebs);  
}

CutApplicatorBase::result_type 
GsfEleConversionVetoCut::
operator()(const reco::GsfElectronPtr& cand) const{  
  if( _thebs.isValid() && _convs.isValid() ) {
    return !ConversionTools::hasMatchedConversion(*cand,_convs,
						  _thebs->position());
  } else {
    edm::LogWarning("GsfEleConversionVetoCut")
      << "Couldn't find a necessary collection, returning true!";
  }
  return true;
}
